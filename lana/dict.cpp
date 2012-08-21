#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "api.h"
#include "language.h"
#include "dict.h"
#include "cycle.h"
#include "pool.h"
#include "iterobj.h"

#define MT(xx) (lana::HOSTMETHOD)&lana::Dict::xx

DictionaryType::DictionaryType(API *a): ObjectType(){
    proto = new Dict(a);
    
    /*     We have no natives for Dict right now, but I'm
     *     leaving this code here as a reference to how to 
     *     add native methods as properties to a prototype.
    
    a->setNamePrefix("dict$");
    proto->registerNativeMethod("size",0,true,MT(methodSize));
    proto->registerNativeMethod("del",1,true,MT(methodDel));
     
     */
    proto->incRefCt(); // never die!



}

Dict *Dict::create(API *a) {
    // make a new dictonary
    Dict *d = new Dict(a);
    
    // make this new dictionary a clone of the prototype dictionary,
    // stashed handily inside the dictionary type object. We don't
    // use 'clone()' because there's nothing to clone, data-wise.
    
    d->makeCloneOf(((DictionaryType *)Types::vtDictionary)->proto);
    // but we do set the type ID now.
    d->type = Types::vtDictionary;
    
    return d;
}

static Pool<Value,1024> keyPool;

int Dict::allocKey(){
    return keyPool.alloc();
}

void Dict::freeKey(int id){
    keyPool.free(id);
}

Value *Dict::getKey(int id){
    return keyPool.get(id);
}


// dictionary type methods

int DictionaryType::getSize(Value *v){
    return v->d.dict->getSize();
}


const char *DictRefType::repr(const Value *v) const {
    Value *k = Dict::getKey(v->d2.i);
    strcpy(buf2,k->repr());
    startRepr();
    sprintf(buf+strlen(buf),"%p/(ct%d)[%s]",v->d.o,v->d.gc->refct,buf2);
    return buf;
}

void DictRefType::store(Value *ref,Value *v){
    Value *k = Dict::getKey(ref->d2.i);
    ref->d.dict->set(k,v);
}
Value *DictRefType::deref(Value *v){
    v = v->d.dict->get(v->d2.i);
    if(!v)
        throw Exception("unset value in dictionary");
    return v;
}

bool DictRefType::isDefinedReference(Value *v){
    return v->d.dict->get(v->d2.i) ? true : false;
}

bool DictRefType::deleteElement(Value *v){
    Value *key = Dict::getKey(v->d2.i);
    return v->d.dict->del(key);
    
}
