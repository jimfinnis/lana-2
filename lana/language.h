/** @file
 * Contains the declaration of the main Lana class, which does all
 * the parsing and recreation work - implementation of this class
 * is spread across lana.cpp, expr.cpp and recreate.cpp.
 * This class is not for user use, it's wrapped by lana::API.
 */

#ifndef __LANA_H
#define __LANA_H

#include "exception.h"
#include "consts.h"
#include "pool.h"
#include "vars.h"
#include "cg.h"
#include "debug.h"
#include "flags.h"
#include "cycle.h"
#include "intkeyedhash.h"
#include "listener.h"
#include "natfunc.h"

using namespace lana;

namespace lana {

/** The main Lana class, which does all
 * the parsing and recreation work - implementation of this class
 * is spread across lana.cpp, expr.cpp and recreate.cpp.
 * This class is not for user use, it's wrapped by lana::API.
 */

class Language {
    friend class CodeGen;
    friend class VirtualMachine;
    friend class Session;
    friend class API;
    friend class Compiler;
    friend class Serialiser;
private:
    /// the tokeniser object used for lexical analysis
    class Tokeniser *tok;
    
    /// the global variables
    GlobalVars *globs;
    
    /// the virtual machine
    class VirtualMachine *vm;
    
    /// various debugging flags - see debug.h and setDebug()
    int debugFlags;
    /// various operation flags - see flags.h and setFlags()
    int opFlags;
    
public:
    Language(class API *a);
    ~Language();
    
    /// set the constant area for values to my constant area
    void setValueConsts(){
        Value::setConsts(consts);
    }
        
    
    /// set debugging flags, returning previous value
    int setDebug(int flags){
        int old = debugFlags;
        debugFlags = flags;
        return old;
    }
    
    /// set operating flags, returning previous value
    int setFlags(int f){
        int old = opFlags;
        opFlags=f;
        return old;
    }
    
    
    /// are we awaiting more input for the current function?
    bool awaitingInput();
    
    /// generate source code
    char *recreate(instruction *p,class Session *s);
    
    /// register a global variable,returning a pointer to it. Will
    /// panic if the variable already exists. Resets the global variable
    /// system variable marker; any globals defined before (and including)
    /// this one will not be serialised.
    Value *registerGlobalVariable(const char *name);
    
    /// register a native function which is a member of a host object
    /// (the usual)
    void registerNativeFunction(const char *name,int argc,bool returns,
                                class Host *h,
                                HOSTMETHOD m);
    
    /// register a native function calling a plain function
    /// (often used for object creation static methods)
    void registerStaticNativeFunction(class API *a,const char *name,int argc,bool returns,
                                      VOIDFUNC f);
    
    
    /// register an ID - really, just calls the constant module with this string and returns
    /// the constant's ID
    constid registerID(const char *id);
    
    
    
    /// there's nothing for it; things like IteratorObject need access to the API
    /// inside vm.cpp, so we need it here.
    class API *getAPI(){
        return api;
    }
    
    /// the constant storage area and the constant descriptor
    /// table manager
    Constants *consts;
    
    /// the native function registry
    NativeRegistry natFuncs;
    
private:
    /// internal debugging
    void dprintf(const char *s,...);
    
    /// the API which made us
    class API *api;
    
    
    // recreator data
    
    /// set the current LDT used for dumping and recreation - note, uses the 
    /// direct offset, not the descriptor. 
    void setRecreateLDT(constid ldtid);
    /// dump an instruction - for locals, an LDT must be set
    char *dumpInst(instruction *inst,Session *ses);
    /// dump a code block to stdout
    void dumpCode(instruction *firstinst,int size,Session *s);
    
        
    
    /// allocates string
    void recpush(char *s);
    /// returns string which you must free
    char *recpop();
    /// peek, with the option of peeking N under the top
    char *recpeek(int n=0);
    
    /// flush the recreator stack, and write out the items in
    /// reverse order, writing to the growable passed in. 
    void recflush(Growable *g);
    
    
    /// this actually writes the statement out in most cases
    void recreateStmtEnd(Growable *g,instruction *next);
    
    /// Need the next instruction for
    /// detecting comment at end of function def openings
    void recreateLiteral(int n,instruction *next,Session *ses);
    void recreateBinop(const char *op);
    void recreateUnOp(const char *op);
    void recreateAssign();
    void recreatePropRef(int d);
    void recreateComment(Growable *g,int d);
public: // so that the serializer can use it
    char *recreateFunction(instruction *ptr,int size,Session *ses);
    int recreateIndent; //!< indent count
    char *indents(); //!< return recreateIndent spaces
private:
    void recreateFuncCall(int argc);
    char *getComment(int n);
    
    bool recreateQuickIf; //!< currently recreating a quickif
    
    /// the LDT for the function we are currently recreating
    struct LDTHeader *currentRecreateLDT;
    
    /// a temporary growable for the recreator
    Growable *tmpgrow;
    
    char *getLocalName(int n); //!< get name of local in current LDT - number is +nparams!
    char *getParamName(int n); //!< get name of param in current LDT
    char *getGlobalName(int n); //!< get name of global
    
    
    /// end-of-funcdef comment
    instruction *eofdcomment;
    
    /// cycle detector system
    CycleDetector cycle;
    
};    

struct BinaryOperator {
    int token,opcode,precedence;
    const char *symbol;
    static BinaryOperator *getbinopbyop(int token);
    static BinaryOperator *getbinopbytok(int token);
};


}
            

#endif /* __LANA_H */
