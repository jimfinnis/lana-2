
#ifndef __LANAVALUES_H
#define __LANAVALUES_H
/** @file
 * The Value class, representing data values in Lana, and associated data.
 */

#include "basetypes.h"
#include "exception.h"
#include "iterator.h"
#include "opcodes.h"

// uncommenting this will results in lots of debugging stuff
// about reference counting being printed out.
//#define DEBUGREFCT

#ifdef DEBUGREFCT
#define dfprintf printf
#else
#define dfprintf
#endif

namespace lana {


typedef u16 refct_t; //!< reference count - make sure it's unsigned

class Value;

/// a garbage-collected value. Note the required virtual destructor!
/// this contains both the reference count and the next/prev
/// list required to maintain the global container list
/// for cycle detection.

class GarbageCollected {
public:
    GarbageCollected() {
        refct=0;
    }
    
    virtual ~GarbageCollected(){}
    
    /// the reference count
    refct_t refct;
    
    /// the reference count used by the cycle detector. The name
    /// comes from the original doc (see CycleDetector).
    refct_t gc_refs;
    
    /// pointer for maintaining container list
    GarbageCollected *next; 
    /// pointer for maintaining container list
    GarbageCollected *prev;
    
    /// increment the refct, throwing an exception if it wraps
    void incRefCt(){
        refct++;
        dfprintf("++ incrementing count for %x, now %d\n",(u32)this,refct);
        if(refct==0)
            throw Exception("ref count too large");
    }
    
    /// decrement the reference count returning true if it became zero
    bool decRefCt(){
        --refct;
        dfprintf("-- decrementing count for %x, now %d\n",(u32)this,refct);
        return refct==0;
    }
    
    
    /// many GC objects are containers for references to other objects - return a reference
    /// to an iterator iterating over containing these, and you won't need to subclass the methods 
    /// below! This is the values iterator.
    /// \todo rewrite docs for iterators?
    virtual Iterator<class Value *> *createValueIterator(){return NULL;}
    /// many GC objects are containers for references to other objects - return a reference
    /// to a new iterator iterating over the keys for these, and you won't need to subclass the methods 
    /// below! This is the keys iterator - if it returns non-GCs mapped to values, such as
    /// is the case with object properties, it should return NULL when the argument is true.
    virtual Iterator<class Value *> *createKeyIterator(bool incycledetection){return NULL;}
    
    /// deletion prepwork - see CycleDetector::detect(). This should go through any GC objects,
    /// and if the gc_refs field is 0xffff, should clear them (i.e. any objects which were not traced)
    /// Extend for C++ properties which are garbage-collectable.
    virtual void clearZombieReferences(){}
    
    /// decrement gc_refct in all collectable entities I point to.
    /// Extend for C++ properties which are garbage-collectable
    virtual void decReferentsCycleRefCounts(){}
    
    /// trace all collectable entities reachable from this object.
    /// If their gc_refs is zero (i.e. still in the mainlist) move
    /// to the list passed in (the newlist). See CycleDetector for
    /// more details; it's also something best learned by examples!
    /// Extend for C++ properties which are garbage-collectable
    virtual void traceAndMove(class CycleDetector *cycle){}
};

/// this is a sort of "dummy" class which allows us to specify a GarbageCollected,Host mixin as a single class.
/// It's used for iterable objects.

class Iterable : public GarbageCollected,public Host {
public:
    Iterable(class API *a) : GarbageCollected(), Host(a) {}
};


/// Each lana Value has a Type, and each Type has an allocation type
/// specifying how any data attached to it is allocated or deallocated.

enum AllocType{
    /// the value has no attached data
    Unmanaged,
          /// the value has a simple block of memory allocated with malloc(),
          /// the first sizeof(refct_t) bits of which are reference count.
          /// This is how strings work - see \ref Type for details.
          SimpleMalloc, 
          /// the value has a subclass of GarbageCollected attached to it,
          /// but does not refer to any other garbage collected objects 
          /// and so therefore cannot form cycles.
          SimpleNew,
          /// the value has a subclass of GarbageCollected attached to it,
          /// which can itself refer to other garbage collected objects,
          /// leaving us open to the possibility of cycles.
          Complex,
          /// the d2 field contains a pool handle (see dict.h) for a dictionary key, which
          /// is a value. The d field contains a pointer to the dictionary itself.
          DictRefAlloc,
};



/// A Value is of one of these types -
/// remember to add to type name strings in value.cpp if you add to this.
/// Values are of different allocation subtypes, which are encoded into the top bits
/// of the Type - see \ref AllocType.
/// 
/// A note on <b>String values</b>. These are SimpleMalloc,
/// and are implemented using copy-on-write. Copying
/// a string from one value into another just increments
/// a reference to the original string. Using a string modification
/// method creates a new string.
///
///

struct Type {
    static char buf[1024];
    static char buf2[1024];
    
