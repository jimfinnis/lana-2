/**
 * @file
 * Implementation of dictionary based on Hash.
 * 
 */

#ifndef __DICT_H
#define __DICT_H

#include "value.h"
#include "hash.h"
#include "object.h"

namespace lana {

/// this is a garbage-collected dictionary, based on the Hash class.

class Dict : public Object {
    friend struct DictionaryType;
private:
    /// note - private ctor! Use create(), which will create
    /// a prototype if necessary and clone it.
    Dict(class API *a) : Object(a){}
    
    
public:
    virtual ~Dict() {}
    
    /// use this to create dictionaries
    static Dict *create(class API *a);
    
    /// clones of dictionaries should copy the dictionary contents
    
    virtual Object *clone(class API *a){
        Dict *d = create(a);
        d->makeCloneOf(this);
        
        // copy the data into the new object
        
        IteratorPtr<Value *>iterator(createKeyIterator(false));
        for(iterator->first();!iterator->isDone();iterator->next()){
            Value *key = iterator->current();
            Value *val = get(key);
            d->set(key,val);
        }
        return d;
    }
    
    virtual Iterator<Value *> *createValueIterator(){
        return hash.createValueIterator();
    }
    
    virtual Iterator<Value *> *createKeyIterator(bool incycledetection){
        // keys here can be anything, so we need to return an iterator
        // if we're in cycle detection too. That's why we're ignoring that
        // argument.
        return hash.createKeyIterator();
    }
    
    /// return pointer to value stored in dict under this key,
    /// or null.
    Value *get(int keyID){
        Value *k = Dict::getKey(keyID);
        return get(k);
    }
    
    /// direct key from value key
    Value *get(Value *k){
        if(hash.find(k))
            return hash.getval();
        else
            return NULL;
    }
    
    /// copy a value into the hash
    void set(Value *k,Value *v){
        hash.set(k,v);
    }
    
    /// delete an item by key, returning true if done
    bool del(Value *k){
        return hash.del(k);
    }
    
    /// get number of slots filled with active data
    int getSize(){
        return hash.used;
    }
    
    /// allocate a key from the key pool - returns a key handle
    static int allocKey();
    /// free a key to the key pool, given its handle
    static void freeKey(int id);
    
    /// get a key given its handle
    static Value *getKey(int id);
    
private:
    
    Hash hash;
};

/// a type object for a reference to a dictionary, complete with an implementation for the [] operator.

struct DictionaryType : public ObjectType {
    
    DictionaryType(API *a);
    virtual ~DictionaryType(){
        delete proto;
    }
    
    
    virtual bool makeSQBRef(Value *v,Value *item,Value *idx){
        v->setDictRef(item->d.dict,idx);
        return true;
    }
    
    virtual void serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const;
    virtual int getSize(Value *v);
    
    /// the dictionary prototype object, used in cloning --- so we only have one copy of the methods etc.
    Dict *proto;
};
                                       

/// the type object for a reference to an item in a dictionary - these are rather complex
/// because values can themselves be keys, so the actual dictref is stored in a pool.

struct DictRefType : public Type {
    virtual const char *repr(const Value *v) const;
    virtual void store(Value *ref,Value *v);
    virtual Value *deref(Value *v);
    virtual bool isDefinedReference(Value *v);
    virtual bool deleteElement(Value *v);
        
};



}
#endif /* __DICT_H */
