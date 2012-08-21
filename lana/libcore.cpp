/** @file
 * The core library functions, which the API starts up with
 */

#include "api.h"
#include "language.h"
#include "vm.h"
#include "object.h"
#include "dict.h"
#include "list.h"
#include "iterobj.h"
#include "consts.h"

#include <math.h>

namespace lana {

/// the Range iterator, created by the range() lib function

class RangeIterator : public Iterator<Value *> {
public:
    RangeIterator(int b,int t){
        v.setInt(0);
        pct = &v.d.i;
        
        bottom=b;
        top=t;
    }
    
    virtual void first(){
        *pct = bottom;
    }
    
    virtual void next(){
        (*pct)++;
    }
    
    virtual bool isDone() const{
        return *pct>=top;
    }
    
    virtual Value *current(){
        return &v;
    }
    
private:
    Value v;
    s32 *pct;
    int bottom,top;
};


/// handy macro to help writing methods

#define MT(xx) (lana::HOSTMETHOD)&LibCoreHost::xx

/// Useful library functions bound to the API at at startup - note that
/// Lana itself doesn't get these; they're bound when the API is created.

class LibCoreHost : public Host {
public:
    
    virtual ~LibCoreHost(){
        if(argList){
            delete argList;
        }
    }
    LibCoreHost(API *a,Language *l) : Host(a) {
        lana = l;
        argList = NULL;
        argc=0;
        
        a->setNamePrefix("CORE$");
        a->globalNativeHostedMethod("str",1,true,this,MT(str));
        a->globalNativeHostedMethod("int",1,true,this,MT(intg));
        a->globalNativeHostedMethod("float",1,true,this,MT(flt));
        a->globalNativeHostedMethod("hash",1,true,this,MT(hash));
        
        a->globalNativeHostedMethod("print",1,false,this,MT(print));
        a->globalNativeHostedMethod("printrepr",2,false,this,MT(printrepr));
        
        a->globalNativeHostedMethod("resetinstcount",0,false,this,MT(resetinstcount));
        a->globalNativeHostedMethod("instcount",0,true,this,MT(instcount));
        a->globalNativeHostedMethod("setdebug",1,false,this,MT(setdebug));
        
        a->globalNativeHostedMethod("create",0,true,this,MT(create));
        a->globalNativeHostedMethod("clone",1,true,this,MT(clone));
        a->globalNativeHostedMethod("gc",0,true,this,MT(gc));
        a->globalNativeHostedMethod("gccount",0,true,this,MT(gccount));
        a->globalNativeHostedMethod("dict",0,true,this,MT(dict));
        a->globalNativeHostedMethod("list",0,true,this,MT(list));
        a->globalNativeHostedMethod("range",2,true,this,MT(range));
        
        a->globalNativeHostedMethod("defined",1,true,this,MT(defined));
        a->globalNativeHostedMethod("del",1,true,this,MT(del));
        a->globalNativeHostedMethod("size",1,true,this,MT(size));
        a->globalNativeHostedMethod("keys",1,true,this,MT(keys));
        a->globalNativeHostedMethod("values",1,true,this,MT(values));
        
        
	a->globalNativeHostedMethod("native",1,true,this,MT(native));
        
        a->globalNativeHostedMethod("args",0,true,this,MT(args));
        
        a->globalNativeHostedMethod("pow",2,true,this,MT(pow));
        a->globalNativeHostedMethod("sin",1,true,this,MT(sin));
        a->globalNativeHostedMethod("cos",1,true,this,MT(cos));
        a->globalNativeHostedMethod("tan",1,true,this,MT(tan));
        
        a->globalNativeHostedMethod("instring",1,true,this,MT(instring));
    }
    
    /// called after setting argc,argv to create the list
    void createArgList(class API *a){
        argList = List::create(a);
        for(int i=0;i<argc;i++){
            int id = api->lana->consts->findOrCreateString(argv[i]);
            Value *v = argList->list->append();
            v->setStrConst(id);
        }
        argList->incRefCt();
        argList->incRefCt();
        argList->incRefCt();
        argList->incRefCt();
    }
    int argc;
    char **argv;
    
private:    
    List *argList;
    Language *lana;
    
