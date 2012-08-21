/**
 * @file
 * Provides support for "goto" labels, also used in "break"
 * and "continue".
 */

#ifndef __LABEL_H
#define __LABEL_H

#include "value.h"
#include "consts.h"
#include "exception.h"

namespace lana {

/// A label is a reference to an instruction to which another instruction
/// can jump. Each label is in one of two states - set or unset.
/// Initially, a label is unset - we don't know where it is.
/// Unset labels have a set of "unresolved jumps" - instructions which jump
/// to the label's location, but which we can't complete until we
/// know where the label is.
/// Once a label has been set, all unresolved jumps are "resolved" -
/// completed with the correct jump offset - and all subsequent jumps
/// to that label are resolved immediately.


class Label 
{
    /// how many jumps this label can have pointing to it
    static const int MAXJUMPSPERLABEL=64;

public:
    /// create a new unset label
    Label(){
        pos = NULL;
        jumpct=0;
    }
    
    /// reinitialise the label, clearing all jumps and unsetting it. 
    /// Used when the structure is reused.
    void reset(){
        if(jumpct){
            throw ParseException("undefined label");
        }
        pos = NULL;
        jumpct=0;
    }
    
    /// if a label is deleted with pending jumps an exception occurs.
    ~Label() {
        reset();
    }
    
    /// add a jump - for an unset label, this will add it to
    /// the jump list; for a set label it will resolve immediately
    /// and the instruction will be written back with the correct offset.
    void jumpFrom(instruction *op) {
        if(pos)
            resolve(op);
        else {
            if(jumpct<MAXJUMPSPERLABEL)
                jumps[jumpct++]=op;
        }
    }
        
    
    /// set the label - all jumps will be resolved, and
    /// future jumps added will be resolved immediately
    void set(instruction *op) {
        pos = op;
        for(int i=0;i<jumpct;i++)
            resolve(jumps[i]);
        jumpct=0;
    }
    
    /// is this label set?
    bool isSet() {
        return pos ? true : false;
    }
    
private:
    /// resolve a jump - the label must have been set.
    void resolve(instruction *op) {
        if(!pos)
            throw Exception("attempt to resolve unset label");
        int diff = pos-op;
        
        // if's it's a jump which can be bidirectional, change it to
        // the appropriate direction instruction.
        int inst = INSTOP(*op);
        if(inst==OP_GOTOFW)
            inst = (diff>0)?OP_GOTOFW:OP_GOTOBK;
        if(diff<0)diff=-diff;
        *op = INST(inst,diff);
    }
    
    
    /// number of jumps unresolved
    int jumpct;
    /// jump array
    instruction *jumps[MAXJUMPSPERLABEL];
    /// position of label. If null, label is unset
    instruction *pos;
};

}
#endif /* __LABEL_H */
