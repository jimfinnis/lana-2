/**
 * @file
 * Contains the Compiler class.
 *
 */

#ifndef __COMPILER_H
#define __COMPILER_H

namespace lana {

/// Each user's session has a Compiler class, so that many users can interact with the
/// system at the same time.

class Compiler {
public:
    
    
    /// Create the compiler from the session
    Compiler(class Session *s,class Language *l);
    
    /// delete the compiler and composing objects (code generator, etc.)
    ~Compiler();
    
    /// cause a ParseException to be thrown
    void error(const char *s,...);
    
    /// add an event listener
    void addListener(event e, Listener *p);
    
    /// feed a string to the interpreter. A single line to
    /// run immediately, or one line of a function definition.
    
    void feed(const char *s);
    
    /// feed text from a file into the interpreter using feed()
    void feedFile(const char *fileName);
    
    /// are we awaiting more input for the current function?
    bool awaitingInput();


    /// generate OP_SRCFILE and OP_SRCLINE if required - if alwaysFileName is true, OP_SRCFILE will be
    /// generated regardless of whether it's been done for this file already. This is typically used
    /// at the start of functions.
    void generateDebugInstructions(bool alwaysFileName);
    
    /// get last line fed in (just returns pointer most recently passed
    /// to feed() )
    const char *getLastLine(){
        return lastLine;
    }
    
    /// get filename currently being interpreted; set by API::feedFile()
    const char *getFileName(){
        return fileName;
    }
    
    
    
private:
    
    /// the core Language object
    class Language *lana;
    
    /// the session
    class Session *ses;
    
    /// is the expression Parser expect a value or an operator
    bool expectingValue;
    
    /// the tokeniser
    class Tokeniser *tok;
    
    /// should we output the filename before the next instruction in feed()?
    bool outputFileName;
    
    /// get the index of a local variable or -1
    int findLocal(int namedesc);
    /// get the index of a local parameter or -1
    int findParam(int namedesc);
    /// add a local variable, return the index
    int createLocal(int namedesc);
    
    /// the code generator object for this session and its associated memory
    class CodeGen *cg;
    
    /// last line fed to feed()
    char *lastLine;
    
    /// filename for debugging
    char *fileName;
    
    
    /// event listener manager
    class EventMgr *eventMgr;
    
    /// we're going to start parsing a new file, will reset line number
    /// and set source name (will copy, deleting old). Will also notify
    /// the system to output a new source file id at next feed.
    void notifyNewFile(const char *name);
            
    /// current source line number for debugging
    int lineNumber;

    
    
    
    
    
    // scanners
    
    void scanStmt();
    void scanFuncCall();
    /// return true if we did a function start or some other oddity which doesn't require a clear.
    /// The argument is true if an identifier followed by a colon is a label - i.e. we are
    /// parsing an expression statement.
    bool scanExpr(bool topLevel=false);
    /// scan the header of a function/procedure - takes true if it's
    /// a true function returning a value.
    void scanFuncHeader(bool isfunc);
    void scanEndFunc();
    void scanGoto();
    void scanFor();
    void scanEndFor();
    void scanGlobal();
    
    void scanComment(bool startofline);
    void scanPossibleComment();
    
    /// emit a number as a literal, creating in the constants area if required.
    void emitLiteralFloat(float f);
    /// emit a number as a literal, creating in the constants area if required.
    void emitLiteralInt(int i);
    /// emit a string as a literal, creating in the constants area if required.
    void emitLiteralString(const char *s);
    
    /// process a binary operation in an expression, taking the token
    /// for the operation
    void doBinOp(int t);
    
    /// used to process extended operators pairs, such as "<" and "<=",
    /// which would be called as
    /// \code
    /// doBinOpExtended(T_EQUALS,T_LT,T_LTE);
    /// \endcode
    ///
    void doBinOpExtended(int tok2, //!< second token if in extended form
                         int op1, //!< exprtoken to use if not extended
                         int op2 //!< exprtoken to use if extended
                         );
    
    /// expression parser: push a value onto the expression stack
    void epush(int t,int p,const char *s);
    /// expression parser: output an operator from the expression stack
    void outputoperator(struct ExprItem *e);
    /// process the end of an expression
    void doExprEnd();
    
    /// output instructions to stack a variable, looking it 
    /// up in local, then parameter, then global tables.
    /// By default this will output a get, but it could
    /// later get changed to a get..
    void emitVariableRef(const char *s);
    
    /// scan a property dereference - we've just scanned a dot, the next thing
    /// has to be a property ident
    void scanPropDeref();
    
    
    
};

}

#endif /* __COMPILER_H */
