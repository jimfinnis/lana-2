/**
 * @file
 * Implementations of many of the Value methods.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"
#include "consts.h"
#include "object.h"
#include "intkeyedhash.h"
#include "dict.h"
#include "list.h"


using namespace lana;

class Constants *Value::consts = NULL;

char Type::buf[1024];
char Type::buf2[1024];

void Value::setStrClone(const char *s){
    
    int len = strlen(s);
    char *t = (char *)malloc(len+1+sizeof(refct_t));
    *(refct_t *)t = 1; // initialise refct to 1
    memcpy(t+sizeof(refct_t),s,len+1);
    
    clr();
    type = Types::vtString;
    d.s = t;
    
}
void Value::setStrConst(constid id){
    clr();
    type = Types::vtStringConst;
    d.u = id;
}

void Value::setObj(Object *o){
    ((GarbageCollected *)o)->incRefCt();
    clr();
    type = o->type;
    d.o = o;
}

/// @todo faster dereferencing!

Value *Value::deref() {
    Value *v=this;
    for(;;) {
        if(!v->type)
            throw Exception("cannot use an undefined value");
        Value *v2 = v->type->deref(v);
        if(!v2)
            break;
        v=v2;
        if(v==this)
            throw Exception("infinite ref loop");
    }
    return v;
}

void Value::setDictRef(Dict *dt,Value *key){
    ((GarbageCollected *)dt)->incRefCt();
    if(key->type == Types::vtDictRef)
        throw Exception("cyclic dict reference");
    clr();
    d.dict = dt;
    d2.i = Dict::allocKey();
    Value *k = Dict::getKey(d2.i);
    *k = *key;
    
    type = Types::vtDictRef;
}

void Value::incDictKeyRef(){
    d.dict->incRefCt();
    Value *k = Dict::getKey(d2.i);
    k->incRef();
}
void Value::decDictKeyRef(){
    if(d.dict->decRefCt()){
        delete d.dict;
    }
    Dict::freeKey(d2.i);
}


void Value::setListRef(List *list,Value *idx){
    ((GarbageCollected *)list)->incRefCt();
    clr();
    d.list = list;
    d2.i = idx->getInt();
    type = Types::vtListRef;
}






Type::Type(){
}
    

bool Type::makePropRef(Value *v,Value *item,u32 prop){
    return false;
}