    /// initialisation, just sets up the properties.
    Type();
    virtual ~Type() {}
    
    /// used when the type is created - there's a fluent interface
    /// here, this returns the this pointer. Have a look at vtypes.cpp
    /// Types::createTypes() to see how it's used
    
    Type *set(AllocType at, //!< allocation type
              bool serhash, //!< is this hashed during serialisation? (so a=xxx, b=a serialises correctly); set for COMPLEX types which cannot serialise in an expression only.
              const char *lname, //!< long name
              const char *sname //!< short name
              ) {
        allocType = at;
        longName = lname;
        shortName = sname;
        serHash = serhash;
        return this;
    }
    
    /// controls whether referents of this value are hashed at serialisation, so
    /// a=..., b=a outputs as "a=..., b=a" or "a=..., b=...". This would be silly for ints,
    /// but makes sense for objects.
    bool serHash;
    
    /// the allocation type of this type, describing how its value
    /// stores extra data.
    AllocType allocType;
    
    /// the long name of the type
    const char *longName;
    /// the short name of the type
    const char *shortName;
    
    /// get the appropriate name of the type
    const char *getName(bool wantShort=false)const {
        return wantShort ? shortName : longName;
    }
    
    /// return a hash value. This is overriden by most subclasses; the
    /// default is that types can't be hashed.
    virtual u32 getHash(const Value *v) const {
        throw Exception("cannot get hash of this type");
    }
    
    /// convert the value into a string
    virtual char *getStr(const Value *v) const;
    /// convert the value into a pointer (typically just returns d.s)
    virtual char *getPtr(const Value *v) const;
    /// get an internal, debugging representation of the value
    virtual const char *repr(const Value *v) const;
    
    /// get the value as a boolean
    virtual bool getBool(const Value *v) const {
        throw Exception("expected a boolean");
    }
    
    /// get the value as an object
    virtual class Object *getObj(Value *v) {
        throw Exception("expected an object");
    }
    
    /// get the value as an integer
    virtual int getInt(const Value *v) const {
        throw Exception("cannot convert to integer");
    }
    
    /// get the value as a float
    virtual float getFloat(const Value *v) const {
        throw Exception("cannot convert to float");
    }          
    
    /// is this a numeric type?
    virtual bool isNumeric(){
        return false;
    }
    
    /// used to start the repr() system
    void startRepr() const{
        sprintf(buf,"%s:",getName(true));
    }
    
    /// dereference a reference value, returning the value to which
    /// it returns or NULL if it's not a reference.
    virtual Value *deref(Value *v) {
        return NULL; // not a reference
    }
    
    /// store a value into a reference value of this type
    virtual void store(Value *ref,Value *v){
        throw Exception("trying to store a value in a non-reference value");
    }
    
    /// create an iterator for a value of this type
    virtual Iterator<Value *> *createIter(Value *v,
                                          bool keys) {
        throw Exception("trying to iterate a noniterable type");
    }
    
    /// make v into a sqb ref from item with index - "this" is the type of "item"
    /// returns false if that's not permitted for this type
    virtual bool makeSQBRef(Value *v,Value *item,Value *idx){
        return false;
    }
    
    /// make v into a reference to a property contained within an item of this type,
    /// given that "item" is the item, and "prop" is the ID of the property.
    /// False if this isn't valid.
    /// The default action is to check the internal hash for the type,
    /// and set the value if that property existed.
    virtual bool makePropRef(Value *v,Value *item,u32 prop);
    
    /// set "out" to the negative of "in", where "this" is type of "in"
    virtual bool negate(Value *in,Value *out){
        return false;
    }
    /// set "out" to the logical not of "in", "this" is type of "in"
    virtual bool unarynot(Value *in,Value *out){
        return false;
    }
    
    /// arithmetic operators - "this" is type of "lhs"
    virtual void doBinArithOp(Value *out,int op,Value *lhs,Value *rhs){
        throw Exception("invalid lhs");
    }
    /// comparison operators - "this" is type of "lhs"
    virtual void doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs);
    
    
    /// serialise the value - JUST the value. The calling code will
    /// prepend the assignment if required, although in some cases (such as dictionary
    /// keys) it's called directly.
    virtual void serialise(class Serialiser *s,FILE *out,Value *v) const;
    
