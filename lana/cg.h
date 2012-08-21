#ifndef __CG_H
#define __CG_H

/** @file
 * Classes such as CodeGen and CodeGenContext, involved in code generation
 */

#include <map>
#include "growable.h"
#include "stack.h"
#include "value.h"
#include "consts.h"
#include "opcodes.h"
#include "label.h"

namespace lana {

#define MAXLOCALS 128

/// operator stack item used by the expression parser's shunting yard algorithm
struct ExprItem {
    int type;
    int precedence;
};

/// loop stack item used to manage loop begin/end labels for break and continue.
/// These are placed onto the loop stack whenever a loop starts, and popped off at the
/// end by which time any jumps should have been resolved.
struct LoopData {
    
    /// reinitialise the structure when reused in the stack
    void reset(){
        continuelabel.reset();
        breaklabel.reset();
    }
        
    /// destination of "continue" for this loop
    Label continuelabel;
    /// destination of "break" of this loop
    Label breaklabel;
};

/// There may be some more things, but basically a compilation
/// context consists of a growable into which code is being
/// written. A new context is created and the old one pushed onto
/// the code generator's stack whenever a new function starts to
/// be parsed.

class CodeGenContext {
public:
    CodeGenContext();
    ~CodeGenContext();
    
    /// clear the compiler stack and code area in the current
    /// context
    void clear();
    
    class Growable *code;
    
    /// start compiling a statement
    void saveSnapshot();
    /// the statement is a dud, remove any old crap
    void restoreSnapshot();
    
    /// get the location as an integer offset
    int getloc();
    /// get the location as an instruction ptr (will not have been written to!)
    instruction *getlocptr();
    
    /// compiler stack push
    void cpush(int n);
    // special version, pushes the current offset being written to - IN INSTRUCTIONS, not bytes.
    void cpushhere();
    /// compiler stack pop
    int cpop();
    /// special version, pops an instruction offset
    instruction *cpoplocation();
    /// extra special version, pops and ensures it's a correct op, else returns NULL.
    /// The validity check is that the opcode lies between two values! Ugly but easy.
    instruction *cpoplocandcheck(int opmin,int opmax);
    /// compiler stack peek
    int cpeek();
    /// return the distance from an instruction location to the current
    /// location
    int getdiff(instruction *op);
    /// used to ensure the compiler stack is empty
    bool cempty();
    
    /// get the pointer to a given location in the output
    /// stream
    instruction *getPtr(int offset);
    
    /// push an item onto the expression stack given its
    /// token and priority
    void epush(int t,int p);
    /// pop an item off the expression stack
    ExprItem *epop();
    /// look at the top item on the expression stack
    ExprItem *epeek();

    /// stack of expression stacks for the shunting yard
    StackableStack<ExprItem,128> estack;
    
    LDTHeader ldth; //!< header of LDT in current function
    short locals[MAXLOCALS]; //!< names of locals in current function
    short params[MAXLOCALS]; //!< names of params in current function
    
    /// create a new loop here - this will create and push a loop data,
    /// and set the continue label to the current location
    void newloop();
    /// end the current loop here - this will set the break label to the
    /// current location (and thus resolve any break jumps.) It will then
    /// pop the loop stack.
    void endloop();
    
    
    /// this is (for now) an STL map of integer keys (string constants in the CDT)
    /// onto label pointers
    
    std::map<int,class Label *> labels;
    
    /// stack of loops, used to manage break/continue
    SimpleStack<LoopData,16> loopstack;
private:
    
    /// compile stack of integers; mainly accessed through the cpop/cpush/cpeek functions
    /// which trap the stack exceptions and turn them into parse exceptions.
    SimpleStack<int,16> cstack;
    
    
    /// Part of the snapshot. Horrible.
    std::map<int,class Label *> snaplabels;
    /// Part of the snapshot. Horrible.
    SimpleStack<int,16> snapcstack;
    /// Part of the snapshot. Horrible.
    SimpleStack<LoopData,16> snaploopstack;
    
};

/// the code generator, consisting of a stack of code generation contexts.
/// Emitting an instruction (via emitInst()) will generate code into the 
/// current context. This class mainly consists of operations for handling
/// the stack of CodeGenContext objects and passing calls through to 
/// the current context.

class CodeGen {
private:
    CodeGenContext *stack[8];
    int stackct;
    class Language *lana; //!< hate having to do this. Be careful!
    
public:
    CodeGen(class Language *l,class Session *s);
    ~CodeGen();
    
    /// clear the currently compiling context (does not clear the context
    /// stack)
    void clear();
    /// pop back to the main context and clear it
    void clearall();
    
    /// start compiling a statement
    void saveSnapshot();
    /// commit the statement
    void restoreSnapshot();
    
    /// push a new context - typically called when we start compiling
    /// function. 
    void pushContext();
    
    /// pop the context, for example at the end of a function
    void popContext();
    
    /// push the entire expression stack
    void pushestack();
    /// pop the entire expression stack. Pop-Estack, Not Pope-Stack, that would be silly.
    void popestack();
    
    /// write the currently compiling context into a newly allocated memory region
    /// and return it. The code follows the header.
    const char *writeContextToMemory();
    
    /// if we are compiling, we'll have pushed the code generation
    /// context
    bool isCompiling(){
        return stackct>0;
    }
        
    
    /// get the currently compiling code area
    class Growable *getCode(){
        return current->code;
    }
    /// current code generation context
    CodeGenContext *current;
    
    /// output an instruction with an optional data field
    void emit(int op,int n=0);
    
    /// check that all paths do, or don't, return a value
    /// in the code in the current context
    void checkReturns(bool rets);
    
    /// the session which created me (so I can access the session vars)
    class Session *ses;
};


}// namespace
#endif /* __CG_H */
