/**
 * @file
 * primitive type definitions, such as u32.
 *
 * These probably aren't used everywhere they should be.
 */

#ifndef __BASETYPES_H
#define __BASETYPES_H

namespace lana {

typedef unsigned long u64;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned char u8;
typedef signed char s8;

/// the type of an instruction, a basic Lana bytecode element,
/// consisting of 8 bits of opcode and 24 bits of data.
/// It's here because various unexpected things need it.
typedef u32 instruction;

/// a constant ID - actually a index into the constant
/// memory area, divided by 4.
typedef u32 constid;


/// an interface object, in which native functions run as
/// methods.This contains a link to the api. 
/// If you want a full object with the ability to run native
/// methods and store properties, use Object.

class Host {
public:
    virtual ~Host(){}
    Host(class API *a){
        api = a;
    }
    class API *api;
};

/// a method in a subclass of Host
typedef void (Host::*HOSTMETHOD)();
/// just a void function taking an API
typedef void (*VOIDFUNC)(class API *a);

}

#endif /* __BASETYPES_H */