    /// serialise the entire value as a line consisting of name=value. Typically not
    /// overridden.
    virtual void serialiseAssignment(class Serialiser *s,const char *name,FILE *out,Value *v) const;
    
    
    ////////////// generic operations mapped to libcore functions //////////////////////////////
    
    /// is this a reference value? If not, throw. If so, is it a defined reference? True if so.
    virtual bool isDefinedReference(Value *v){
        throw Exception("not a reference value in 'defined'");
        return false;
    }
    
    /// is this a reference value? If not, throw. If so, is it a defined reference? Delete the element if so, returning
    /// true if it was there.
    virtual bool deleteElement(Value *v){
        throw Exception("not a reference value in 'del'");
        return false;
    }
    
    /// if this is a container type, return the number of items contained
    ///
    
    virtual int getSize(Value *v){
        throw Exception("not a container value in 'size'");
        return -1;
    }
    
    Type *next; //!< used for linkage so we can delete all the types
};



/// this is a namespace for the types, really
struct Types {
    static void createTypes(API *a);
    static void deleteTypes();
    
    /// the d.i value gives the integer value
    static Type *vtInteger;
    /// the d.f value gives the float value
    static Type *vtFloat;
    /// the d.u value is a constant descriptor index giving
    /// a pointer to a constant, consisting of a short offset
    /// of characters into the line followed by a null-terminated
    /// string
    static Type *vtComment;
    /// d.s is an offset into the constant data area giving
    /// a pointer to an instruction, i.e. a user function
    static Type *vtFunction;
    /// d.u is an offset into the constant data area giving
    /// a pointer to a locals descriptor table
    static Type *vtLDT;
    /// d.s is a pointer to another value, which this value
    /// references
    static Type *vtRef;
    /// d.s is a pointer to a NativeFuncData structure allocated
    /// on the heap
    static Type *vtNativeFunctionRef;
    /// d.u is an offset into the constant data area giving
    /// a pointer to a string
    static Type *vtStringConst;
    /// d.i is 1 for true, 0 for false
    static Type *vtBoolean;
    /// d.o is a pointer to an object and d2.u is a property ID
    /// for a property defined or undefined in the object. Because
    /// this refers to an object, it's VT_COMPLEX.
    static Type *vtPropRef;
    /// the value is a string, to which the d.s pointer points.
    static Type *vtString;
    /// the value is a reference to an object
    static Type *vtObject;
    
    /// the value is a reference to an dictionary
    static Type *vtDictionary;
    /// this is an IteratorObject, like an object but is an
    /// iterator for something (such as a hash)
    static Type *vtIterObj;
    /// this is a key to a dictionary entry. d.dict is the dictionary, d.d2 is
    /// an integer handle into a global growable pool containing values, for the key.
    static Type *vtDictRef;
    /// a special value for deleted items in a hash
    static Type *vtDeleted;
    /// a reference to a native method - contains a pointer to the object and a pointer
    /// to the method
    static Type *vtNativeMethodRef;
    /// a reference to a an item in a list
    static Type *vtListRef;
    /// a reference to a list object
    static Type *vtList;
};


/// used by decRef() for strings and the like; data
/// whose first few bytes is reference count but not 
/// objects.
inline void decRefSimpleMalloc(void *p){
    refct_t *ct;
    
    ct = (refct_t *)p;
    (*ct)--;
    if(*ct==0){
        dfprintf("freeing %s\n",(u32)(((char *)p)+sizeof(refct_t)));
        free(p);
    }
}        

/// handy for debugging
inline void dumprefsimplemalloc(const char *msg,void *p){
    refct_t ct;
    ct = *(refct_t *)p;
    dfprintf("%s : refct for %x = %x\n",msg,(u32)p,ct);
}

/// increment the refct which precedes simple
/// malloced data, like strings.
inline void incRefSimpleMalloc(void *p){
    refct_t *ct;
    ct = (refct_t *)p;
    (*ct)++;
    if(!*ct)
        throw Exception("ref count too large");
}


/// the union used in the Value. Each value has two of
/// these, but usually only one is used - the other is
/// for references, delegates etc.
union ValUnion {
    char *s;
    class Object *o;
    class List *list;
    class IteratorObject *iterobj;
    class Dict *dict;
    class Iterable *iterable;
    class GarbageCollected *gc;
    s32 i;
    u32 u;
    float f;
    class NativeFuncData *nd;
};

