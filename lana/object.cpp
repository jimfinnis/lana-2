#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "language.h"
#include "object.h"
#include "cycle.h"
#include "ser.h"

using namespace lana;

Object::Object(API *a) : Iterable(a) {
    a->cycle->add(this);
    type = Types::vtObject;
    parent = NULL;
}

Object::~Object(){
//    printf("DELETING %x\n",this);
    api->cycle->remove(this);
    if(parent && parent->decRefCt())
        delete parent;
}

void Object::setprop(int id,Value *v){
    // right, here we go. Set the property in the object.
    
    // the property must be a valid constant
    if(!api->lana->consts->isValidPropID(id))
        throw Exception("setting of invalid property - must be an ID");
    
    Value *p = properties.set(id); // then we set.
    *p = *v; //!< copy ctor should run so strings will clone
}

Value *Object::getprop(int id){
    
    // scan for this property in this object and up through
    // its parents until we find it, returning NULL if we don't.
    
    for(Object *o=this;o;o=o->parent){
        if(o->properties.find(id))
            return o->properties.getval();
    }
    return NULL;
}

// object type methods

Value *PropRefType::deref(Value *v){
    Object *o = v->d.o;
//    printf("deref of %lx\n",o);
    u32 k = v->d2.u;
    //    v = v->d.o->getprop(v->d2.u);
    v = o->getprop(k);
    if(!v)
        throw Exception("undefined property");
    return v;
}

void PropRefType::store(Value *ref,Value *v){
    ref->d.o->setprop(ref->d2.u,v);
}


/// an iterator for running through property keys and converting them
/// to Values. Note that we don't need to keep the object pointer
/// and inc/dec refcounts - that's handled by the IteratorObject
/// wrapped around this.

class PropertyKeyIterator : public Iterator<Value *> {
public:
    PropertyKeyIterator(class Object *o){
        v.setInt(0); // initialise value to an int.. 
        pct = &v.d.u; // .. and use pointer - it's quicker
        
        iterator = o->properties.createKeyIterator();
    }
    
    virtual ~PropertyKeyIterator(){
        delete iterator;
    }
    
    virtual void first(){
        iterator->first();
    }
    
    virtual void next(){
        iterator->next();
    }
    
    virtual bool isDone() const{
        return iterator->isDone();
    }
    
    virtual Value *current(){
        *pct = iterator->current();
        return &v;
    }
    
private:
    Value v;
    Iterator<u32> *iterator;
    u32 *pct; //!< pointer to u32 inside value
};


Iterator<Value *> *Object::createKeyIterator(bool incycledetection){
    // return null if we're in cycle detection, because none of the keys here are GC object - they're all
    // just integers and so cannot form parts of cycles.
    if(incycledetection)
        return NULL;
    
    return new PropertyKeyIterator(this);
}


bool ObjectType::makePropRef(Value *v,Value *item,u32 prop){
    Object *o = item->d.o;
    
    // handle special props first
    if(IterableType::makePropRef(v,item,prop)) // try the standard GC props
        return true;
    
    // otherwise it's a user property 
    v->setPropRef(item->d.o,prop);
    return true;
}

int ObjectType::getSize(Value *v){
    Object *o = v->d.o;
    return o->properties.used;
}

bool PropRefType::isDefinedReference(Value *v){
    return v->d.o->getprop(v->d2.u) ? true : false;
}

bool PropRefType::deleteElement(Value *v){
    Object *o = v->d.o;
    return o->properties.del(v->d2.u);
}


void Object::registerMethod(const char *name, NativeFuncData *nd){
    int id = api->getID(name);
    Value *p = properties.set(id); // then we set.
    p->setNativeMethodRef(this,nd);
}

void Object::registerNativeMethod(const char *name,int argc,bool returns,HOSTMETHOD m){
    NativeFuncData *nd = api->registerNativeMethod(name,argc,returns,m,NULL);
    registerMethod(name,nd);
}



void Object::traceAndMove(CycleDetector *cycle){
    if(parent)
        cycle->traceAndMoveEntity(parent);
}

void Object::decReferentsCycleRefCounts(){
    if(parent)
        parent->gc_refs--;
}

void Object::clearZombieReferences(){
    // this basically says that if my parent class is also about to be deleted
    // (has been "maxreffed" in detect()) then just set it to null. Otherwise
    // we could end up with a free-twice.
    if(parent && parent->gc_refs == 0xffff)
        parent = NULL;
}



void Object::serialiseAll(Serialiser *s, const char *name, FILE *out) {
  serialise(s,name,out);
    // add to the hash
  s->setHash((u64)this,name);
  serialisePropertyHash(s,name,out);
}

void Object::serialisePropertyHash(Serialiser *s,const char *name,FILE *out) {
    char buf[256];
    const char *indents = s->lana->indents();    
    s->lana->recreateIndent++; // increase indenting!
    indents = s->lana->indents();
    
    // now output the properties
    
    IteratorPtr<u32> iterator(properties.createKeyIterator());
    
    int levtemporaryct=0;
    for(iterator->first();!iterator->isDone();iterator->next()){
        u32 key = iterator->current();
        ConstDesc *desc = s->consts->get(key);
        
        bool tempused=false;
        if(desc->getType()!=CT_STRING)
            throw Exception("invalid property in serialised object");
        
        // build the name
        const char *propname = (char *)desc->get();
        sprintf(buf,"%s.%s",name,propname);
        // if the name is really long, use a special temporary
        if(strlen(buf)>40){
            sprintf(buf,"ptmp%d_%d",s->lana->recreateIndent,levtemporaryct++);
            tempused = true;
        }
        
        // if this fails we're in trouble - there's a property in the iterator
        // which isn't in the iterable :/
        
        if(!properties.find(key))
            throw Exception("hash iterator problem in property serialisation");
        
        Value *v = properties.getval();
        // and serialise. 
        s->serialiseValue(out,v,buf);
        // If we used a temporary because the name was long, write an assignmen
        // to the actual property
        if(tempused)
            fprintf(out,"%s%s.%s = %s\n",indents,name,propname,buf);
    }
    s->lana->recreateIndent--; // decrease indenting!
}


void Object::serialise(Serialiser *s, const char *name, FILE *out) {
   char buf[256];
   static int tmpobjct=0;
   const char *indents = s->lana->indents();

    if(getSuper()){ // must be cloned, not created
        const char *parentname;
        // first we have to make sure the superclass has already been serialised
        parentname = s->getHash((u64)getSuper());
        if(!parentname) {
            // damn, no - we have to serialise it as a temporary. This could result in recursion.
            sprintf(buf,"object%03d",tmpobjct++);
	    getSuper()->serialiseAll(s,buf,out);
            parentname = buf;
        }
        fprintf(out,"%s%s = clone(%s)\n",indents,name,parentname);
    } else {
        fprintf(out,"%s%s = create()\n",indents,name);
    }

}
