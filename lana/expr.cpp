#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "language.h"
#include "tokeniser.h"
#include "tokens.h"
#include "opcodes.h"
#include "cg.h"
#include "label.h"
#include "compiler.h"
#include "session.h"

// operator types


// these priorities come from the wikipedia article on C++ operator
// precedence.
static BinaryOperator binops[]={
    T_DIV,	OP_DIV,		5,	"/",
    T_PERC,	OP_MOD,		5,	"%",
    T_MUL,	OP_MUL,		5,	"*",
    T_ADD,	OP_ADD,		6,	"+",
    T_SUB,	OP_SUB,		6,	"-",
    T_EQUALITY,	OP_EQUALS,	9,	"==",
    T_LT,	OP_LT,		8,	"<",
    T_GT,	OP_GT,		8,	">",
    T_LTE,	OP_LTE,		8,	"<=",
    T_GTE,	OP_GTE,		8,	">=",
    
    T_NOTEQUALS,OP_NEQUALS,	9,	"!=",
    T_NEAREQ,	OP_NEAREQ,	9,	"~",
    T_NOTNEAREQUALS,OP_NNEAREQ,	9,	"!~",
    
    T_ASSIGN,	OP_SET,		16,	"=",
    
    // logicals
    T_LOGAND,   OP_LOGAND,	13,	"&&",
    T_LOGOR,	OP_LOGOR,	14,	"||",
    
    // bitwises
    
    T_BITAND,   OP_BITAND,	10,	"&",
    T_BITOR,	OP_BITOR,	12,	"_",
    T_XOR,	OP_XOR,		11,	"^",
    
    
    
    // weirdos and unaries
    T_OPREN,	OP_PAREN,	2,	"(",
    T_NEGATE,	OP_NEGATE,	3,	"-",
    T_BITNOT,	OP_BITNOT,	3,	"@",
    T_NOT,	OP_NOT,		3,	"!",
    T_OSQB,	OP_SQB,		2,	"[",
    T_END,0,0,
};

BinaryOperator *BinaryOperator::getbinopbytok(int token) {
    for(int i=0;;i++){
        if(binops[i].token == T_END)
            return NULL;
        if(binops[i].token == token)
            return binops+i;
    }
}

BinaryOperator *BinaryOperator::getbinopbyop(int opcode) {
    for(int i=0;;i++){
        if(binops[i].token == T_END)return NULL;
        if(binops[i].opcode == opcode)
            return binops+i;
    }
}




void Compiler::outputoperator(ExprItem *e){
    BinaryOperator *oper = BinaryOperator::getbinopbytok(e->type);
    cg->emit(oper->opcode);
}

void Compiler::emitLiteralString(const char *s) {
    int desc = lana->consts->findOrCreateString(s);
    cg->emit(OP_LIT,desc);
}

void Compiler::emitLiteralInt(int i) {
    // check that we can indeed make a valid data field
    // for an immediate without it running into the opcode
    if(i>=0 && i<(1<<24))
        cg->emit(OP_IMMED,i);
    else {
        int desc = lana->consts->findOrCreateInt(i);
        cg->emit(OP_LIT,desc);
    }
}

void Compiler::emitLiteralFloat(float f) {
    int desc = lana->consts->findOrCreateFloat(f);
    cg->emit(OP_LIT,desc);
}