/// A Lana value, which can hold several different types of data
/// according to the Type field - these are the values Lana stores
/// on the execution stack and in variables and object properties.
/// Strings can be coerced to integers or floats, all values
/// can be coerced to strings (generally giving debug data if not numeric).
/// Any other coercion attempts will generally fail.


class Value {
    friend class Language;
public:
    
    /// pointer to the value's type object
    Type *type;
    
    /// the primary value
    union ValUnion d;
    
    /// the secondary value, used in references, delegates,
    /// closures etc.
    union ValUnion d2;
    
    
    
    /// values are created with the type None
    Value() {
        initNone();
    }
    
    /// upon deletion any string data is deleted
    ~Value() {
        clr();
    }
    
    /// quick method for telling if this is a string
    
    inline bool isStr() const{
        return type==Types::vtString || type==Types::vtStringConst;
    }
    
    /// get the allocation type
    AllocType getAllocType(){
        if(!type)
            return Unmanaged;
        return type->allocType;
    }
    
    /// increment reference count
    void incRef(){
        switch(getAllocType()){
        case SimpleMalloc:
            incRefSimpleMalloc(d.s);
            break;
        case SimpleNew:
        case Complex:
            d.gc->incRefCt();
            break;
        case DictRefAlloc:
            incDictKeyRef();
            break;
        default:break;
        }
    }
    
    /// returns -1 if not refcounted, otherwise the refcount
    int getRefCt(){
        switch(getAllocType()){
        case SimpleMalloc:{
            refct_t*ct = (refct_t *)d.s;
            return *ct;
        }
        case SimpleNew:
        case Complex:
            return d.gc->refct;
        case DictRefAlloc:
            // might not be accurate, since the key will also have a refct
            return d.gc->refct;
        default:return -1;
        }
    }
    
    /// decrement a reference count and delete if it becomes zero. Or do nothing,
    /// depending on the type.
    
    inline void decRef(){
        switch(getAllocType()){
        case SimpleMalloc:
            decRefSimpleMalloc(d.s);
            break;
        case SimpleNew:
        case Complex:
            if(d.gc->decRefCt()){
                dfprintf("deleting %s\n",repr());
                delete d.gc;
            }
            break;
        case DictRefAlloc:
            decDictKeyRef();
        default:break;
        }
    }
    
    /// separate out-of-line method to avoid making inlining all
    /// if incRef()
    void incDictKeyRef();
    /// separate out-of-line method to avoid making inlining all
    /// if decRef()
    void decDictKeyRef();
    
    
    /// comparison operator, used in hashes
    bool equalsForHashTable(Value *v){
        if(isStr() && v->isStr()){
            return strcmp(getStr(),v->getStr())?false:true;
        }
        
        if(type!=v->type)
            return false;
        
        // we should only ever be comparing values which don't use
        // d2!
        
        return d.u == v->d.u;
    }
    
    
    /// used by copy ctor/operator
    void copy(const Value& source){
        clr();
        d.u = source.d.u; 
        d2.u = source.d2.u;
        type = source.type;
        incRef();
    }
    
    /// a copy constructor. Will increment reference counts.
    /// \todo{don't copy d2 if it's not used - perhaps have a d2-used bit?}
    Value(const Value& source){
        d.u = source.d.u; 
        d2.u = source.d2.u;
        type = source.type;
        incRef();
    }
    
    /// copy assignment operator. Will increment ref counts.
    Value& operator= (const Value& source) {
        if(this != &source){
            copy(source);
        }
    }
    
    /// get the type of the value
    Type *getType() const { return type; }
    
    /// assign a string - DO NOT USE for constants stored in the const area, as these will move! This will MAKE A COPY. If you don't
    /// want to do that, tough - because you need to store the reference
    /// count anyway as the first two bytes. However, you can always
    /// use setOther(). Note that this method will add the new refct for you.
    void setStrClone(const char *s);
    /// assign a string constant - use this for constants in the const area; the value is the const id
    void setStrConst(constid offset);
    /// set the value to a property reference
    void setPropRef(Object *o,unsigned int id) {
        ((GarbageCollected *)o)->incRefCt();
        clr();
        d.o = o;
        d2.u = id;
        type = Types::vtPropRef;
    }
    /// set to a reference to a native method, containing the object pointer
    void setNativeMethodRef(Object *o,class NativeFuncData *nd){
        ((GarbageCollected *)o)->incRefCt();
        clr();
        d.o = o;
        d2.nd = nd;
        type = Types::vtNativeMethodRef;
    }
    
