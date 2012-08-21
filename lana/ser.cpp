#include "api.h"
#include "language.h"
#include "session.h"
#include "object.h"
#include "dict.h"
#include "ser.h"
#include "list.h"

using namespace lana;


Serialiser::Serialiser(Session *s){
    session = s;
    api = s->api;
    lana = s->lana;
    consts = s->lana->consts;
    stringStore = new Growable(256,256,1);
    lana->recreateIndent=0;
}

Serialiser::~Serialiser(){
    delete stringStore;
}


const char *Serialiser::indents(){
    return lana->indents();
}

void Serialiser::write(FILE *out){
    // iterate through the globals, writing out each one.
    
    IteratorPtr<int> iterator(lana->globs->createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        int i = iterator->current();
        if(lana->globs->isUser(i)){
            int descID = lana->globs->getName(i);
            const char *name = lana->consts->getStr(descID);
            Value *v = lana->globs->get(i);
            serialiseValue(out,v,name);
            fputs("\n",out); // extra newline twixt each global
        }
    }
}

void Serialiser::setHash(u32 k,const char *nam){
    //    printf("storing %lx as %s\n",k,nam);
    *(hash.set(k)) = stringStore->getOffset();
    char *s = stringStore->write(nam);
    stringStore->terminate();
}

const char *Serialiser::getHash(u32 k){
    if(hash.find(k)){
        return (const char *)stringStore->get(*hash.getval(),0);
    } else 
        return NULL;
}

void Serialiser::serialiseValue(FILE *out, Value *v, const char *name){
    if(!v->type)return; // don't output empty values
    if(v->type == Types::vtNativeMethodRef)
        return; // or native method refs (as these can't be assigned)
    
    // first, is the value already in the value hash? We determine if
    // it's the sort of value which should be.
    
    if(v->type->serHash) {
        // get the target of this object
        u32 target = v->d.u;
        
        // this is in the hash?
        const char *t = getHash(target);
        if(t){
            // it's already there, do an output pointing to that old name
            fprintf(out,"%s%s = %s\n",indents(),name,t);
        } else {
            // otherwise output it, and add it to the hash
            v->type->serialiseAssignment(this,name,out,v); // then value
            setHash(target,name);
        }            
    } else {
        // the very straightforward route
        v->type->serialiseAssignment(this,name,out,v); // then value
    }
}

void Type::serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const {
    // this is the standard version for simple types
    
    fputs(s->indents(),out);
    fputs(name,out);
    fputs(" = ",out);
    serialise(s,out,v);
    fputs("\n",out);
}


/// standard serialisers

void Type::serialise(Serialiser *s,FILE *out,Value *v) const {
    // this is the standard version
    fputs(getStr(v),out);
}

// just wraps the string in quotes; should also escape and line break!

void StringType::serialise(Serialiser *s,FILE *out,Value *v) const {
    // consider putting line breaking in here :)
    fprintf(out,"\"%s\"",getStr(v));
}

void FunctionType::serialise(Serialiser *s,FILE *out,Value *v) const {
    // first four bytes are size
    int *ptr = (int *)v->getPtr();
    instruction *byteCode = (instruction *)(ptr+1);
    
    const char *str = s->lana->recreateFunction(byteCode,*ptr,s->session);
    fputs(str,out);
    free((void *)str);
    
}

void NativeFunctionType::serialise(Serialiser *s,FILE *out,Value *v) const {
  NativeFuncData *d = (NativeFuncData *)(v->getPtr());
  fprintf(out,"native(\"%s\")",d->name);
}

/** \page objser Note on serialisation of objects
 * Serialisation of objects is somewhat ... hairy. If an object is serialised as part of the right-hand side of an expression,
 * it must already be in the serialisation hash - i.e. have a name. Then Object::serialise() is called. To get the object into
 * the hash, we have to call serialiseAssignment() on the object. This is broken into two parts. The first can be overriden by
 * custom objects : the object is created, or cloned if it has a superclass/parent. Custom objects will override this to provide
 * calls to their constructors or special cloners. They should then write code to output assignments for any custom properties.
 * Finally, ObjectType::serialisePropertyHash is called. This outputs the standard properties.
 */

