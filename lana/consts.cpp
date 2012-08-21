#include <stdio.h>
#include <stdlib.h>

#include "value.h"
#include "consts.h"

using namespace lana;

Constants::Constants() {
    constantArea = new Growable(1024,1024,4);
    props.setUp(this);
}

Constants::~Constants() {
    IteratorPtr<ConstDesc *> iterator(createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        ConstDesc *e = iterator->current();
        if(e->getType() == CT_FUNC){
            char *ptr = *(char **)e->get();
            free(ptr);
        }
        
    }
    
    delete constantArea;
}

constid Constants::create(ConstType type,const void *p,u16 flags,u32 size) {
    
    // round up to 4, because that's what growable will do
    u32 allocsize = (size+3L)& ~3L;
    
    if(allocsize>=65536L)
        throw Exception("cannot allocate constant more than 64K in size");
    
    // allocate room for the data and descriptor
    
    u32 off = constantArea->allocate(allocsize+sizeof(ConstDesc));
    
    // get a pointer to the descriptor
    ConstDesc *cd = (ConstDesc *)constantArea->get(off,allocsize+sizeof(ConstDesc));
    // write the descriptor data
    cd->set(type,flags,allocsize);
    // write the constant data if any
    if(p){
        void *out = cd->get();
        memcpy(out,p,size); // only copy the actual number of bytes
    }
    
    // return the constant descriptor index
    return (constid)(off>>2);
}

constid Constants::findString(const char *s){
    IteratorPtr<ConstDesc *> iterator(createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        ConstDesc *e = iterator->current();
        if(e->getType() == CT_STRING){
            char *ptr = (char *)e->get();
            if(!strcmp(ptr,s)){
                return getIdx(e);
            }
        }
        
    }
    return NOTFOUND;
}

constid Constants::findOrCreateString(const char *s){
    constid d = findString(s);
    if(d == NOTFOUND) {
        d = create(CT_STRING,s,0,strlen(s)+1);
    }
    return d;
}

constid Constants::createComment(const char *s,int pos){
    constid d = create(CT_STRING,s,pos,strlen(s)+1+sizeof(short)); // allow space for position
    
    char *mem = (char *)(get(d)->get());
    *(short *)mem = (short)pos;
    mem+=sizeof(short);
    strcpy(mem,s);
    
    return d;
}

constid Constants::findInt(int n){
    IteratorPtr<ConstDesc *> iterator(createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        ConstDesc *e = iterator->current();
        if(e->getType() == CT_INT){
            int *ptr = (int *)e->get();
            if(*ptr == n) {
                return getIdx(e);
            }
        }
    }
    return NOTFOUND;
}

constid Constants::findOrCreateInt(int n){
    constid d = findInt(n);
    if(d == NOTFOUND)
        d = create(CT_INT,&n,0,sizeof(int));
    return d;
}

constid Constants::findFloat(float n){
    IteratorPtr<ConstDesc *> iterator(createIterator());
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        ConstDesc *e = iterator->current();
        if(e->getType() == CT_FLOAT){
            float *ptr = (float *)e->get();
            if(*ptr == n) {
                return getIdx(e);
            }
        }
    }
    return NOTFOUND;
}

constid Constants::findOrCreateFloat(float n){
    constid d = findFloat(n);
    if(d == NOTFOUND)
        d = create(CT_FLOAT,&n,0,sizeof(float));
    return d;
}



/*
 * 
 * Iteration
 * 
 */

/// this is the iterator subclass for the constant area, private to this class

class ConstIterator : public Iterator<ConstDesc *> {
    Constants *consts;
    constid id;
    ConstDesc *currentv;
public:
    ConstIterator(Constants *c){
        consts = c;
        currentv = NULL;
    }
    virtual void first(){
        id = 0;
        if(consts->isValid(id))
            currentv = consts->get(id);
    }
    virtual void next(){
        if(consts->isValid(id)){
            u32 offset = id<<2;
            offset += currentv->size + sizeof(ConstDesc);
            id = offset>>2;
            if(consts->isValid(id)){
                currentv = consts->get(id);
            } else {
                currentv = NULL;
            }
        }else
              currentv=NULL;
    }
    virtual bool isDone() const {
        return currentv?false:true;
    }
    
    virtual ConstDesc *current() {
        return currentv;
    }
};



Iterator<ConstDesc *>* Constants::createIterator(){
    return new ConstIterator(this);
}



constid PropIDs::add(const char *id){
    return consts->findOrCreateString(id);
}