    void str(){
        char *s = api->popStr();
        api->pushStr(s);
    }
    
    void intg(){
        api->pushInt(api->popInt());
    }
    
    void flt(){
        api->pushFloat(api->popFloat());
    }
    
    void print(){
        char *s = api->popStr();
        printf("%s\n",s);
    }
    void printrepr(){
        lana::Value *v = api->popRaw();
        char *s = api->popStr();
        printf("%s: %s\n",s,v->repr());
    }
    
    void resetinstcount(){
        api->resetInstructionCount();
    }
    
    void instcount() {
        api->pushInt(api->getInstructionCount());
    }
    
    void setdebug() {
        api->setDebug(api->popInt());
    }
    
    void create() {
        Object *o = new Object(api);
        api->pushObj(o);
    }
    void clone() {
        Object *o = api->popObj();
        Object *clone = o->clone(api);
        api->pushObj(clone);
    }
    
    void gc(){
        api->cycle->detect();
        api->pushInt(api->cycle->count());
    }
    
    void gccount(){
        api->pushInt(api->cycle->count());
    }
    
    void hash(){
        u32 hash = api->popRaw()->getHash();
        api->pushInt(hash);
    }
    
    void dict(){
        Value *v = api->pushRaw();
        v->setOther(Types::vtDictionary,(void *)Dict::create(api));
        v->incRef();
    }
    void list(){
        Value *v = api->pushRaw();
        v->setOther(Types::vtList,(void *)List::create(api));
        v->incRef();
    }
    
    void range(){
        int top = api->popInt();
        int bottom = api->popInt();
        
        IteratorObject *o = IteratorObject::create(api,new RangeIterator(bottom,top));
        Value *v = api->pushRaw();
        v->setIterObj(o);
    }
    
    ////////// these only work on container references //////////////////
    
    // in the first two, the stacked item is "x[y]" -- it's a ref.
    // in the third, it's a container object.
    void defined() {
        Value *v = api->popRawWithoutDeref();
        api->pushBool(v->type->isDefinedReference(v));
    }
    
    void del(){
        Value *v = api->popRawWithoutDeref();
        api->pushBool(v->type->deleteElement(v));
    }
    
    void size(){
        Value *v = api->popRaw();
        api->pushInt(v->type->getSize(v));
    }
    
    void keys(){
        Value *src = api->popRaw();
        IteratorObject *o = IteratorObject::create(api,src,true);
        Value *v = api->pushRaw();
        v->setIterObj(o);
    }
    
    void values(){
        Value *src = api->popRaw();
        IteratorObject *o = IteratorObject::create(api,src,false);
        Value *v = api->pushRaw();
        v->setIterObj(o);
    }
    
    //////////// maths functions /////////////////////////////////////////
    
    
    void pow() { 
        float y = api->popFloat();
        float x = api->popFloat();
        api->pushFloat(powf(x,y));
    }
    
    void cos() { api->pushFloat(cosf(api->popFloat()));}
    void sin() { api->pushFloat(sinf(api->popFloat()));}
    void tan() { api->pushFloat(tanf(api->popFloat()));}
    
    
    
    //////////// other stuff     /////////////////////////////////////////
    
    void instring(){
        char *s = api->popStr();
        printf(s);
        char buf[256];
        fgets(buf,256,stdin);
        api->pushStr(buf);
    }
    
    void native(){
        char *s = api->popStr();
        NativeFuncData *d = lana->natFuncs.getByName(s);
        if(!d)
            throw Exception(NULL).set("unknown native function: %s",s);
        Value *v = api->pushRaw();
        v->setNativeFunctionRef(d);
    }
    
    void args(){
        Value *v = api->pushRaw();
        if(!argList)
            throw Exception(NULL).set("no arguments set");
        v->setOther(Types::vtList,(void *)argList);
        v->incRef();
    }
};



class Host *registerCoreHost(API *api,Language *lana){
    LibCoreHost *h = new LibCoreHost(api,lana);
    return h;
}

void libcoreSetArgcArgv(API *api,LibCoreHost *h,int argc,char **argv){
    h->argc = argc;
    h->argv = argv;
    h->createArgList(api);
}

}
