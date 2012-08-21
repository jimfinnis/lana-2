/**
 * @file
 * Implementation of a list based on ArrayList.
 * 
 */

#ifndef __LISTOBJ_H
#define __LISTOBJ_H


#include "value.h"
#include "arraylist.h"
#include "object.h"

namespace lana {

/// this is a garbage-collected array list, based on the ArrayList class,
/// it is incomplete!
class List : public Object {
    friend struct ListType;
    
private:
    // private constructor, use create() instead.
    List(class API *a);
    
public:
    virtual ~List();
    
    // use this to create lists
    static List *create(class API *a);
    
    virtual Iterator<Value *> *createValueIterator(){
        return list->createValueIterator();
    }
    
    // keys here can be anything, so we need to return an iterator
    // if we're in cycle detection too. That's why we're ignoring that
    // argument.
    
    virtual Iterator<Value *> *createKeyIterator(bool incycledetection){
        return list->createKeyIterator();
    }
    
    /// clone the list and its contents
    virtual Object *clone(class API *a){
        List *l = create(a);
        l->makeCloneOf(this);
        
        // copy the data into the new object
        int ct = list->count();
        for(int i=0;i<ct;i++)
            l->set(i,list->get(i));
        return l;
    }
    
    /// change the value of an existing slot
    void set(int i,Value *v){
        list->set(i,v);
    }
    
    /// get the value of an existing slot (or null)
    Value *get(int i){
        return list->get(i);
    }
    
    /// remove an existing item from the list
    bool remove(int i){
        return list->remove(i);
    }
    
    /// native function for 'append(x)' and 'push(x)'
    void methodAppend();
    
    /// native function for 'insert(i,x)'
    void methodInsert();
    
    /// native function for 'v=pop()'
    void methodPop();
    
    /// native function for 'remove(i)'
    void methodRemove();
    /// native function for 'v=size()'
    void methodSize();
    
    /// native function for "v=peek()"
    void methodPeek();
    
    /// native function for "v=unshift()"
    void methodUnshift();
    
    /// native function for "shift(x)"
    void methodShift();
    
    // also need to add the special get properties
    // for size and capacity
    
    ArrayList *list;
};

/// the type object for lists

struct ListType : public ObjectType {
    /// create a square-bracket-ref value for an object. This is set
    /// into v. The list is in item, and the index is in idx. The index
    /// must be evaluatable as an integer (getInt will be called from
    /// inside setListRef().
    
    virtual bool makeSQBRef(Value *v,Value *item,Value *idx){
        v->setListRef(item->d.list,idx);
        return true;
    }
    
    /// serialise an assignment of a list to a variable
    
    virtual void serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const;
    
    // get the size of a list.
    virtual int getSize(Value *v);
    
    /// the prototype list
    List *proto;
    
    /// create the list type, creating the prototype
    
    ListType(API *a);
    
    /// delete the list type, which deletes the prototype
    virtual ~ListType(){
        delete proto;
    }
};

/// the type object for a reference to an item in a list

struct ListRefType : public Type {
    virtual const char *repr(const Value *v) const;
    /// store a value in a list reference
    virtual void store(Value *ref,Value *v);
    /// dereference a value!
    virtual Value *deref(Value *v);
    /// is there a value stored in the list under
    /// this key (v is a valid listreftype)
    virtual bool isDefinedReference(Value *v);
    /// delete an element from the list
    virtual bool deleteElement(Value *v);
};

    
}    

#endif /* __LISTOBJ_H */