    /// turn this Value into a DictRef to a value in
    /// the given dict, keyed by the given Value key.
    void setDictRef(Dict *dt,Value *key);
    
    /// turn this value into a ListRef to a value in a list
    void setListRef(class List *list,Value *idx);
    
    /// assign a function, using the constant id
    void setFunc(int i){
        clr();
        type = Types::vtFunction;
        d.u = i;
    }
    
    /// set the value to an integer
    void setInt(int i){
        clr();
        type = Types::vtInteger;
        d.i = i;
    }
    
    /// set the value to a float
    void setFloat(float f){
        clr();
        type = Types::vtFloat;
        d.f = f;
    }
    
    /// set the value to a boolean
    void setBool(bool b){
        clr();
        type = Types::vtBoolean;
        d.i = b?1:0;
    }
    
    /// set the value to an object. Semantics designed to 
    /// ensure a refct never goes to zero erroneously. Also,
    /// sets the type to an appropriate type object if
    /// it's some kind of native, such as dict or list.
    void setObj(class Object *o);
    
    /// set the value to an iterator object
    void setIterObj(class IteratorObject *o){
        ((GarbageCollected *)o)->incRefCt();
        clr();
        type = Types::vtIterObj;
        d.iterobj = o;
    }
    
    /// set the value to a native method
    void setNativeFunctionRef(class NativeFuncData *nd){
        clr();
        type = Types::vtNativeFunctionRef;
        d.nd = nd;
    }
        
    
    /// set to an uninitialized value. DOES NOT CLEAR.
    void initNone() { type=NULL; d.i=0xdeadbeef;}
    
    /// set to some other type of value. You'll probably have to incRef()
    /// after this; I don't do it in here because some optimisations can
    /// be made that way.
    void setOther(Type *t,void *p){
        clr();
        type = t;
        d.s = (char *)p;
    }
    /// set to some other type of value, using the d2 field.
    void setOther(Type *t,void *p,void *p2){
        clr();
        type = t;
        d.s = (char *)p;
        d2.s = (char *)p2;
    }
    
    /// dereference a Value repeatedly so it's a plain value.
    Value *deref();
    
    /// debugging - dereference like deref() but without errors and
    /// return a string like repr()
    //    const char *debugDerefRepr();
    
    /// Set to None,
    /// Will do other things depending on the subtype:
    /// - Primitive : nothing
    /// - SimpleMalloc : decrement the refct at the start of the block,
    ///   if it becomes zero, free the block
    /// - SimpleNew : decrement the refct in the object, if it becomes
    ///   zero, delete the object
    void clr(){
        decRef();
        type = NULL;
    }
    
    // retrievals, coercing!
    
    /// retrieve a string. Unusual semantics - in the case of coerced
    /// numerics, result will be in a static string buffer. In the case
    /// of strings, result will be the actual buffer.
    
    char *getStr() const {
        return type->getStr(this);
    }
    
    /// just get the pointer, dereferencing StringConst and Function in the constant area
    char *getPtr() const {
        return type->getPtr(this);
    }
    
    /// get the integer value, converting if possible
    int getInt() const {
        return type->getInt(this);
    }
    
    /// queries the value's type to determine whether it is
    /// numeric
    bool isNumeric() {
        if(!type)
            return false;
        else
            return type->isNumeric();
    }
    
    /// get the float value, converting if possible
    float getFloat() const {
        return type->getFloat(this);
    }
    
    /// get the bool value - THIS IS NOT CONVERTED, thus telling the user of things like "=" and "==" confusion.
    bool getBool() const {
        return type->getBool(this);
    }
    
    /// get the object value - THIS IS NOT CONVERTED because that would be silly
    class Object *getObj() {
        return type->getObj(this);
    }
    
    /// get a hash value - objects use the pointer,
    /// integers the value itself, strings a hash function,
    /// etc. Note that coercion doesn't work for hashes!
    u32 getHash() const {
        return type->getHash(this);
    }
    
    /// given that this value is a reference of some sort,
    /// store another value in it.
    void store(Value *v){
        type->store(this,v);
    }
    
    
    
    
    /// get a static representation in a handy debugging form.
    /// getStr() is probably more use to you.
    const char *repr() const {
        if(NULL)
            return "none";
        return type->repr(this);
    }
    
    /// this stores where we get constants from; Lana can change it.
    static class Constants *consts;
    
    /// set the value of consts
    static void setConsts(class Constants *c){ 
        consts = c;
    }
    
};

}

#include "vtypes.h"

#endif /* __LANAVALUES_H */

