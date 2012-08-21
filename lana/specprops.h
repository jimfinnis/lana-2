#ifndef __SPECPROPS_H
#define __SPECPROPS_H

namespace lana {

/// this holds special property IDs. Add to it when you add properties.

class PropIDs {
private:
    /// register a property ID string, and return the ID.
    constid add(const char *id);
    
    /// pointer to the constants
    class Constants *consts;
    
public:
    /// add all the special properties:
    /// insert "add" calls to this for each new property name.
    
    void setUp(class Constants *c){
        consts = c;
    
        propDel = add("del");
        propSize = add("size");
        propValues = add("values");
        propKeys = add("keys");
        propFirst = add("first");
        propNext = add("next");
        propIsDone = add("isDone");
        propCurrent = add("current");
    }
    
    // this is a list of the actual property IDs. They're set up in setUp().
    
    constid propDel,propSize;
    constid propValues,propKeys,propCurrent,propIsDone,propFirst,propNext;
};

}
#endif /* __SPECPROPS_H */
