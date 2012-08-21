/**
 * @file
 * Implementations of the type-specific method for most types - many of the implementations
 * are inline in vtypes.h.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"
#include "value.h"
#include "consts.h"
#include "object.h"
#include "iterobj.h"
#include "intkeyedhash.h"
#include "dict.h"
#include "list.h"

using namespace lana;

Type *Types::vtInteger=NULL;
Type *Types::vtFloat=NULL;
Type *Types::vtComment=NULL;
Type *Types::vtFunction=NULL;
Type *Types::vtLDT=NULL;
Type *Types::vtRef=NULL;
Type *Types::vtNativeFunctionRef=NULL;
Type *Types::vtStringConst=NULL;
Type *Types::vtBoolean=NULL;
Type *Types::vtPropRef=NULL;
Type *Types::vtString=NULL;
Type *Types::vtObject=NULL;
Type *Types::vtDictionary=NULL;
Type *Types::vtIterObj=NULL;
Type *Types::vtList=NULL;
Type *Types::vtListRef=NULL;
Type *Types::vtDictRef=NULL;
Type *Types::vtDeleted=NULL;
Type *Types::vtNativeMethodRef=NULL;

#include "fasthash.h"
/// head of list
static Type *typeList=NULL;


/// add a type to the list - see createTypes() - passes the type through.
static Type *addt(Type *t){
    t->next = typeList;
    typeList = t;
    return t;
}

void Types::createTypes(API *a){
    // arguments to set: alloctype, is it hashed during serialisation,  shortname, long name
    
    // the order of these is VERY important. If you use prototype objects
    // in your types (dictionaries, iterobjs) they should go at the end.
    
    vtInteger=addt((new IntegerType)->set(Unmanaged,false,"integer","I"));
    vtFloat=addt((new FloatType)->set(Unmanaged,false,"float","f"));
    vtComment=addt((new Type)->set(Unmanaged,false,"comment","C"));
    vtFunction=addt((new FunctionType)->set(Unmanaged,true,"function","FN"));
    vtLDT=addt((new Type)->set(Unmanaged,false,"LDT","LDT"));
    vtNativeMethodRef=addt((new SimpleHashableType)->set(Complex,false,"natmethodref","NM"));
    vtRef=addt((new RefType)->set(Unmanaged,false,"Ref","R"));
    vtNativeFunctionRef=addt((new NativeFunctionType)->set(Unmanaged,false,"nativefunc","NF"));
    vtStringConst=addt((new StringConstType)->set(Unmanaged,false,"stringconst","SC"));
    vtBoolean=addt((new BooleanType)->set(Unmanaged,false,"boolean","B"));
    vtPropRef=addt((new PropRefType)->set(Complex,false,"propref","PR"));
    vtString=addt((new StringType)->set(SimpleMalloc,false,"string","S"));
    vtObject=addt((new ObjectType)->set(Complex,true,"object","O"));
    vtDictRef=addt((new DictRefType)->set(DictRefAlloc,false,"dictref","DR"));
    vtListRef=addt((new ListRefType)->set(Complex,false,"listref","LR"));
    vtDeleted=addt((new Type)->set(Unmanaged,false,"deletedkey","DelKey"));
    
    // anything which is an object, where the type holds a prototype,
    // needs to be down here
    
    vtIterObj=addt((new IterObjectType(a))->set(Complex,false,"iteratorobject","IO"));
    vtDictionary=addt((new DictionaryType(a))->set(Complex,true,"dictionary","Dict"));
    vtList=addt((new ListType(a))->set(Complex,true,"list","LST"));
}

void Types::deleteTypes(){
    Type *t = typeList;
    while(t){
        Type *n = t->next;
        delete t;
        t=n;
    }
    typeList = NULL;
}


void Type::doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs){
    // equality/inequality always works, but note that in this default case
    // it's an identity check - and we only check d. Complex types which use d2
    // may not do this check correctly, but they're usually internal reference types
    // at the moment.
    
    bool r;
    if(rhs->type == this && lhs->d.u == rhs->d.u){
        switch(op){
        case OP_EQUALS: r = true;break;
        case OP_NEQUALS: r = false;break;
        default:throw Exception("invalid comparison operator");
        }
    } else {
        switch(op){
        case OP_EQUALS: r = false;break;
        case OP_NEQUALS: r = true;break;
        default:throw Exception("invalid comparison operator");
        }
    }
    out->setBool(r);
}


char *Type::getStr(const Value *v) const {
    sprintf(buf,"%s:%08x",getName(true),v->d.i);
    return buf;
}

char *Type::getPtr(const Value *v) const {
    return v->d.s;
}

const char *Type::repr(const Value *v) const {
    startRepr();
    sprintf(buf+strlen(buf),"%08x",v->d.i);
    return buf;
}


char *FunctionType::getPtr(const Value *v) const {
    // the constant holds the the pointer, so get() will get
    // a pointer to a pointer..
    char *p = *(char **)Value::consts->get(v->d.u)->get();
    return p;
}




/// near equality check
inline bool neareq(float a,float b){
    extern float *neareq_epsilon;
    a -=b;
    bool p = a<*neareq_epsilon;
    bool q = a>-*neareq_epsilon;
    return (a<*neareq_epsilon && a>-*neareq_epsilon);
}

/// helper for comparisons of floats and ints, converting both values to floats
inline void floatCompare(Value *out,int op,Value *lhs,Value *rhs){
    float a = lhs->getFloat();
    float b = rhs->getFloat();
    bool r;
    switch(op){
    case OP_EQUALS: r = a==b;break;
    case OP_NEQUALS:r = a!=b;break;
    case OP_NEAREQ: r = neareq(a,b);break;
    case OP_NNEAREQ:r = !neareq(a,b);break;
    case OP_LT:     r = a<b; break;
    case OP_LTE:    r = a<=b; break;
    case OP_GT:     r = a>b; break;
    case OP_GTE:    r = a>=b; break;
    default:r=false;
    }
    out->setBool(r);
}

/// helper for comparisons of floats and ints, converting both values to ints
inline void intCompare(Value *out,int op,Value *lhs,Value *rhs){
    float a = lhs->getInt();
    float b = rhs->getInt();
    bool r;
    switch(op){
    case OP_NEAREQ:
    case OP_EQUALS: r = a==b;break;
    case OP_NNEAREQ:
    case OP_NEQUALS:r = a!=b;break;
    case OP_LT:     r = a<b; break;
    case OP_LTE:    r = a<=b; break;
    case OP_GT:     r = a>b; break;
    case OP_GTE:    r = a>=b; break;
    default:r=false;
    }
    out->setBool(r);
}


void StringType::doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs){
    
    if(rhs->type == Types::vtFloat)
        // string compared to float, do a float comparison
        floatCompare(out,op,lhs,rhs);
    else if(rhs->type == Types::vtInteger)
        // string compared to int, do an int comparison
        intCompare(out,op,lhs,rhs);
    else {
        // straight string compare, although RHS might get converted
        
        char *p = lhs->getStr();
        char *q = rhs->getStr();
        bool r;
        
        switch(op){
        case OP_LT:
            r = strcmp(p,q)<0;        break;
        case OP_LTE:
            r = strcmp(p,q)<=0;        break;
        case OP_GT:
            r = strcmp(p,q)>0;        break;
        case OP_GTE:
            r = strcmp(p,q)>=0;        break;
        case OP_EQUALS:
            r = strcmp(p,q)==0;        break;
        case OP_NEQUALS:
            r = strcmp(p,q)!=0;        break;
        case OP_NEAREQ:
            r = strcasecmp(p,q)==0;        break;
        case OP_NNEAREQ:
            r = strcasecmp(p,q)!=0;        break;
        default:r=false; // never happens, we hope
        }
        out->setBool(r);
    }
}

u32 StringType::getHash(const Value *v) const {
    char *s = getStr(v);
    return fastHash(s,strlen(s));
}

char *StringConstType::getStr(const Value *v) const {
    return (char *)Value::consts->getStr(v->d.u);
}
char *StringConstType::getPtr(const Value *v) const {
    return (char *)Value::consts->get(v->d.u)->get();
}
int StringConstType::getInt(const Value *v) const {
    return atoi((char *)Value::consts->get(v->d.u)->get());
}
float StringConstType::getFloat(const Value *v) const {
    return atof((char *)Value::consts->get(v->d.u)->get());
}
void IntegerType::doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs){
    if(rhs->type == Types::vtFloat)
        floatCompare(out,op,lhs,rhs);
    else
        intCompare(out,op,lhs,rhs);
}
void FloatType::doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs){
    floatCompare(out,op,lhs,rhs);
}



bool IterableType::makePropRef(Value *v,Value *item,u32 prop){
    Iterable *iterable = item->d.iterable;
    /* for clarity's sake I've turned these off - if you can't do
     * a.size why should you be able to do a.keys?
     * 
    if(prop == Value::consts->props.propKeys) {
        IteratorObject *i = IteratorObject::create(iterable->api,item,true);
        v->setIterObj(i);
        return true;
    }else if(prop == Value::consts->props.propValues) {
        IteratorObject *i = IteratorObject::create(iterable->api,item,false);
        v->setIterObj(i);
        return true;
    }
*/    
    return false;

}


bool StringType::makePropRef(Value *v,Value *item,u32 prop){
    char *s = item->getStr();
    
    if(prop == Value::consts->props.propSize){
        v->setInt(strlen(s));
        return true;
    }
    return false;
}
