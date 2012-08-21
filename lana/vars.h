#ifndef __VARS_H
#define __VARS_H

/**
 * @file
 * Code for handling global variables.
 */


#include "value.h"
#include "growable.h"
#include "pool.h"
#include "intkeyedhash.h"

namespace lana {


/// this class encapsulates a set of variables in a single namespace. There is one global set,
/// and a set for each session. In prior versions of Lana it was possible to promote an existing
/// variable from session to global, moving it from one set to another - this is no longer possible
/// because of the problems it would cause in usage: consider one user promoting a variable another was using as
/// a session. Also, globals need to be lexically distinct from session variables so that users don't inadvertently
/// overwrite some global or other. This is not enforced in this code, but rather in the parser.

class Vars {
    
protected:
    
    /// one of these for each variable
    struct vardata {
        unsigned nameAndFlags; //!< name descriptor, bottom 24 bits; flags, top 4 bits.
        Value v; //!< value of variable
    };
    
    Pool<vardata,128> pool; //!< pool of values
    IntKeyedHash<int> hash; //!< hash from name to pool slot
    
    class Constants *consts; //!< handy pointer
    
public:
    Vars(class Constants *c);
    ~Vars();
    
    /// clear all and return their slots to the pool 
    void clear();
    
    /// get variable ID (slot number) by name or -1
    int find(const char *name);
    
    /// get variable ID (slot number) by name descriptor of name string or -1
    int find(int desc);
    
    /// create new variable with a given descriptor returning the slot number
    virtual int create(int namedesc,int flags=0);
    
    /// get the value by slot
    Value *get(int id);
    
    /// get the name descriptor by slot
    int getName(int id);
    /// get the flags by slot
    int getFlags(int id);
    
    /// set the flags in a slot
    void setFlags(int id, int flags);
    
    /// find a value in the table by the value's hash, returning the slot,
    /// a bit slow but only used in serialisation. Compares each value's getHash() if
    /// its hashable with the target.
    /// Should only be used if val->type->serHash would be true. Returns -1 if no
    /// value found.
    
    int findByHashableValue(u32 target);
    
    /// return an ID iterator
    Iterator<int> *createIterator();
    
    void dump();
    
};


/// global variables have a little more functionality to support
/// only serialising non-system variables.

class GlobalVars : public Vars {
    
    /// This is a user variable and should be saved.
    static const int VARF_USER = 1;
    
public:
    
    GlobalVars(Constants *c) : Vars(c) {
        userFlag = false;
    }
    
    /// used by serialiser, and only for globals - is this a user defined global, or a system one
    /// which should not be serialised?
    bool isUser(int id){
        return (getFlags(id) & VARF_USER)?true:false;
    }
    
    /// this is called as soon as all the system globals have been created.
    /// It makes all existing variables system, and any created after
    /// this user variables.
    
    void setSystemGlobMarker();
    
    /// overrides create, setting the VARF_USER flag if setSystemGlobMarker() has been called.
    virtual int create(int namedesc,int flags=0){
        Vars::create(namedesc,userFlag?VARF_USER:0);
    }
    
    
private:
    bool userFlag; //!< create all variables after this as user vars, which will get saved.
    
};

}

#endif /* __VARS_H */