// expression scanner
// scan position: before expression
bool Compiler::scanExpr(bool topLevel) {
    char tmp[256];
    int ct;
    expectingValue=true;
    
    // a bit of code duplication to detect labels here
    
    int t = tok->getnext();
    if(t == T_IDENT){
        // first, we check for the weird case of labels 
        strcpy(tmp,tok->getstring());
        if(tok->getnext() == T_COLON && topLevel) {
            // it's a label
            if(!cg->isCompiling())
                error("must be compiling a function/procedure to set a label");
            Label *lab;
            int d = lana->consts->findOrCreateString(tmp);
            // output the label instruction
            cg->emit(OP_LABEL,d);
            // the HIDEOUS hackery for checking if a value is present
            std::map<int,Label *>::iterator it = cg->current->labels.find(d);
            if(it != cg->current->labels.end()) {
                // there's already a definition
                lab = it->second;
                if(lab->isSet()) // we've set this elsewhere!
                    error("duplicate label: %s",tmp);
            } else {
                // label does not exist, so create it.
                lab = new Label();
                cg->current->labels[d] = lab;
            }
            // now we have our label, set it to the current instruction (the one after the label instruction)
            lab->set(cg->current->getlocptr()); // this will also cause existing gotos to be resolved.
            return true; // NOTE -  return true if it was a goto, indicating no CLEARSTK
        } else {
            tok->rewind();
            // it really is an expression
            
            // output a start of expression-stmt opcode
            if(topLevel)
                cg->emit(OP_STARTESTMT);
            
            if(!expectingValue)
                error("invalid id '%s'",tok->getstring());
            emitVariableRef(tok->getstring());
            expectingValue=false;
        }
    } else {
        // it's not an ident, and it is an expression
        tok->rewind(); // put the token back
        if(topLevel)
            cg->emit(OP_STARTESTMT); // output a start of expression-stmt opcode
    }
    
    for(;;){
        t = tok->getnext();
        
        // if it's a minus, do some magic to determine if its a unary
        // minus. Binary minus cannot appear after a left paren or
        // an operator.
        
        
        switch(t){
        case T_OSQB: // open square bracket
            // push the token
            if(expectingValue)
                error("invalid '['");
            cg->current->epush(t,2);
            expectingValue=true;
            break;
        case T_CSQB:
            if(expectingValue)
                error("invalid ']'");
            expectingValue=false;
            // this works rather like a normal CPREN (close bracket)
            for(;;){
                ExprItem *e = cg->current->epop();
                if(!e) // should have found T_OSQB by now!
                    error("mismatched ']'");
                outputoperator(e);
                if(e->type == T_OSQB)break;
            }
            break;
        case T_THIS:
            if(!cg->isCompiling())
                error("must be compiling a function/procedure to use 'this'");
            if(!expectingValue)
                error("invalid id '%s'",tok->getstring());
            cg->emit(OP_THIS);
            expectingValue=false;
            break;
        case T_IDENT: {
            // first, we check for the weird case of labels 
            strcpy(tmp,tok->getstring());
            if(tok->getnext() == T_COLON && topLevel) {
                if(!cg->isCompiling())
                    error("must be compiling a function/procedure to set a label");
                Label *lab;
                int d = lana->consts->findOrCreateString(tmp);
                // output the label instruction
                cg->emit(OP_LABEL,d);
                // the HIDEOUS hackery for checking if a value is present
                std::map<int,Label *>::iterator it = cg->current->labels.find(d);
                if(it != cg->current->labels.end()) {
                    // there's already a definition
                    lab = it->second;
                    if(lab->isSet()) // we've set this elsewhere!
                        error("duplicate label: %s",tmp);
                } else {
                    // label does not exist, so create it.
                    lab = new Label();
                    cg->current->labels[d] = lab;
                }
                // now we have our label, set it to the current instruction (the one after the label instruction)
                lab->set(cg->current->getlocptr()); // this will also cause existing gotos to be resolved.
                return true; // NOTE -  return true if it was a goto, indicating no CLEARSTK
            } else {
                tok->rewind();
                
                // it really is an expression
                if(!expectingValue)
                    error("invalid id '%s'",tok->getstring());
                emitVariableRef(tok->getstring());
                expectingValue=false;
            }
            
        }
            break; // otherwise just do the standard stuff.

        case T_BACKTICK:
            if(!expectingValue)
                error("invalid '%s'",tok->getstring());
            // it's a dollar followed by an ident, saying we should
            // stack the value of that ident as a constant descriptor index.
            // Used in things like obj.del($propname)
            if(tok->getnext() != T_IDENT)
                error("expecting an identifier after `");
            else {
                constid id = lana->consts->findOrCreateString(tok->getstring());
                cg->emit(OP_LITIDENT,id);
                expectingValue = false;
            }
            break;
        case T_TRUE:
            if(!expectingValue)
                error("invalid '%s'",tok->getstring());
            cg->emit(OP_TRUE);
            expectingValue=false;
            break;
        case T_FALSE:
            if(!expectingValue)
                error("invalid '%s'",tok->getstring());
            cg->emit(OP_FALSE);
            expectingValue=false;
            break;
        case T_DOT: // property dereference. What follows must be a global property identifier
            if(expectingValue)
                error("invalid '%s'",tok->getstring());
            scanPropDeref();
            expectingValue=false; //expecting operator
            break;
            
            // the awkward case
        case T_PROC:
        case T_FUNCTION:
            if(!expectingValue || cg->current->epeek()->type!=T_ASSIGN)
                error("function/procedure must follow assignment operator");
            
            // Write a dummy OP_DUMMY, stacking its location on
            // on the current context's cstack.
            
            // At function end, we'll pop off that value
            // and write a correct OP_LIT with the function's descriptor.
            
            cg->current->cpushhere();
            cg->emit(OP_DUMMY,100);
            
            // push the context
            cg->pushContext();
            // now parse the function header
            // now start to parse the function proper,
            // but first, write a dummy instruction for the OP_LOCALS and stack
            // where it'll be.
            cg->current->cpushhere();
            cg->emit(OP_DUMMY,101);
            scanFuncHeader(t==T_FUNCTION);
            // also output src file and line if in debug mode
            generateDebugInstructions(true);
            return true;
            
        case T_INT:
            if(!expectingValue)
                error("invalid value '%s'",tok->getstring());
            emitLiteralInt(tok->getint());
            expectingValue=false;
            break;
        case T_FLOAT:
            if(!expectingValue)
                error("invalid value '%s'",tok->getstring());
            emitLiteralFloat(tok->getfloat());
            expectingValue=false;
            break;
        case T_STRING:
            if(!expectingValue)
                error("invalid string '%s'",tok->getstring());
            emitLiteralString(tok->getstring());
            expectingValue=false;
            break;
        case T_COMMA:
            for(;;) {
                ExprItem *e = cg->current->epop();
                if(!e){
                    tok->rewind(); // put the comma back
                    return false; // END OF EXPR
                }
                //                dprintf("comma, popped %d",e->type);
                outputoperator(e);
                if(e->type == T_OPREN)break;
            }
            expectingValue=true;
            break;
            
            // binops            
            
        case T_BITAND:
            doBinOpExtended(T_BITAND,T_BITAND,T_LOGAND);
            break;
        case T_BITOR:
            doBinOpExtended(T_BITOR,T_BITOR,T_LOGOR);
            break;
        case T_LT:
            doBinOpExtended(T_EQUALS,T_LT,T_LTE);
            break;
        case T_GT:
            doBinOpExtended(T_EQUALS,T_GT,T_GTE);
            break;
        case T_BITNOT:
            if(!expectingValue)
                error("invalid @ operator");
            cg->current->epush(T_BITNOT,3);
            // still expecting a value...
            break;
        case T_PLING:
            // is it a !=, a !~ or just ! ?
            t = tok->getnext();
            if(t==T_EQUALS) {              // do NOTEQUALS
                if(expectingValue)
                    error("invalid binary operator after operator or (");
                doBinOp(T_NOTEQUALS);
                expectingValue=true;
            } else if(t==T_NEAREQ) {
                if(expectingValue)
                    error("invalid binary operator after operator or (");
                doBinOp(T_NOTNEAREQUALS);
                expectingValue=true;
            } else if(expectingValue){
                // unary !
                cg->current->epush(T_NOT,3);
                tok->rewind();
            } else error("invalid unary not");
            break;
        case T_EQUALS:
            doBinOpExtended(T_EQUALS,T_ASSIGN,T_EQUALITY);
            break;
        case T_SUB:
            if(expectingValue){
                // push negate
                cg->current->epush(T_NEGATE,3);
                break;
            }
            // fall through otherwise
        case T_MUL:
        case T_ADD:
        case T_PERC:
        case T_DIV:
        case T_XOR:
        case T_NEAREQ:
            if(expectingValue && t!=T_SUB)
                error("invalid operator '%s'",tok->getstring());
            doBinOp(t);
            expectingValue=true;
            break;
        case T_OPREN: // open bracket
            if(expectingValue){ // it's just a standard bracket
                //                dprintf("Pushing opren");
                cg->current->epush(t,2);
                expectingValue=true;
            } else {
                // it's a function call, or function-call-like pattern
                scanFuncCall();
                expectingValue=false;
            }
            break;
        case T_CPREN: // close bracket
            if(expectingValue)
                error("misplaced closing parenthesis");
            expectingValue=false;
            for(;;) {
                ExprItem *e = cg->current->epop();
                if(!e) { // out to parent
                    //                    dprintf("cpren, popped exit",e->s);
                    tok->rewind(); // put the bracket back!
                    return false; //EXIT
                }
                //                dprintf("cpren, popped %s",e->s);
                outputoperator(e);
                if(e->type == T_OPREN)break;
            }
            // THIS TEST SHOULDN'T HAPPEN
            if(cg->current->epeek() && cg->current->epeek()->type == T_IDENT) // function left on stack?
                outputoperator(cg->current->epop()); // pop it.
            break;
        case T_END:
            doExprEnd();
            return false;
            break;
        case T_COMMENT:
            // rest of the line is a comment, put it back and terminate the expression
        case T_COLON:
            // we also end the expression on a colon, for quick-if, but
            // we put the colon back into the tokeniser!
            tok->rewind();
            doExprEnd();
            return false;
        }
    }
    return false;
}
void Compiler::doExprEnd(){
    for(;;) {
        ExprItem *e = cg->current->epop();
        if(!e)return;
        //                dprintf("end, popped %s",e->s);
        if(e->type == T_OPREN)
            error("misplaced separator or mismatched parentheses");
        outputoperator(e);
    }
}

