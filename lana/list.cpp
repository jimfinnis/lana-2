#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "api.h"
#include "language.h"
#include "arraylist.h"
#include "list.h"
#include "cycle.h"

using namespace lana;

#define MT(xx) (lana::HOSTMETHOD)&lana::List::xx

ListType::ListType(API *a) : ObjectType() {
    proto = new List(a);
    
    a->setNamePrefix("list$");
    
    proto->registerNativeMethod("append",1,false,MT(methodAppend));
    proto->registerNativeMethod("push",1,false,MT(methodAppend));
    proto->registerNativeMethod("pop",0,true,MT(methodPop));
    proto->registerNativeMethod("peek",0,true,MT(methodPeek));
    proto->registerNativeMethod("shift",1,false,MT(methodShift));
    proto->registerNativeMethod("unshift",0,true,MT(methodUnshift));
    
    
    proto->registerNativeMethod("insert",2,false,MT(methodInsert));
    proto->registerNativeMethod("remove",1,false,MT(methodRemove));
    
    proto->incRefCt();
}

List *List::create(API *a) {
    List *d = new List(a);
    /// make it a clone of the prototype, without using clone()
    /// because there's no data to clone out.
    d->makeCloneOf(((ListType *)Types::vtList)->proto);
    d->type = Types::vtList;
    return d;
}


List::List(class API *a) : Object (a) {
    list = new ArrayList(16); // initial size
}

List::~List(){
    delete list;
}

void List::methodAppend(){
    Value *dest = list->append();
    Value *src = api->popRaw();
    
    *dest = *src; // SHOULD result in an incref
}

void List::methodInsert(){
    Value *v = api->popRaw();
    int i = api->popInt();
    
    *list->insert(i) = *v; // SHOULD result in an incref
}

void List::methodPop(){
    Value *v = list->pop();
    *api->pushRaw() = *v;
    
}

void List::methodRemove(){
    int i = api->popInt();
    list->remove(i);
}

void List::methodPeek(){
    Value *v = list->peek();
    *api->pushRaw() = *v;
}

void List::methodShift(){
    Value *dest = list->insert(0);
    Value *src = api->popRaw();
    
    *dest = *src; // SHOULD result in an incref
}

void List::methodUnshift(){
    if(!list->count())
        throw ArrayListException("unshift on empty list");
    
    // note the order here - remove() will clear the item, so
    // we need to copy it into the destination first.
    
    Value *v = list->get(0);
    *api->pushRaw() = *v;
    list->remove(0);
}


// List type methods

int ListType::getSize(Value *v){
    return v->d.list->list->count();
}

const char *ListRefType::repr(const Value *v) const {
    startRepr();
    sprintf(buf+strlen(buf),"%x/(ct%d)[%d]",(u32)v->d.list,
            v->d.gc->refct,v->d2.i);
    return buf;
}

void ListRefType::store(Value *ref,Value *v){
    ref->d.list->set(ref->d2.i,v);
}
Value *ListRefType::deref(Value *v){
    v = v->d.list->get(v->d2.i);
    if(!v)
        throw Exception("unset value in list");
    return v;
}

bool ListRefType::isDefinedReference(Value *v){
    return v->d.list->get(v->d2.i) ? true : false;
}

bool ListRefType::deleteElement(Value *v){
    return v->d.list->remove(v->d2.i);
    
}
