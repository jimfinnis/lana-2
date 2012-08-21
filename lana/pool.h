#ifndef __POOL_H
#define __POOL_H

/** 
 * @file
 * Pool, a dynamic pool implementation.
 */

#include "exception.h"
#include "iterator.h"

namespace lana {

template <class T,int I> class PoolIterator;

class PoolException : public Exception{
public:
    PoolException(const char *s) : Exception(s){}
};


/// The pool templates allow for the creation of allocatable pools of statically sized objects.
/** a pool is a collection of blocks of memory of fixed size, which can
 * be allocated and freed as required. The size of the block is the size of the template type of the pool. Objects which 
 * are allocated from pools can automatically have init() run on them when allocated, 
 * and shutdown() run on them when freed.
 * 
 * If a pool becomes full, it resizes automatically (to 1.5 times the former size)
 */

template <class TYPE,int initMaxItems> class Pool
{
    friend class PoolIterator<TYPE,initMaxItems>;
private:
    /// the elements within the pool
    struct Element
    {
        char data[ sizeof( TYPE ) ];  //!< memory for the data
        int nextFree; //!< either -1 for end of list, -2 for allocated,  or the index of the next free item
    };
    Element *data;
    int firstFree;	//!< first free item index, or -1
    int entries;	//!< number of allocated slots
    int maxItems;	//!< number of slots
    
public:
    
    void assertEmpty()
    {
        if(entries)
            throw PoolException("pool is not empty");
    }
    
    bool hasSpace()
    {
        return firstFree != -1;
    }
    
    void growIfRequired()
    {
        if(firstFree<0){
            // work out the new size
            int newSize = maxItems * 3;
            newSize >>=1;
            
//            printf("GROW %d -> %d!\n",maxItems,newSize);
            
            Element *newdata = new Element [newSize];
            for(int i=0;i<newSize;i++){
                newdata[i].nextFree = i+1;
            }
            newdata[newSize-1].nextFree=-1;
            
            // switch data around
            firstFree = 0;
            entries = 0;
            Element *oldData = data;
            data = newdata;
            
            for(int i=0;i<maxItems;i++){
                if(oldData[i].nextFree == -2){
                    // allocated, so add
                    int id = alloc();
                    TYPE *p = get(id);
                    // copy old data into new
                    TYPE *src = (TYPE *)&oldData[i].data;
                    *p = *src;
                    // delete old
                    src->~TYPE();
                }
            }
            maxItems = newSize;
            // and clear.
            delete [] oldData;
        }
    }
    
    int freeSlots()
    {
        return maxItems-entries;
    }
    
    /// delete all items
    void empty()
    {
        for( int elementLoop = 0; elementLoop < maxItems; ++elementLoop )
        {
            if(data[elementLoop].nextFree == -2){
                TYPE *d = (TYPE *)&data[elementLoop].data;
                d->~TYPE();
            }
            data[ elementLoop ].nextFree = elementLoop + 1;
        }
        data[ maxItems - 1 ].nextFree = -1;
        firstFree = 0;
        entries = 0;
    }
    
    /// get an index to the item
    TYPE *get(int idx){
        return(TYPE *)(data+idx);
    }
                       
    /// allocate an item of the pool's type, and invoke its constructor
    int alloc()
    {
        growIfRequired();
        int ID = firstFree;
        firstFree = data[ID].nextFree;
        data[ID].nextFree = -2;
        TYPE *item = (TYPE*)&data[ ID ].data;
        new ( item ) TYPE();
        entries++;
        return ID;
    }
    
    /// allocate an item of the pool's type, and invoke its constructor (version for types with ctors with 1 arg)
    template <class ARG1>
              int alloc( ARG1 arg1 )
    {
        growIfRequired();
        int ID = firstFree;
        firstFree = data[ID].nextFree;
        TYPE *item = (TYPE*)&data[ ID ].data;
        new ( item ) TYPE( arg1 );
        data[ID].nextFree = -2;
        entries++;
        return ID;
    }
    
    /// allocate an item of the pool's type, and invoke its constructor (version for types with ctors with 2 arg)
    template <class ARG1, class ARG2>
              int alloc( ARG1 arg1, ARG2 arg2 )
    {
        growIfRequired();
        int ID = firstFree;
        firstFree = data[ID].nextFree;
        data[ID].nextFree = -2;
        TYPE *item = (TYPE*)&data[ ID ].data;
        new ( item ) TYPE( arg1, arg2 );
        entries++;
        return ID;
    }
    
    /// remove an item from the pool, freeing up its slot.
    /// may produce a free twice exception, but not if the slot
    /// has been reallocated since the last free!
    void free( int ID )
    {
        if(ID<0 || ID >= maxItems){
            throw PoolException("attempt to free item not in pool");
        }
        Element *element = data+ID;
        if(element->nextFree>=0){
            throw PoolException("attempt to free item twice");
        }
        TYPE *item = (TYPE *)element;
        item->~TYPE();
        element->nextFree = firstFree;
        entries--;
        firstFree = ID;
    }
    
public: // constructors and destructors
    Pool()
    {
        maxItems = initMaxItems;
        data = new Element[initMaxItems];
        for(int i=0;i<maxItems;i++){
            data[i].nextFree = i+1;
        }
        empty();
    }
    virtual ~Pool() { empty(); delete [] data; }
    
    /// return a slot iterator
    Iterator<int> *createIterator();
};

/// this is the class which implements pool iteration, don't use it
/// directly.

template <class T,int I> class PoolIterator : public Iterator<int> {
    Pool<T,I> *p;
    int i;
public:
    PoolIterator(Pool<T,I> *_p){
        p=_p;
        i=-1;
    }
    virtual void first(){
        for(i=0;i<p->maxItems;i++){
            if(p->data[i].nextFree == -2)
                return;
        }
        i=-1;
    }
    virtual void next(){
        if(i==-1)
            return; // gone past end
        for(++i;i<p->maxItems;i++){
            if(p->data[i].nextFree == -2)
                return;
        }
        i=-1;
    }
    virtual bool isDone() const {
        return i==-1;
    }
    
    virtual int current() {
        return i;
    }
};


template <class T,int I> Iterator<int> *Pool<T,I>::createIterator() {
    return new PoolIterator<T,I>(this);
}


}
#endif /* __POOL_H */
