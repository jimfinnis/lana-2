/**
 * @file
 * Constant manager and data structures.
 * 
 * This file contains the constant manager and declarations for
 * the data types used by constants. Constants are kept
 * in a Growable, and are each preceded by an 4 byte Constant Descriptor
 * giving the flags, type and size of the data. All constant allocations
 * are therefore aligned to 4 bytes.
 * The actual constant data is limited to 65536 bytes.
 * 
 * Within the code, constants are always referred to by their
 * constant ID, or constid. This is the offset of the start of the
 * descriptor in the Growable's memory, divided by four. This gives us
 * the ability to index up to 64Mbytes of constant data with a 24-bit
 * word (the size of the data field of an instruction.)
 */


#ifndef __CONSTS_H
#define __CONSTS_H

#include "growable.h"
#include "value.h"
#include "iterator.h"
#include "specprops.h"
#include <new> // so we get placement-new

namespace lana {

/// the header for the locals descriptor table for functions.
/// These values follow the header in the constant area    
/// \code
///    u32 paramnames[numparams];
///    u32 localnames[numlocals];
/// \endcode
struct LDTHeader {
    void init(){
        numparams = 0;
        numlocals = 0;
        flags = 0;
    }
    
    u32 flags;	//!< flag values
    u16 numparams; //!< how many parameters
    u16 numlocals; //!< how many locals
    
    // data follows..
};    
#define LDTF_RETURNS 1


enum ConstType {
    CT_STRING=0,CT_INT,CT_FLOAT,CT_LDT,CT_COMMENT,CT_FUNC
};
          

/// a constant descriptor, which precedes constant data.
struct ConstDesc {
    /// size of the data in bytes
    u16 size;
    /// bottom bits are type, the rest are flags. The number varies :)
    u16 flagsandtype;
    
    /// set this descriptor's values
    inline void set(ConstType t,int f,int sz){
        size = sz;
        flagsandtype = (f<<8)+(int)t;
    }
    
    /// get the flags data
    inline int getFlags(){
        return flagsandtype>>8;
    }
    /// get the type
    inline ConstType getType(){
        return (ConstType)(flagsandtype & 0xff);
    }
    
    /// get a pointer to the constant data
    void *get() {
        return (void *)(this+1);
    }
    
};

/// this class manages a Growable memory area containing constant data

class Constants {
public:
    Constants();
    ~Constants();
    
    /// this value is returned by various methods if a const is not found
    static const constid NOTFOUND = 0xffffffff;
    
    /// return an iterator
    Iterator<ConstDesc *>* createIterator();
    
    /// create a constant, returning the ID (longword
    /// offset into the growable memory)
    /// Will copy p into the area if p is not NULL.
    
    constid create(ConstType type,const void *p,u16 flags,u32 size);
    
    /// gets the descriptor data for a given ID, or NULL if it's out of range
    
    ConstDesc *get(constid desc){
        return (ConstDesc *)constantArea->get(desc<<2,0);
    }
    
    /// return an constdesc's ID
    constid getIdx(ConstDesc *c){
        u32 offset = constantArea->getOffset(c);
        return offset>>2;
    }
    
    /// return whether or not a constid is valid
    bool isValid(constid id){
        return constantArea->isinside(id<<2,4);
    }
    
    /// find a string constant by string, returning the ID or NOTFOUND
    
    constid findString(const char *s);
    
    /// find a string constant by string, returning the ID. If
    /// not found, create a new constant and return the new ID.
    
    constid findOrCreateString(const char *s);
    
    /// create a comment - this is a string preceded by a short giving the offset
    /// of the comment's first character in the line
    
    constid createComment(const char *s,int pos);
    
    /// find an integer constant, returning the ID or NOTFOUND
    constid findInt(int n);
    constid findOrCreateInt(int n);
    
    /// find a float constant, returning the ID or NOTFOUND
    constid findFloat(float n);
    constid findOrCreateFloat(float n);
    
    /// is this constant valid as a property ID. Has to be a constant string.
    bool isValidPropID(constid id){
        if(!isValid(id))
            return false;
        if(get(id)->getType() == CT_STRING)
            return true;
    }
    
    /// helper - get the string for a given ID
    char *getStr(constid desc){
        ConstDesc *e = get(desc);
        if(e->getType() != CT_STRING)
            throw Exception("internal error : constant is not a string");
        return (char *)(e->get());
    }
    
    /// these are the IDs for the names of special properties such as "size"
    /// and "del".
    
    PropIDs props;
    
private:
    Growable *constantArea;
    
};

}

#endif /* __CONSTS_H */
