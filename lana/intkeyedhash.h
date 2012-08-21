#ifndef __INTKEYEDHASH_H
#define __INTKEYEDHASH_H

/**
 * @file
 * Hash : An implementation of a hash table keyed on unsigned integer,
 * based on the Python dictionary. See Hash for a version with a general
 * key. The code is largely identical, but this version is faster. I
 * don't like the idea of code duplication on this scale, but it's
 * not really possible any other way.
 */

#include <stdlib.h>
#include <string.h>
#include "iterator.h"

namespace lana {

// 5 in original
#define PERTURB_SHIFT 5
// 32 in original
#define INITIAL_SIZE 32
// 4 in original
#define RESIZE_FACTOR	4

// 2/3 in original
#define RESIZE_THRESHOLD_NUMERATOR 2
#define RESIZE_THRESHOLD_DENOMINATOR 3

//#define DEBUG 1

#define HSH_FREE    0 
#define HSH_USED    1
#define HSH_DELETED 2

/// an individual slot in the property hash table.

template <class T>struct IntKeyedHashEnt {
    u32 k; 	//!< the key 
    int s;		//!< state: free, used or deleted?
    T v;		//!< the value
    IntKeyedHashEnt(){k=0;s=HSH_FREE;}
    ~IntKeyedHashEnt(){
        s = HSH_DELETED;
        v.~T();
    }
        
};


/// templated destroy so we can run the dtor
template <class T> inline void destroy(T& x) { x.~T();}

/// This is an implementation of a hash table in which the keys are 
/// unsigned integers. It's based on the Python dictionary, from notes
/// in Beautiful Code. There's also a good description of the Python
/// implementation at http://www.laurentluce.com/?p=249
/// Also the original source code is in dictobject.c
///
/// This is a much simpler version than that in Hash because the
/// keys are simple unsigned integers.
///
///

template <class T> class IntKeyedHash {
public:
    
    IntKeyedHash(){
#ifdef DEBUG
        miss=0;
#endif
        mask = INITIAL_SIZE-1;
        table = new IntKeyedHashEnt<T>[mask+1];
        used=0;
        fill=0;
        recalcresizethreshold();
    }
    
    ~IntKeyedHash(){
        delete [] table;
#ifdef DEBUG
//        fprintf(stderr,"misses : %d, size %d\n",miss,mask+1);
#endif
    }
    
    /// empty the hash of all values
    void clear(){
        IntKeyedHashEnt<T> *ent = table;
        for(int i=0;i<=mask;i++){
            if(ent->s == HSH_USED){
                ent->v.~T();
                ent->s = HSH_FREE;
            }
        }
        used=0;
        fill=0;
    }
    
    /// set a value in the table - returns a value pointer for
    /// you to write to
    
    virtual  T* set(u32 k){
        //printf("set %d to %d, used %d, size %d\n",k,v,used,mask+1);
        
        if(used > resizethreshold){
            //printf("RESIZE!!!!!\n");
            resize();
        }
        
        IntKeyedHashEnt<T> *ent = look(k);
        if(ent->s != HSH_USED){
            // there wasn't a value there before
            if(ent->s == HSH_FREE)
                fill++; //we aren't overwriting a dummy, so increment fill
        
            ent->k = k;
            ent->s = HSH_USED;
            used++; // increment used
        }
        return &ent->v;
    }
    
    /// finds a value in the hash table, returning true and setting
    /// the internal found value pointer if found. If found, the value
    /// can then be retrieved with getval().
    
    virtual bool find(u32 k){
        IntKeyedHashEnt<T> *ent = look(k);
        if(ent->s == HSH_USED) {
            v = &ent->v;
            return true;
        }
        else
            return false;
    }
    
    /// get the last value found by find()
    T *getval(){
        return v;
    }
    
    /// delete an item with a given key, returning true if we did it
    bool del(u32 k) {
        IntKeyedHashEnt<T> *ent = look(k);
        if(ent->s != HSH_USED)
            return false;
        ent->~IntKeyedHashEnt<T>(); // delete old value!
        used--;
    }
    
    
    // create the iterator
    class Iterator<T *> *createValueIterator();
    class Iterator<u32> *createKeyIterator();

#ifdef DEBUG
    int miss;
#endif
    // I wish this lot could be private, but the template below needs to
    // see them. And I can't put a friend declaration for the template in,
    // because then that would have to be above this class. And *that* wouldn't work,
    // because it needs things in *this* class. The joy of templates.
    IntKeyedHashEnt<T> *table;
    T *v;
    
    unsigned int used; //!< number of slots occupied by keys
    unsigned int fill; //!< number of slots occupied by keys or dummies (used only if we implement deletion)
    unsigned int mask; //!< hashtable contains mask+1 slots
    unsigned int resizethreshold;
    
    void recalcresizethreshold(){
        resizethreshold = (mask+1)*RESIZE_THRESHOLD_NUMERATOR;
        resizethreshold /= RESIZE_THRESHOLD_DENOMINATOR;
    }
    
    void resize(){
        unsigned int oldsize = mask+1;
        IntKeyedHashEnt<T> *oldtable = table;
        unsigned int minused = used*RESIZE_FACTOR;
              
        int newsize;
        for(newsize = oldsize; newsize<=minused && newsize>0;newsize<<=1){}
        //printf("resizing to %d\n",newsize);
        
        table = new IntKeyedHashEnt<T>[newsize];
        mask = newsize-1;
        used = 0;
        fill = 0;
        // iterate values, reinserting into new table
        IntKeyedHashEnt<T> *ent=oldtable;
        T *p;
        for(unsigned int i=0;i<oldsize;i++,ent++){
            if(ent->s == HSH_USED){
                //printf("RESIZE SET\n");
                p = set(ent->k);
                *p = ent->v;
            }
        }
        delete [] oldtable;
        recalcresizethreshold();
        //printf("RESIZE END!\n");
    }
    
    /// scan, looking for either a slot with this key or the
    /// slot where this key would go
    
    IntKeyedHashEnt<T> *look(u32 k){
        register unsigned int slot = k & mask;
        register IntKeyedHashEnt<T> *ent = table+slot;
        register IntKeyedHashEnt<T> *freeslot;
        if(ent->s==HSH_FREE || ent->k == k)
            return ent;
        if(ent->s == HSH_DELETED)
            freeslot = ent;
        else
            freeslot = NULL;
        
        for(unsigned int perturb = k;;perturb>>=PERTURB_SHIFT){
#ifdef DEBUG
            miss++;
#endif
            slot = (slot<<2)+slot+1+perturb;
            ent = table+(slot&mask);
            if(ent->s == HSH_FREE)
                return freeslot==NULL ? ent : freeslot;
            if(ent->k == k && ent->s!=HSH_DELETED)
                return ent;
            else if(ent->s==HSH_DELETED && freeslot==NULL)
                freeslot = ent;
        }
    }
};

/// IntKeyedHash value iterator - you probably won't access this
/// directly.

template <class T> class IntKeyedHashValueIterator : public Iterator<T *> {
public:
    IntKeyedHashValueIterator(IntKeyedHash<T> *h){
        hash = h;
        ent=NULL;
    }
    
    virtual void first(){
        size=hash->mask+1;
        ent=hash->table;
        idx=0;
        while(idx<size && ent->s != HSH_USED) {idx++;ent++;}
    }
    virtual void next(){
        ent++;idx++;
        while(idx<size && ent->s != HSH_USED) {idx++;ent++;}
    }
    virtual bool isDone() const {
        return idx>=size;
    }
    virtual T *current() {
        if(!ent)
            throw Exception("first() not called on iterator");
        if(idx>=size)
            throw Exception("iterator out of range");
        return &ent->v;
    }
    
private:
    IntKeyedHash<T> *hash;
    int idx,size;
    IntKeyedHashEnt<T> *ent;
};

template<class T> Iterator<T *> *IntKeyedHash<T>::createValueIterator() {
    return new IntKeyedHashValueIterator<T>(this);
};


/// IntKeyedHash key iterator, used by PropertyKeyIterator

template <class T> class IntKeyedHashKeyIterator : public Iterator<u32> {
public:
    IntKeyedHashKeyIterator(IntKeyedHash<T> *h){
        hash = h;
    }
    
    virtual void first(){
        size=hash->mask+1;
        ent=hash->table;
        idx=0;
        while(idx<size && ent->s != HSH_USED) {idx++;ent++;}
    }
    virtual void next(){
        ent++;idx++;
        while(idx<size && ent->s != HSH_USED) {idx++;ent++;}
    }
    virtual bool isDone() const {
        return idx>=size;
    }
    virtual u32 current() {
        if(!ent)
            throw Exception("first() not called on iterator");
        return ent->k;
    }
    
private:
    IntKeyedHash<T> *hash;
    int idx,size;
    IntKeyedHashEnt<T> *ent;
};

template<class T> Iterator<u32> *IntKeyedHash<T>::createKeyIterator() {
    return new IntKeyedHashKeyIterator<T>(this);
};



}    
    
#endif /* __INTKEYEDHASH_H */
