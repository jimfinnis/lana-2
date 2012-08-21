#ifndef __VM_H
#define __VM_H

/**
 * @file
 * VirtualMachine and associated classes, which handle running
 * the code.
 */

#include "object.h"
#include "dict.h"

namespace lana {

/// data on the return stack

struct ReturnData {
    instruction *ret; //!< return address
    Object *thisptr; //!< "this"
    int vstackbase; //!< start of current function's local area on vstack
    int vstacknext; //!< where the next function's local area will start on vstack
    int stkbase; //!< the depth of the main stack on function entry
    constid file; //!< debugging data - filename constant string desc.
    constid line; //!< debugging data - current line
};

/// this is an ugly solution to a nasty problem. Custom subclasses of Object override
/// Object::getprop() to return a Value pointer. Unfortunately, the property we want to
/// get is often just a primitive data type, or a pointer - not a Value. We therefore need
/// to wrap the data in a Value before we can return it. The only ideal solution is for each
/// property to always be embedded in a Lana value. That's the best way to do it, but it can
/// add a lot of memory (8 bytes per time.) This is an attempt at a cheaper solution: a cyclic
/// buffer of Value structures, from which we can allocate new ones. There don't need to be many:
/// the number is the maximum possible number of deref() operations that can occur between a deref()
/// being done and the dereffed value being used. There will also be a slight delay to finalisation
/// of GC data, because the last reference will only be lost once the buffer wraps round and a value
/// is overwritten. To deal with this, the buffer can be cleared from time to time.
///

class CyclicValueBuffer {
    static const int SIZE=16;
    int count;
    Value d[SIZE];

public:

    CyclicValueBuffer(){
	count=0;
	for(int i=0;i<SIZE;i++)
	    d[i].initNone();
    }

    ~CyclicValueBuffer(){
	clear();
    }

    void clear(){
	for(int i=0;i<SIZE;i++){
	    if(d[i].type)
		d[i].clr();
	}
    }

    Value *alloc(){
	Value *v = &d[count++];
	if(count==SIZE)
	    count=0;
	return v;
    }
};


/// this is the Virtual Machine for Lana
class VirtualMachine {
public:
    VirtualMachine(class Language *l){
        file = Constants::NOTFOUND;
        line = Constants::NOTFOUND;
        lana = l;
        consts = l->consts;
        globs = l->globs;
        vstacknext=0;
        vstackbase=0;
        thisptr=NULL;
    }
    ~VirtualMachine();
    
    static const int VSTACKSIZE = 1024; //!< size of the variable stack
    static const int EXSTACKSIZE = 128; //!< size of the execution stack
    
    /// set the start and reset the system, then call run(). We need to tell
    /// the system what the session is so it can find session variables.
    void interpret(instruction *start,Session *s);
    
    /// the core of the interpreter - run from current instruction address
    /// until OP_RETURN or OP_END executes with an empty return stack,
    void run(Session *s);
    
    /// pop a value and deference until it's just a plain value
    Value *popval(){
        Value *v = xstack.popptr();
        v=v->deref();
        if(v->type == Types::vtObject){
            if(v->d.gc->refct==0)
                error("snark");
        }
        
        return v;
    }
    
    /// pop a value and don't dereference it. Used for things which
    /// need to actually manipulate a reference
    Value *popvalnoderef(){
        Value *v = xstack.popptr();
        return v;
    }
    
    
    /// pop a value if one exists, and deref and return it.
    /// if there's no value return NULL
    Value *popvalnoexception(){
        Value *v = xstack.popptrnoex();
        if(v)
            v=v->deref();
        return v;
    }
    
    /// push a new value on a stack returning a pointer
    /// to the new value
    Value *pushptr(){
        return xstack.pushptr();
    }
    
    /// get the source line, or -1 if debugging wasn't enabled (the LDEBUG_SRCDATA debug flag
    /// must be set during compile)    
    int getSourceLine(){
        return line;
    }
    
    /// get the source file, or NULL if debugging wasn't enabled (the LDEBUG_SRCDATA debug flag
    /// must be set during compile)
    const char *getSourceFile();
    
    
    /// reset instruction counter
    void resetInstructionCount(){
        instct=0;
    }
    
    /// return number of instructions run since last resetInstructionCounter()
    int getInstructionCount(){
        return instct;
    }
    
    /// throw a runtime exception with the current line number and filename
    /// if available (by setting LDEBUG_SRCDATA during compilation)
    void error(const char *s,...);
    
    /// the session in which we are currently running code
    class Session *curSession;
    

    /// the cyclic value buffer for user value allocations. Ugh.
    CyclicValueBuffer cvb;
    
private:
    class Language *lana;
    class Constants *consts;
    class Vars *globs;
    
    /// debugging instruction counter
    int instct;
    
    
    /// do a function call
    void doFuncCall(instruction op);
    
    void rpush(); //!< push execution context
    bool rpop(); //!< pop execution context, return false if there isn't any more
    
    /// completely clear the execution stack
    /// and the variable stack
    
    void clearstacks();
    
    /// special commands! Usually debugging etc.
    void doSpecial(int spec);
    
    // execution context
    
    SimpleStack<Value,EXSTACKSIZE> xstack;	//!< execution stack
    instruction *ip; 	//!< instruction pointer
    Value *locals;	//!< local variable table
    Object *thisptr;	//!< current object pointer
    
    Value vstack[VSTACKSIZE]; //!< variable stack    
    int vstackbase;           //!< variable stack base for current function
    int vstacknext;           //!< variable stack base for next function: vstackbase+nlocals+nparams
    int stkbase; //!< the depth of the main stack on function entry
    int exprstackct; //!< the depth of the main stack on entering a stmt-expr - is reset to this afterwards. See OP_STARTESTMT/OP_ENDESTMT
    int file; //!< debugging data - filename constant string desc.
    int line; //!< debugging data - current line
    
    SimpleStack<ReturnData,128> retstack; //!< return stack
    
    char *stkDump(); //!< debugging routine, returns a representation of the stack
    
    
    /// clear the stack and set all values to None, do the same with the vstack.
    /// Should cause any zombies - objects left in the stack and locals but past the end
    /// of the stack pointers - to clean up.
    void clearAndFlush();
};

}

#endif /* __VM_H */
