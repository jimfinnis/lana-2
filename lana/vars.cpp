#include <stdio.h>
#include <stdlib.h>
#include "vars.h"
#include "consts.h"
#include "exception.h"
#include "pool.h"

using namespace lana;

Vars::Vars(Constants *c) {
    consts = c;
}

Vars::~Vars(){
    clear();
}

void Vars::clear(){
    hash.clear();
    pool.empty();
}

int Vars::getName(int id){
    vardata *v = pool.get(id);
    // there is no guarantee that v is valid here - we could have asked for an unallocated slot.
    return v->nameAndFlags & 0x0fffffff;
}

int Vars::getFlags(int id){
    vardata *v = pool.get(id);
    // there is no guarantee that v is valid here - we could have asked for an unallocated slot.
    return (int)(v->nameAndFlags >> 28);
}

void Vars::setFlags(int id, int flags){
    vardata *v = pool.get(id);
    v->nameAndFlags = (v->nameAndFlags&0x0fffffff) | (flags<<28);
    
}

int Vars::find(int namedesc){
    
    if(hash.find(namedesc)){
        return *hash.getval();
    } else
        return -1;
}

int Vars::find(const char *name){
    // find the descriptor of the given string
    
    int namedesc = consts->findString(name);
    if(namedesc<0)
        return -1; // it hasn't got one, this variable can't exist
    
    return find(namedesc);
}

int Vars::create(int namedesc,int flags){
    if(hash.find(namedesc))
        throw Exception("var already exists");
    else {
        int slot = pool.alloc();
        *hash.set(namedesc) = slot;
	vardata *v = pool.get(slot);
	v->nameAndFlags = namedesc | (flags<<28);

        return slot;
    }
    return -1;
}

Value *Vars::get(int i){
    return &pool.get(i)->v;
}

int Vars::findByHashableValue(u32 target){
    IteratorPtr<int> iterator(pool.createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        int i = iterator->current();
        Value *v = &pool.get(i)->v;
        if(v->type->serHash && v->type->getHash(v) == target)
            return i;
    }
    return -1;
}

void Vars::dump(){
    IteratorPtr<int> iterator(pool.createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        int i = iterator->current();
        vardata *v = pool.get(i);
        const char *name = consts->getStr(v->nameAndFlags & 0x0fffffff);
        printf("%s (%d) = %lx\n",name,v->nameAndFlags,i);
    }
}




Iterator<int> *Vars::createIterator() {
    return pool.createIterator();
}


void GlobalVars::setSystemGlobMarker(){
    userFlag = true;
    IteratorPtr<int> iterator(pool.createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        int i = iterator->current();
        setFlags(i,0);
    }
}
