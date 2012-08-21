
#ifndef __USEROBJ_H
#define __USEROBJ_H

#include "lana/object.h"
#include "lana/consts.h"
#include "lana/cycle.h"
#include "lana/language.h"
#include "lana/ser.h"

#define MT(xx) (lana::HOSTMETHOD)&TestObject::xx

class TestObject : public lana::Object {
    
public:
    TestObject(lana::API *a) : lana::Object(a) {
        testVal = 0;
        linkProperty=NULL;
        
        // all objects created with this method have these methods.
        // In reality, it's better to use a prototype object.
        
        a->setNamePrefix("test$");
        registerNativeMethod("test1",1,false,MT(testMethod1));
        registerNativeMethod("test2",0,true,MT(testMethod2));
        registerNativeMethod("testLink",1,true,MT(testLink));
        
    }
    
    // destructor has to clear any old references, so call decRefCt() on any GC
    // objects we have pointers to.
    
    ~TestObject(){
        if(linkProperty) // delete old reference!
            linkProperty->decRefCt();
    }
    
    // call this to register the new object with the API
    
    static void reg(lana::API *a){
        // these are the ID's for custom properties we have
        
        printf("test object registered\n");        
        testPropertyID = a->getID("i"); // test integer
        linkPropertyID = a->getID("link"); // test prop, link to one of these!
        
        // this is the creation function
        
        a->globalNativeFunction("createTest",0,true,
                                &TestObject::create);
    }
    
    // custom property accessor - there's a clause in this for each custom property
    virtual void setprop(int id,lana::Value *v){
        if(id == testPropertyID)
            testProperty = v->getInt();
        else if(id == linkPropertyID){
            if(linkProperty) // delete old reference!
                linkProperty->decRefCt();
	    // HAS TO BE an object of this type, but there's no way of checking.
            linkProperty = (TestObject *)v->getObj(); 
            linkProperty->incRefCt();
        } else
            lana::Object::setprop(id,v);
    }
    
    // custom property accessor - there's a clause in this for each custom property
    virtual lana::Value *getprop(int id){
        // here, we're using the VM's cyclic value buffer to store temporary values, which will
        // wrap the native properties. Another approach would be to have the properties in this
        // class as actual Lana values. This is probably preferable ;)
        lana::Value *v;
        if(id == testPropertyID) {
            // this is a strange property - getting it returns twice
            // the value it was set to!
            v = api->getTempValue();
            v->setInt(testProperty*2);
        } else if(id == linkPropertyID) {
            v = api->getTempValue();
            v->setObj(linkProperty);
        } else
            return lana::Object::getprop(id);
	return v;
    }
    
    // Objects which could possibly be part of cycles should override the following three
    // methods. See lana::GarbageCollected for how this works.
    
    // clear references to objects which are marked as about to be deleted
    virtual void clearZombieReferences(){    
        Object::clearZombieReferences();
        if(linkProperty && linkProperty->gc_refs == 0xffff)
            linkProperty=NULL;
    }

    // decrement the cycle detector reference counts of all references objects
    virtual void decReferentsCycleRefCounts(){
        Object::decReferentsCycleRefCounts();
        if(linkProperty)
            linkProperty->gc_refs--;
    }

    // move those objects which have a ref count of zero into the newlist, and recurse for
    // properties of those objects too.
    virtual void traceAndMove(lana::CycleDetector *cycle) {
        Object::traceAndMove(cycle);
        if(linkProperty && linkProperty->gc_refs==0) {
            dfprintf(" custom trace moving %x into new list\n",linkProperty);
            cycle->move(linkProperty);
	    cycle->traceAndMoveEntity(linkProperty);
        }
    }
    
    // this is a custom serialisation method, which assumes these
    // objects are not subclasses.
    virtual void serialise(lana::Serialiser *s,const char *name,FILE *out) {
        fprintf(out,"%s%s = createTest()\n",s->indents(),name);
    }

private:
    // test method - one argument. Set internal value (NOT property) to input*40
    void testMethod1(){
        testVal = api->popInt() * 40;
    }
    
    // test method - get value of internal property
    void testMethod2(){
      api->pushInt(testVal*20);
    }
    
    // test method - ensure link object == argument
    void testLink() {
        Object *o = api->popObj();
        api->pushBool(o==linkProperty);
    }
        
    
    static void create(lana::API *a){
        TestObject *o = new TestObject(a);
        a->pushObj(o);
    }
    // a property
    int testProperty;
    // an object property! Scary.
    TestObject *linkProperty;
    // test value for methods
    int testVal;
    
    // the property IDs
    static int testPropertyID,linkPropertyID;
};

#undef MT
#define MT(xx) (lana::HOSTMETHOD)&MyThing::xx

class Square : public lana::Object {
private:
    /// Constructor also takes API pointer and calls Object ctor.
    Square(lana::API *api,int t, int c) : lana::Object(api) {
        tex = t;
        col = c;
    }
    
    /// pointer to prototype
    static Square *proto;
    
    int tex,col;
    
public:
    void draw(int x,int y,int w,int h){
//        SomeGraphicsEngine::draw(x,y,w,h,tex,col);
    }
    
    static Square *create(lana::API *api, int t, int c){
        Square *s = new Square(api,t,c);
        s->makeCloneOf(proto);
        return s;
    }
    
    static void init(lana::API *api) {
        proto = new Square(api,0,0);
        
        proto->incRefCt();
    }
};
    
    

#endif /* __USEROBJ_H */
