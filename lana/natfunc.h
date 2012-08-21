/**
 * @file
 * Native function registry
 */


#ifndef __NATFUNC_H
#define __NATFUNC_H

#include "value.h"

namespace lana {

/// data for a native function.

struct NativeFuncData {
    
    char name[32]; //!< internal name
    
    u8 argc; //!< number of argument
    bool returns;//!< does the value return a function?
    class Host *h; //!< if ismethod is true and this is set, it's a "hosted function"
    
    bool ismethod; //!< if true, it's a method, else a plain func
    
    /// this union uses m if it's a ismethod is true, otherwise f.
    union{
        HOSTMETHOD m;
        VOIDFUNC f;
    } d;
    
    NativeFuncData *next; //!< linked list link
    
    NativeFuncData(const char *nm,int a,bool r){
        if(strlen(nm)>32)
            throw Exception("name too long");
        strcpy(name,nm);
        argc=a;
        returns=r;
    }
};

/// a registry for native functions and methods, linked
/// together in a linked list and searchable by name (used
/// in deserialisation)

class NativeRegistry {
private:
    NativeFuncData *head;
public:
    NativeRegistry(){
        head = NULL;
    }
    
    ~NativeRegistry(){
        NativeFuncData *d,*n;
        for(d=head;d;d=n){
            n=d->next;
            delete d;
        }
    }
    
    NativeFuncData *addMethod(const char *nm,int argc,bool returns,
                   HOSTMETHOD m,class Host *h) {        
        NativeFuncData *d = new NativeFuncData(nm,argc,returns);
        d->ismethod=true;
        d->h = h;
        d->d.m = m;
        d->next = head;
        head = d;
        return d;
    }
    
    NativeFuncData *addFunction(const char *nm,int argc,bool returns,
                     VOIDFUNC f,class API *a) {
        NativeFuncData *d = new NativeFuncData(nm,argc,returns);
        d->ismethod=false;
        d->h = (class Host *)a;
        d->d.f = f;
        d->next = head;
        head = d;
        return d;
    }

    NativeFuncData *getByName(const char *s){
      for(NativeFuncData *d = head;d;d=d->next){
	if(!strcmp(s,d->name))
	  return d;
      }
      return NULL;
    }
};

}

#endif /* __NATFUNC_H */
