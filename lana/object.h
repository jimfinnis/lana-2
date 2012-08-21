#ifndef __OBJECTx_H
#define __OBJECTx_H

/**
 * @file
 * Declarations of classes used for handling Lana objects, and for
 * interfacing with C++ objects.
 */

#include "api.h"
#include "value.h"
#include "intkeyedhash.h"


namespace lana {




/// a full Lana object : native functions can run in it, as with a Host,
/// but it can also run native methods and store properties (which can
/// include user methods.) Subclass this to create your own native objects
/// usable with Lana, and create subclasses in Lana by cloning these.

class Object : public Iterable {
public:
    
    Object(API *a);
    virtual ~Object();
    
    /// override this to create special properties, but remember to serialise them
    virtual void setprop(int id,Value *v);
    // scan for this property in this object and up through
    // its parents until we find it, returning NULL if we don't.
    /// override this to create special properties, but remember to serialise them
    virtual Value *getprop(int id);
    
    /// clone an object. Subclasses of object will need to override
    /// this so the appropriate subclass is created and any data copied.
    /// We do need to tell it the API, though.
    virtual Object *clone(API *a){
        Object *o = new Object(a);
        o->makeCloneOf(this);
        return o;
    }
    
    /// this adds a native method to the properties. Typically you'll allocate the
    /// NativeFuncData statically or in the constant area, and set it using the set()
    /// method. That'll avoid you adding new NativeFuncData structures for every single
    /// instance! Of course, it might be even better to write a method to clone your object
    /// and create a prototype. You might also use registerNativeMethod(), which will allocate
    /// the NativeDataFunc for you and register it with the API.
    
    void registerMethod(const char *name,NativeFuncData *nd);
    
    /// another version of registerMethod(), which calls the API to allocate the NativeDataFunc.
    /// Note that "name" will be prefixed with the current name prefix (set by API::setNamePrefix())
    /// inside the APIs global list. So remember to set the name prefix!
    
    void registerNativeMethod(const char *name,int argc,bool returns,
                              HOSTMETHOD m);
    
    
    /// return the value iterator of the properties
    virtual Iterator<Value *> *createValueIterator(){
        return properties.createValueIterator();
    }
    /// return the iterator for my properties' keys
    virtual Iterator<Value *> *createKeyIterator(bool incycledetection);    
    
    /// our properties
    IntKeyedHash<Value> properties;
    
    /// return the superclass
    Object *getSuper(){
        return parent;
    }
    
    /// the type identifier is a pointer to a type object for
    /// things like lists and dicts. For pure objects, it's vtObject.
    /// Cloners and creators should set it accordingly when a new
    /// object is stacked.
    Type *type;
    

    /// override this to provide correct serialisation of 
    /// constructors, cloners and custom properties. This will
    /// (at least) write out the appropriate line to create the object
    /// and assign it to a variable. It may also add code to initialise
    /// custom properties.

    virtual void serialise(
			   class Serialiser *s, //!< serialiser object
			   const char *name, //!< name of variable to assign to
			   FILE *out //!< output stream
			   );

    /// this serialises the property hash of this object recursively

    void serialisePropertyHash(Serialiser *s,const char *name,FILE *out);

    /// really does the bipartite serialisation of objects : see \ref objser.
    /// this serialises the object, calling its (possibly overriden) serialiser,
    /// setting a name in the serialiser hash, then serialising its property hash.
    void serialiseAll(Serialiser *s, const char *name, FILE *out);
    
    
    /////////// behaviour for dealing with garbage collection, involving
    /////////// links through the "parent" pointer.
    
    /// Objects can form loops involving inheritance; deal with it.
    /// Object subclasses must also call this if they subclass it.
    virtual void traceAndMove(class CycleDetector *cycle);
    /// see traceAndMove()
    virtual void decReferentsCycleRefCounts();
    /// see traceAndMove()
    virtual void clearZombieReferences();
    
    
protected:
    /// make this object a clone of another by setting the parent
    /// value. Make DAMN SURE that parent object isn't deleted :)
    
    void makeCloneOf(Object *o){
        
        if(type != o->type){
            throw Exception("cannot make an object of a different type into a clone");
        }
        if(parent){
            if(parent->decRefCt()){
                dfprintf("deleting parent %lx\n",parent);
                delete parent;
            }
        }
        parent = o;
        parent->incRefCt();
    }
    
private:    
    /// the object we look in for properties we don't have. Should really
    /// be called 'superclass', I suppose :)
    Object *parent;
};


/// the type object for a reference to an object, complete with an implementation fo
/// the [] operator, and an implementation for properties in the object.

struct ObjectType : public IterableType {
    virtual class Object *getObj(Value *v) {
        return v->d.o;
    }
    virtual bool makeSQBRef(Value *v,Value *item,Value *idx) {
        v->setPropRef(item->d.o,idx->getHash());
        return true;
    }
    
    virtual int getSize(Value *v);
    
    
    virtual bool makePropRef(Value *v,Value *item,u32 prop);
    
    /// serialises assignment to an object; just calls doSerialise()
    virtual void serialiseAssignment(Serialiser *s,const char *name,FILE *out,Value *v) const;


    /// serialises the property hash of an object, assuming the object has
    /// already been created.
    void serialisePropertyHash(Serialiser *s,const char *name,FILE *out,Object *o) const;
    

};

/// the type for references to properties of objects
struct PropRefType : public Type {
    virtual const char *repr(const Value *v) const {
        startRepr();
        sprintf(buf+strlen(buf),"%x(ct%d)/%d",(u32)v->d.o,
                v->d.gc->refct,v->d2.u);
        return buf;
    } 
    virtual Value *deref(Value *v);
    virtual void store(Value *ref,Value *v);
    virtual bool isDefinedReference(Value *v);
    virtual bool deleteElement(Value *v);

};

}

#endif /* __OBJECTx_H */
