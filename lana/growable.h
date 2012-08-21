/**
 * @file
 * contains the Growable class, implementing a block of memory
 * which can grow indefinitely.
 */

#ifndef __GROWABLE_H
#define __GROWABLE_H


#include <memory.h>
#include <stdio.h>

#include "basetypes.h"
#include "exception.h"

namespace lana {

/// This is a simple growable block of memory, using realloc().
/// Memory can be allocated, but never freed. All the allocations
/// are contiguous, although padding is added if the offset
/// into memory for a new allocation would have incorrect alignment.
/// If the current block isn't big enough, a realloc()
/// is done to allocate a new, larger block and the existing data
/// is copied over. The values returned from the allocation methods
/// are offsets into the memory block. The pointer to a block can
/// then be retrieved with get().
///
/// <b>Caution:</b> do NOT store pointers to things in a Growable -
/// if the Growable is resized they will no longer be valid!

class Growable
{
protected:
    
    /// current size of the block in bytes
    u32 mCurSize;
    
    /// size the block grows, in bytes
    u32 mGrowSize;
    
    /// base pointer of the block
    char *mBase;
    
    /// current index of the block
    u32 mPtr;
    
    /// alignment of allocations minus 1
    u32 mAlignment;
    
    /// a snapshot pointer
    u32 mSnapshot;
    
public:
    
    
    /// the constructor
    Growable(
             u32 basesize,	//!< starting size in bytes
             u32 growsize,	//!< size to grow when runs out
             u32 align=4	//!< alignment of each item (including the descriptor,) must be power of 2 
             )  {
        mCurSize = basesize;
        mGrowSize = growsize;
        mBase = new char [mCurSize];
        memset(mBase,0,mCurSize);
        mPtr = 0;
        mBase[0]=0;
        mAlignment = align-1;
    }
    
    /// delete the growable and its memory
    ~Growable() {
        delete [] mBase;
    }
    
    /// allocate memory from the block and return an offset,
    /// growing if required.
    /// Throws an exception if out of space.
    
    u32 allocate(u32 size) {
        size = (size+mAlignment)&~mAlignment;
        if(mPtr+size >= mCurSize)
        {
            // out of space, grow!
            
            int grow = (mPtr+size)-mCurSize;
            grow /= mGrowSize;
            grow = (grow+1)*mGrowSize;
            
            char *oldbase = mBase;
            mBase = new char [mCurSize+grow];
            memset(mBase+mCurSize,0,grow);
            if(!mBase)
                throw GrowableException("out of memory");
                
            memcpy(mBase,oldbase,mCurSize);
            memset(oldbase,0xff,mCurSize);
            mCurSize += grow;
            delete [] oldbase;
            
        }
        u32 ret = mPtr;
        if(mPtr>mCurSize)
            throw GrowableException("oops");
        mPtr+=size;
        return ret;
    }
    
    /// does allocate and get, for when we don't need the offset
    u8 *allocget(u32 size) {
        return (unsigned char *)get(allocate(size),size);
    }
    
    /// allocates enough room for the string but NOT the trailing
    /// null, and copies it into the allocated memory without said
    /// null. This is used to build up strings of arbitrary length
    /// in the growable. The string you're building must be terminated
    /// with terminate(). Once that's done you can call get(0,0) to
    /// get a pointer to the start of the string.
    char *write(const char *s){
        char *p = (char*)allocget(strlen(s));
        strcpy(p,s);
        return p;
    }
    /// does null termination of a string built up with write()
    void terminate(){
        char *p = (char*)allocget(1);
        *p=0;
    }
        
    
    /// allocate an object from the block using allocate(). Does
    /// not run a constructor.
    template <class T> u32 allocate()
    {
        u32 d=allocate(sizeof(T));
        return d;
    }
    
    /// is a given offset for a given size all inside the memory?
    bool isinside(u32 i,u32 size){
        return(i+size<=mPtr);
    }
    
    /// return a pointer into the memory, given
    /// an offset and the size (for checking). If you
    /// set size to 0 you won't get the check except for
    /// the first byte.
    
    void *get(u32 i,u32 size){
        if(i+size>mPtr)
            throw GrowableException("bad offset requested in growable");
        
        return (void *)(mBase+i);
    }
    
    /// templateised version, added later - this isn't used everywhere it should be!
    template <class T> T *get(u32 i){
        return (T *)get(i,sizeof(T));
    }
    
    /// clear the memory back to the start - memory remains at its
    /// grown size
    
    void clear() {
        mPtr = 0;
    }
    
    /// return next offset to be written to, or the offset of an item in the memory
    int getOffset(void *p=NULL) {
        if(!p)
            return mPtr;
        else
            return (u32)(((char *)p) - mBase);
    }
    
    /// create a marker
    void saveSnapshot(){
        mSnapshot = mPtr;
    }
    
    /// delete memory allocated after the marker. Ugly.
    void restoreSnapshot() {
        char *oldbase = mBase;
        // shrink
        mCurSize = mSnapshot;
        // allocate new area
        mBase = new char[mCurSize];
        // copy old data into new area
        memcpy(mBase,oldbase,mCurSize);
        mPtr = mSnapshot;
        // delete old area
        delete [] oldbase;
    }
};
}
    

#endif /* __GROWABLE_H */
