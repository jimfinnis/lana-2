#include "iterobj.h"

using namespace lana;

#define MT(xx) (lana::HOSTMETHOD)&IteratorObject::xx

IterObjectType::IterObjectType(API *a) : ObjectType() {
    proto = new IteratorObject(a);
        
    a->setNamePrefix("iterobj$");
    proto->registerNativeMethod("first",0,false,MT(first));
    proto->registerNativeMethod("next",0,false,MT(next));
    proto->registerNativeMethod("isDone",0,true,MT(methodIsDone));
    proto->registerNativeMethod("current",0,true,MT(methodCurrent));
    proto->incRefCt(); // never die!
}


IteratorObject *IteratorObject::create(API *a){
    IteratorObject *iter = new IteratorObject(a);
    
    /// make it a clone without using 'clone'
    iter->makeCloneOf(((IterObjectType *)Types::vtIterObj)->proto);
    iter->type = Types::vtIterObj;
    return iter;
}

IteratorObject *IteratorObject::create(API *a,Iterator<Value *> *i){
    IteratorObject *o = create(a);
    o->init(i);
    return o;
}


IteratorObject *IteratorObject::create(API *a, Value *s,bool keys){
    // check the type of iterable and create an iterator if we can
    Iterator<Value *> *i = s->type->createIter(s,keys);
    if(!i)
        throw Exception("cannot obtain iterator");
    
    IteratorObject *o = create(a);
    
    if(s)
        o->source = *s; // will incref, dtor will decref
    
    o->init(i);
    return o;
}
