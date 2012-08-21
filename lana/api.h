#ifndef __API_H
#define __API_H

/** 
 * \file
 * This file describes the public Lana API
 * 
 * The API is a facade around the internal Lana classes.
 */

#include "value.h"
#include "listener.h"
#include "session.h"

namespace lana {

    

/// The main public Lana API - a facade hiding the Lana classes,
/// primarily Lana itself. All access to the Lana system should be
/// through this class.


class API {
public:
    /// create the Lana objects
    API();
    /// destroy the Lana objects
    ~API();
    
    /// get the version number of the library
    const char *getVersion();
    
    /// create a native function constant entry - a data block
    /// identifying a native function under a given name (not always the name the function or
    /// method will be called by but a unique, internal name it will be serialised by.)
    
    NativeFuncData *registerNativeMethod(const char *internalname,
                                           int argc,bool returns,HOSTMETHOD m,
                                           class Host *h=NULL);
    
    
    /// create a global variable referencing the native function data.
    
    void globalNativeFunctionOrMethod(const char *name,class NativeFuncData *nd);
    
    /// does registerNativeFunction() and globalNativeFunctionOrMethod() - typically
    /// used for "hosted" methods.
    /// The internal name (or the name under which
    /// references to such methods are serialised) is formed by adding the current prefix
    /// to the passed name. This is set with setNamePrefix().
    
    void globalNativeHostedMethod(const char *name,
                                  int argc,bool returns,
                                  class Host *h,HOSTMETHOD m);
    
    /// register a *real* native function, a plain function which will
    /// take this API as its sole argument.
    /// The internal name (or the name under which
    /// references to such methods are serialised) is formed by adding the current prefix
    /// to the passed name. This is set with setNamePrefix().
    
    void globalNativeFunction(const char *name,int argc,bool returns,
                                        VOIDFUNC f);
    
    /// set the internal name prefix used for functions and methods internal names. This
    /// must be a constant
    void setNamePrefix(const char *pre){
        prefix = pre;
    }
    
    /// pop an integer from the stack, Coerces if possible.
    int popInt();
    /// push an integer onto the stack
    void pushInt(int i);
    /// push a raw value onto the stack - returns the value to be written to
    Value *pushRaw();
    
    
    /// pop an object
    class Object *popObj();
    /// push an object
    void pushObj(class Object *o);
    
    /// set LDEBUG_ flags, returning previous value
    int setDebug(int flags);
    /// set LOP_ flags, returning previous value
    int setFlags(int flags);
    
    /// return a pointer to the value of a global or NULL if it
    /// doesn't exist
    Value *findGlobal(const char *name);
    
    /// clear all globals - functions, variables, builtins...
    void clearGlobals();
    
    
    /// return a pointer to the value of a global or create it
    /// if it doesn't exist
    Value *findOrCreateGlobal(const char *name);

    /// pop a float from the stack. Coerces if possible.
    float popFloat();
    /// push a float onto the stack
    void pushFloat(float f);
    
    /// pop a string from the stack - NOT a copy. Coerces if possible.
    char *popStr();
    /// push a float onto the stack, making a copy
    void pushStr(char *s);
    
    /// pop a boolean from the stack - NO COERCION IS DONE
    int popBool();
    /// push a boolean onto the stack
    void pushBool(bool b);
    
    /// pop a raw value
    Value *popRaw();
    
    /// pop a raw value, not even dereferencing first!
    Value *popRawWithoutDeref();
    
    /// reset the instruction counter
    void resetInstructionCount();
    /// read the instruction counter
    int getInstructionCount();
    
    /// a fatal error method you might need - throws a runtime exception
    void error(const char *s);
    
    /// get a string ID given a string (i.e. create or find a string
    /// constant)
    int getID(const char *s);
    
    /// get the source line, or -1 if debugging wasn't enabled (the LDEBUG_SRCDATA debug flag
    /// must be set during compile) (only works during exceptions)
    int getSourceLine();
    
    /// get the source file, or NULL if debugging wasn't enabled (the LDEBUG_SRCDATA debug flag
    /// must be set during compile) (only works during exceptions)
    const char *getSourceFile();

    /// this will allocate a new value from the VM's cyclic value buffer. This will not be
    /// valid very long - each value will be cleared after 16 dereferences have been done, and
    /// also at the end of every statement. They're here for you to wrap data in Value structures
    /// for returning from custom objects in their getprop() methods. Ideally you should use
    /// Value structures in your objects, but the overhead (both in the computer's memory and
    /// your own brainspace) could get nasty.

    Value *getTempValue();
    
    
    // no harm in these being public, they're hidden behind the facade
    
    /// the lana object hidden by this facade
    class Language *lana;
    /// and a handy pointer to the VM
    class VirtualMachine *vm;
    /// and a handy pointer to the cycle detector
    class CycleDetector *cycle;
    
    /// a pointer to the object hosting the core native functions
    class Host *coreLib;
    
    /// set argc and argv for libcore args() function
    void setArgcArgv(int argc,char **argv);
    
private:
    /// internal name prefix for functions and methods
    const char *prefix;
    
    /// get prefixed name, returns ptr to static buffer
    const char *getPrefixedName(const char *s){
        static char buf[256];
        strncpy(buf,prefix,256);
        strncat(buf,s,256);
        return buf;
    }
};


}

#endif /* __API_H */