void Compiler::doBinOpExtended(int tok2,int op1,int op2) {
    int t;
    if(expectingValue)
        error("invalid binary operator after operator or (");
    t = tok->getnext();
    if(t==tok2){
        doBinOp(op2);
    } else {
        doBinOp(op1);
        tok->rewind();
    }
    expectingValue=true;
}


// scan a property dereference .. EXPR . *** PROPNAME

void Compiler::scanPropDeref(){
    int t = tok->getnext();
    if(t!=T_IDENT)
        error("expected a property name after '.'");
    int id = lana->consts->findOrCreateString(tok->getstring());
    
    // we just output that straight away
    cg->emit(OP_PROPREF,id);
}



void Compiler::doBinOp(int t){
    BinaryOperator *oper = BinaryOperator::getbinopbytok(t);
    for(;;){
        ExprItem *e = cg->current->epeek();
        if(!e || e->type == T_OPREN || e->type == T_OSQB || (oper->precedence < e->precedence))
            break;
        e = cg->current->epop();
        outputoperator(e);
    }
    
    cg->current->epush(t,oper->precedence);
}



void Compiler::emitVariableRef(const char *s) {
    
    int namedesc = lana->consts->findOrCreateString(s);
    int i;

    if(cg->isCompiling()){
        if((i=findLocal(namedesc))>=0){
            // first try local variables
            i+=cg->current->ldth.numparams; // add the number of parameters, so locals follow parameters
            cg->emit(OP_VARREFLOC,i);
        } else if((i=findParam(namedesc))>=0){
            // then try parameters
            cg->emit(OP_VARREFPRM,i);
        } else if((i=ses->findSesVar(namedesc))>=0){
	  // then session.
            cg->emit(OP_VARREFSES,i);
	} else if((i=lana->globs->find(namedesc))>=0){
	    // then global
	    cg->emit(OP_VARREFGLB,i);
        }  else {
            // didn't find it; we're compiling - if it starts with a $ create a
	    // global, else create a local.
	    if(*s == '$'){
	      i=lana->globs->create(namedesc);
	      cg->emit(OP_VARREFGLB,i);
	    } else {
	      i=createLocal(namedesc)+cg->current->ldth.numparams; // see above
	      cg->emit(OP_VARREFLOC,i);
	    }
        }
    } else {
      // if not compiling, it must be a session variable if not a global
      if((i=ses->findSesVar(namedesc))>=0){
	cg->emit(OP_VARREFSES,i);
      } else if((i=lana->globs->find(namedesc))>=0){
	cg->emit(OP_VARREFGLB,i);
      } else {
	if(*s == '$'){
	  i=lana->globs->create(namedesc);
	  cg->emit(OP_VARREFGLB,i);
	} else {
	  cg->emit(OP_VARREFSES,ses->findOrCreateSesVar(namedesc));
	}
      }
    }
}


/// scan function call
/// scan position: varname ( *** expr, expr, expr... )

void Compiler::scanFuncCall() {
    
    // scan and stack the arguments
    
    int argc=0; // argument count
    
    // get the first argument symbol, or close bracket
    int t = tok->getnext();
    
    if(t != T_CPREN) {
        tok->rewind(); // put the symbol back, there are arguments
        for(;;) {
            cg->pushestack();
            scanExpr(); // scan the argument and compile it
            cg->popestack();
            argc++;
            t = tok->getnext();
            if(t==T_CPREN)
                break; // out of arguments
            else if(t!=T_COMMA)
                error("expected ) or , in argument list");
        }
    }
    
    // and put the function call
    cg->emit(OP_CALL,argc);
}


