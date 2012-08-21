/**
 * @file
 * Defines the Lana session, through which a user communicates with Lana.
 *
 * 
 * 
 */

#ifndef __SESSION_H
#define __SESSION_H

namespace lana {

/// this is a session object, through which the embedding application
/// communicates with Lana to compile and run code.. Because multiple users
/// may be using Lana (for example, in a server) each needs to create one of
/// these first. Note that Lana is not thread-safe, however (yet.) 

class Session {
    friend class CodeGen;
    friend class Serialiser;
    
    /// the API's Language object
    class Language *lana;
    
    /// the session's compiler object
    class Compiler *compiler;
    
public:
    /// create a new session
    Session(class API *a);
    /// delete the session and all the variables, dereffing them
    ~Session();
    
    /// feed a string to the interpreter. A single line to
    /// run immediately, or one line of a function definition.
    void feed(const char *s);
    
    /// feed text from a file into the interpreter using feed()
    void feedFile(const char *fileName);
    
    /// true if the compiler is building a function or procedure.
    /// Typically used to modify the prompt.
    bool awaitingInput();
    
    /// return a recreation of a stream of instructions. Typically
    /// used in parse listeners.
    const char *recreate(instruction *op);

    /// add an event listener
    void addListener(event e, Listener *p);
    
    /// get last line parsed
    const char *getLastLine();
    
    
    /// find a session variable, creating it if it doesn't exist. This returns
    /// the pool index. Takes the long key, the string descriptor
    int findOrCreateSesVar(u32 desc);
    
    /// find a session variable by string descriptor
    int findSesVar(u32 desc);
    
    /// get a session variable by name, if doesn't exist will return NULL
    /// unless create is set, in which case it will create it.
    Value *getSesVar(const char *name,bool create=false);
    
    /// this gets the value from the pool index returned by findOrCreateSesVar
    Value *getSesVar(int idx);
    
    /// get the session variable name given the pool index
    char *getSesVarName(int idx);
    
    /// the API
    class API *api;
    
    /// a namespace for variables
    class Vars *vars;
};
    
    

}
#endif /* __SESSION_H */
