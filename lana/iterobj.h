/**
 * @file
 * This file contains the definition of iterator objects - lana
 * native objects which wrap iterators and can therefore be
 * used in for-statements.
 *
 */

#ifndef __ITEROBJ_H
#define __ITEROBJ_H

#include "lana/api.h"
#include "lana/object.h"
#include "cycle.h"

namespace lana {

/// An iterator object is a lana Object wrapped around an Iterator.
/// It's created by 
/// - values()
/// - keys()
/// - range()
/// - implicitly by "for" when an object is passed directly

class IteratorObject : public Object {
    friend struct IterObjectType;
private:
    /// initialiser shared by both constructors
    void init(Iterator<Value *> *i){
        iterator = i;
        
    }
    
    /// private constructor. The iterator prototype has no
    /// actual iterator, so needs to set it to null
    IteratorObject(API *a) : Object(a) {
        iterator=NULL;
    }
    
    /// base creator, used by createIterObj() variants
    static IteratorObject *create(API *a);
    
public:
    
    /// use this to create an iterator object, not the constructor. It wraps an iterator.
    static IteratorObject *create(
                                  API *a,	//!< the API
                                  Iterator<Value *> *i	//!< the iterator we should wrap
                                  );
    
    /// another version that will automatically get an iterator out of a value and wrap it with an object.
    static IteratorObject *create(
                                  API *a,	//!< the API
                                  Value *s,	//!< the value containing the thing from which we'll get the iterator
                                  bool keys	//!< should we get a key or value iterator?
                                  );
    
    /// cloning is not permitted on iterators.
    virtual Object *clone(class API *a){
        throw Exception("cannot clone iterators");
    }
    
    virtual ~IteratorObject() {
        if(iterator)
            delete iterator;
    }
    
    
    // this is both a Lana and C++ method
    void first(){
        iterator->first();
    }
    
    // this is both a Lana and C++ method
    void next(){
        iterator->next();
    }
    
    Value *current() const {
        if(iterator->isDone())
            throw Exception("iterator overrun");
        return iterator->current();
    }
    
    bool isDone(){
        return iterator->isDone();
    }
    
    // these are both Lana native methods
    void methodCurrent() {
        Value *v = api->pushRaw();
        *v = *iterator->current();
    }
    
    void methodIsDone(){
        api->pushBool(iterator->isDone());
    }
    
private:
    /// a copy of the value we were made from; helps the GC to keep this
    Value source;
    // the iterator we are wrapping
    Iterator<Value *> *iterator;
};



/// the type object for a reference to an iterator

struct IterObjectType : public ObjectType {
public:
    IterObjectType(API *a);
    virtual ~IterObjectType(){
        delete proto;
    }
        
    /// the prototype from which we create things
    IteratorObject *proto;
};


}


#endif /* __ITEROBJ_H */