void ObjectType::serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const {
  v->d.o->serialiseAll(s,name,out);
}



// this is used if the object is being used as, say, a key
void IterableType::serialise(class Serialiser *s,FILE *out,Value *v) const {
    Object *o = v->d.o;
    const char *n = s->getHash((u32)o);
    if(!n)
        throw Exception("serialisation error: object must be in hash if serialised as rhs");
    
    fputs(n,out);
}


void DictionaryType::serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const {
    char buf[32];
    Dict *d = v->d.dict;
    
    // output header
    fprintf(out,"%s%s = dict()\n",s->indents(),name);
    // add to the hash
    s->setHash((u32)d,name);
    
    s->lana->recreateIndent++;
    
    const char *indents = s->indents();
    
    IteratorPtr<Value *> iterator(d->createKeyIterator(false));
    for(iterator->first();!iterator->isDone();iterator->next()){
        Value *key = iterator->current();
        Value *v = d->get(key);
        
        // we need to make sure the value, if a complex thing, is in
        // the hash - this may involve serialising it first.
        
        char vnamebuf[32];
        const char *vname;
        if(v->type->serHash){
            const char *n = s->getHash(v->d.u);
            if(n){
                vname = n;
            } else {
                static int valct=0;
                sprintf(vnamebuf,"dictval%03d",valct++);
                s->serialiseValue(out,v,vnamebuf);
                vname = vnamebuf;
            }
        }
        
        
        // here, we need to serialise the key data. This might get a bit hairy.
        // If the data is a non-serialisable-hash type, we can just output the
        // getStr() value, otherwise we need to make sure it actually is in the hash
        if(key->type->serHash){
            const char *keyname;
            u32 k = key->d.u;
            const char *n = s->getHash(k);
            if(!n){
                // if the key isn't a value in the hash, serialise it
                static int keyct=0;
                sprintf(buf,"dictkey%03d",keyct++);
                s->serialiseValue(out,key,buf);
                // and use the name
                keyname = buf;
            } else {
                // if it is in the hash, just use the name there.
                keyname = n;
            }
            // output the key 
            fprintf(out,"%s%s[%s] = ",indents,name,keyname);
        } else {
            fprintf(out,"%s%s[",indents,name);
            key->type->serialise(s,out,key); // easy case
            fputs("] = ",out);
        }
        
        // get and serialise the value
        if(v->type->serHash)
            fputs(vname,out);
        else
            v->type->serialise(s,out,v);
        fputs("\n",out);
    }
    
    
    
    s->lana->recreateIndent--;
}


const char *Serialiser::preSerialise(FILE *out, u32 target,Type *tp){
    char buf[32];
    if(!tp->serHash)
        throw Exception(NULL).set("cannot preserialise the type '%s'",tp->getName());
    
    const char *t = getHash(target);
    if(t)
        return strdup(t);
    else {
        // firstly, is it a global which we haven't got round to yet? A bit slow, this - 
        // we have to scan all the globals
        int i = lana->globs->findByHashableValue(target);
        if(i>=0){
            // this will write the new name/val tuple to the hash
            int desc = lana->globs->getName(i);
            const char *name = lana->consts->getStr(desc);
            serialiseValue(out,lana->globs->get(i),name);
            return strdup(name);
        }
        
        // it's neither in the hash nor a global
        
        // generate a name
        static int tmpct=0;
        sprintf(buf,"t%03d",tmpct++);
        
        // generate a value - this is just a temporary. No need for refcount increment,
        // since we're going to throw this away very quickly.
        
        Value v;
        v.setOther(tp,(void *)target);
        
        // output that value under that name, hashing it.
        serialiseValue(out,&v,buf);
        return strdup(buf);
    }
}


void ListType::serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const {
}

