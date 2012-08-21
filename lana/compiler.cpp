/**
 * @file
 * The Lana compiler's core. Each Session has a compiler, but there's only one VM,
 * only one global namespace, only one constant area.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "language.h"
#include "cg.h"
#include "consts.h"
#include "tokeniser.h"
#include "tokens.h"
#include "opcodes.h"
#include "growable.h"
#include "vm.h"
#include "label.h"
#include "session.h"
#include "compiler.h"
#include "ser.h"

Compiler::Compiler(Session *s,Language *l){
    ses = s;
    lana = l;
    lastLine = NULL;
    lineNumber = 1;
    fileName = NULL;
    outputFileName = false;
    cg = new CodeGen(lana,ses);
    tok = lana->tok;
    eventMgr = new EventMgr;
}

Compiler::~Compiler(){
    if(lastLine)
        free(lastLine);
    if(fileName)
        free(fileName);
    delete cg;
    delete eventMgr;
}

void Compiler::error(const char *s,...)
{
    char buf[1024];
    
    va_list args;
    va_start(args,s);
    
    const char *fn = fileName?fileName:"<<unknown>>";
    sprintf(buf,"%s line %d : ",fn,lineNumber);
    vsprintf(buf+strlen(buf),s,args);
    
    throw ParseException(buf);
}

void Compiler::notifyNewFile(const char *name){
    if(fileName)
        free(fileName);
    if(name)
        fileName = strdup(name);
    else
        fileName = NULL;
    lineNumber = 1;
    outputFileName = true;
}

void Compiler::feed(const char *buf){
    if(lana->debugFlags & LDEBUG_SHOW)
        lana->dprintf("feed: %s",buf);
    
    // output src file and line if in debug mode
    generateDebugInstructions(false);
    
    if(lastLine)
        free(lastLine);
    lastLine = strdup(buf);
    try{
        tok->reset(buf);
        
        cg->saveSnapshot(); // record where we are in case we need to rewind because of a parse error
        scanStmt();
        
        // terminate and execute if we're not compiling, otherwise we just add to the function.
        if(!cg->isCompiling()) {
            // terminate the sequence
            cg->emit(OP_END);
            
            // get the codegen area
            Growable *g = cg->getCode();
            int size = g->getOffset()/sizeof(instruction);
            instruction *p = (instruction *)g->get(0,sizeof(instruction));
            
            if(lana->debugFlags & LDEBUG_RECREATE){
                char *t = lana->recreate(p,ses);
                printf("RECREATED:\n***\n%s\n***\n",t);
                free(t);
            }
            
            // notify any listener
            eventMgr->notify(COMMAND_PARSED, EventData(p));
            
            if(!(lana->opFlags & LOP_NORUN)){ // if we want to run the code...
                if(lana->debugFlags & LDEBUG_TRACE)
                    printf("EXECUTE and clear\n");
                lana->vm->interpret(p,ses);
                
                Value *v = lana->vm->popvalnoexception();
                if(v){
                    printf("%s\n",v->getStr());
                }
            }
            
            if(lana->debugFlags & LDEBUG_DUMP){
                printf("Dump of interpreter block:\n");
                lana->dumpCode((instruction *)cg->current->code->get(0,sizeof(instruction)),
                               cg->current->code->getOffset()/sizeof(instruction),ses);
            }
            
            cg->clear(); // all done with the code, clear it ready for more input
        }
    } catch(Exception &ex){
        // rewind the code generator to before the broken bit
        cg->restoreSnapshot();
//        printf("Error - recovered : %s\n",ex.what());
        // rethrow
        throw;
    }
    lineNumber++;
}

void Compiler::feedFile(const char *fileName){
    FILE *a = fopen(fileName,"r");
    
    if(!a){
        throw Exception(NULL).set("cannot open file %s",fileName);
    }
    
    try {
        char buf[1024];
        notifyNewFile(fileName);
        while(fgets(buf,1024,a)!=NULL) {
            feed(buf);
        }
        fclose(a);
    } catch(...) {
        fclose(a);
        notifyNewFile(NULL);
        throw;
    }
    notifyNewFile(NULL);
}

bool Compiler::awaitingInput(){
    return cg->isCompiling();
}

void Compiler::generateDebugInstructions(bool alwaysFileName) {
    if(lana->debugFlags & LDEBUG_SRCDATA){
        if(outputFileName || alwaysFileName){
            outputFileName = false;
            int desc = lana->consts->findOrCreateString(fileName?fileName:"??");
            cg->emit(OP_SRCFILE,desc);
        }
        cg->emit(OP_SRCLINE,lineNumber);
    }
}



int Compiler::findLocal(int namedesc) {
    for(int i=0;i<cg->current->ldth.numlocals;i++)
        if(cg->current->locals[i] == namedesc){
            return i;
        }
    return -1;
}

int Compiler::findParam(int namedesc) {
    for(int i=0;i<cg->current->ldth.numparams;i++)
        if(cg->current->params[i] == namedesc){
            return i;
        }
    return -1;
}

int Compiler::createLocal(int namedesc) {
    int i = cg->current->ldth.numlocals;
    if(cg->current->ldth.numlocals==MAXLOCALS)
        error("too many local variables");
    cg->current->locals[cg->current->ldth.numlocals++] = namedesc;
    return i;
}



void Compiler::addListener(event e, Listener *p){
    eventMgr->add(e,p);
}









/*
 * scanners
 */


/// scan statement. 
/// Scan position: start of line

void Compiler::scanStmt() {
    char buf[256]; // temporary buffer
    int t;
    
    switch(tok->getnext()){
    case T_IDENT: // it's an expression
    case T_INT:
    case T_FLOAT:
    case T_SUB:
    case T_BITNOT:
    case T_PLING:
    case T_STRING:
    case T_BACKTICK:
    case T_OPREN:
        // deal with debugging words!
        if(!strcmp(tok->getstring(),"dumplocs")){
            cg->emit(OP_SPECIAL,0);break;
        }
        if(!strcmp(tok->getstring(),"breakpoint")){
            cg->emit(OP_SPECIAL,1);break;
        }
        
        tok->rewind(); // put the token back
        // scan the expression, might be a label
        if(!scanExpr(true))
            // clear all statements if not a func or other oddity,
            // or just a dummy for recreation purposes if in immediate mode.
            cg->emit(cg->isCompiling()?OP_ENDESTMT:OP_ENDESTMT2);
        break;
    case T_LOAD:
        {
            if(cg->isCompiling())
                error("can only run 'load' in interactive mode");
            Session *s;
            if(tok->getnext()!=T_STRING)
                error("expected a string after 'load'");
            try {
                s = new Session(ses->api);
                s->feedFile(tok->getstring());
            } catch(Exception &e){
                delete s;
                throw e;
            }
            delete s;
        }
        break;
    case T_SAVE:
        {
            if(cg->isCompiling())
                error("can only run 'save' in interactive mode");
            if(tok->getnext()!=T_STRING)
                error("expected a string after 'save'");
            const char *fname = tok->getstring();
            
            FILE *a;
            if(!strlen(fname))
                a = stdout;
            else
                a = fopen(fname,"w");
            
            if(!a)
                error("cannot open file '%s'",fname);
            
            Serialiser *ser = new Serialiser(ses);
            ser->write(a);
            if(strlen(fname))
                fclose(a);
            delete ser;
        }
        break;
    case T_SAVEVAR:
        {
            if(cg->isCompiling())
                error("can only run 'savevar' in interactive mode");
            if(tok->getnext()!=T_IDENT)
                error("expected a variable name after 'savevar'");
            const char *vname = tok->getstring();
            int vdesc = lana->consts->findOrCreateString(vname);
            if(tok->getnext()!=T_STRING)
                error("expected a string after 'savevar'");
            const char *fname = tok->getstring();
            
            // try to get the value
            Value *v;
            int id;
            id = lana->globs->find(vdesc);
            if(id>=0) {
                v = lana->globs->get(id); // it's a global
            } else {
                id = ses->findSesVar(vdesc);
                if(id<0)
                    error("variable not found: %s",lana->consts->getStr(vdesc));
                v = ses->getSesVar(id);
            }
            
            FILE *a;
            if(!strlen(fname))
                a = stdout;
            else
                a = fopen(fname,"w");
            
            if(!a)
                error("cannot open file '%s'",fname);
            
            Serialiser *ser = new Serialiser(ses);
            ser->serialiseValue(a,v,lana->consts->getStr(vdesc));
            if(strlen(fname))
                fclose(a);
            delete ser;
        }
        break;
    case T_FOR:
        scanFor();
        break;
    case T_ENDFOR:
        scanEndFor();
        break;
    case T_THIS:
        tok->rewind(); // put the token back
        if(!scanExpr(true))
            // clear all statements if not a func or other oddity,
            // or just a dummy for recreation purposes if in immediate mode.
            cg->emit(cg->isCompiling()?OP_ENDESTMT:OP_ENDESTMT2);
        break;
    case T_GOTO:
        if(!cg->isCompiling())
            error("must be compiling a function/procedure to use '%s'",tok->getstring());
        scanGoto();
        break;
    case T_ENDFUNC:
        scanEndFunc();
        break;
    case T_END:
        if(!(lana->opFlags & LOP_STRIPCOMMENTS))
            cg->emit(OP_BLANKLINE); // yes, these are wasteful .. very slightly
        break;
    case T_IF:
        // first we push a special value onto the compiler stack
        // to mark the start of this if..elseif..elseif..endif
        cg->current->cpush(-9999);
        // we scan the expression 
        if(scanExpr())
            error("cannot use a function/procedure expression in if");
        // stack and output an incomplete if - but this might be a normal if, or a quick if.
        cg->current->cpushhere();
        
        // now for some cleverness. Is the next token a colon?
        if(tok->getnext() == T_COLON){
            cg->emit(OP_QUICKIF,-100);
            // if so, parse the next statement recursively
            scanStmt();
            // note that we don't need to output a quick endif, since the recreator
            // doesn't need it!
            instruction *ptr = cg->current->cpoplocandcheck(OP_QUICKIF,OP_QUICKIF); // MUST be an OP_IF, no ELSE.
            if(!ptr)
                error("not a simple statement in quick-if");
            // write the IF, ELSE or ELSEIF again with the correct distance
            *ptr = INST(INSTOP(*ptr),cg->current->getdiff(ptr));
            // now pop off!
            int n;
            do{
                n = cg->current->cpop();
            }while(n!=-9999);
        } else {
            // not - put it back!
            tok->rewind();
            cg->emit(OP_IF,-100);
        }
        break;
    case T_ENDIF:
        {
            // get the corresponding OP_IF, OP_ELSEIF or OP_ELSE
            cg->emit(OP_ENDIF);
            instruction *ptr = cg->current->cpoplocandcheck(OP_IF,OP_ELSE);
            if(!ptr)
                error("mismatched endif");
            // write the IF, ELSE or ELSEIF again with the correct distance
            *ptr = INST(INSTOP(*ptr),cg->current->getdiff(ptr));
            
            // now pop and fixup OP_JMPELSEIFs until we get the special -9999 which marked the start
            
            for(;;){
                int n = cg->current->cpop();
                if(n==-9999)break; // we're done!
                // we're not done - get the code pointer
                instruction *ptr = cg->current->getPtr(n*sizeof(instruction));
                // make sure it's a OP_JMPELSEIF!
                if(INSTOP(*ptr)!=OP_JMPELSEIF)
                    error("badly formed conditional statement");
                // change it so that it jumps to the current location
                *ptr = INST(INSTOP(*ptr),cg->current->getdiff(ptr));
            }
        }
        break;
    case T_ELSEIF:
        {
            // pop the instruction off the stack, an OP_IF or OP_ELSEIF
            instruction *ptr = cg->current->cpoplocandcheck(OP_IF,OP_ELSEIF);
            // first we need to terminate the previous condition, so
            // push the location and output a OP_JMPELSEIF ready to fill in.
            // This will get left on the stack!
            cg->current->cpushhere();
            cg->emit(OP_JMPELSEIF,-100);
            // now make the IF or ELSEIF we popped jump to this point
            *ptr = INST(INSTOP(*ptr),cg->current->getdiff(ptr));
            
            // now scan the expression
            if(scanExpr())
                error("cannot use a function/procedure expression in if");
            // push the location, and..
            // output an OP_ELSEIF with a dummy jump
            cg->current->cpushhere();
            cg->emit(OP_ELSEIF,-100);
        }
        break;
    case T_ELSE:
        {
            // write the OP_ELSE which will become a jump forward,
            // but first recording the location
            int elseloc = cg->current->getloc();
            cg->emit(OP_ELSE,-100);
            // now we need to make the IF jump to here
            // get the corresponding OP_IF or OP_ELSEIF
            instruction *ptr = cg->current->cpoplocandcheck(OP_IF,OP_ELSEIF);
            if(!ptr)
                error("mismatched else");
            // write the IF again with the correct jump distance
            int diff = cg->current->getdiff(ptr);
            *ptr = INST(INSTOP(*ptr),diff);
            // now push the location of the OP_ELSE, which
            // will get processed by the OP_ENDIF
            cg->current->cpush(elseloc);
        }
        break;
        // pop the location
    case T_RETURN:
        if(!cg->isCompiling())
            error("must be compiling a function/procedure to use '%s'",tok->getstring());
        if(tok->getnext() == T_END){ // end of line?
            tok->rewind();
            // no return value
            if(cg->current->ldth.flags & LDTF_RETURNS)
                error("functions must return a value");
            cg->emit(OP_RETURN,0);
        } else {
            tok->rewind();
            if(!(cg->current->ldth.flags & LDTF_RETURNS))
                error("procedures cannot return a value");
            if(scanExpr())
                error("cannot directly return a function");
            cg->emit(OP_RETURN,1);
        }
        break;
    case T_WHILE:
        if(!cg->isCompiling())
            error("must be compiling a function/procedure to use '%s'",tok->getstring());
        // push the current location onto the stack - this is where ENDWHILE will
        // jump to
        cg->current->cpushhere();
        // we also create and push the loop data here, so that we can use break and
        // continue!
        cg->current->newloop();
        // scan and output the expression
        if(scanExpr())
            error("cannot use a function/procedure expression in `while`");
        // push the WHILE onto the stack so we can write the terminating jump into it
        cg->current->cpushhere();
        // output OP_WHILE with a dummy
        cg->emit(OP_WHILE,-100);
        break;
    case T_ENDWHILE:
        {
            // pop the location of the WHILE from the stack
            instruction *whileptr = cg->current->cpoplocandcheck(OP_WHILE,OP_WHILE);
            if(!whileptr)
                error("mismatched endwhile");
            // pop the location for the backward jump
            instruction *jumpdest = cg->current->cpoplocation();
            // output the endwhile, which will do the backward jump
            cg->emit(OP_ENDWHILE,cg->current->getdiff(jumpdest));
            // now patch the while instruction with the forward jump to use if the
            // condition is false
            *whileptr = INST(OP_WHILE,cg->current->getdiff(whileptr));
            /// and end the loop, setting the break label and popping the loop stack
            cg->current->endloop();
            break;
        }
        
    case T_REPEAT:
        if(!cg->isCompiling())
            error("must be compiling a function/procedure to use '%s'",tok->getstring());
        // output OP_REPEAT, pushing its location. We don't
        // jump to here, though - we jump to the following opcode.
        // This is done just so we can check that the until matches
        // a repeat. See T_UNTIL.
        cg->current->cpushhere();
        cg->current->newloop(); // push and initialise a new loop stack entry (see T_WHILE above)
        cg->emit(OP_REPEAT,0);
        break;
    case T_UNTIL:
        {
            // scan and output the expression
            if(scanExpr())
                error("cannot use a function/procedure expression in `until`");
            // pop the location of the OP_REPEAT from the stack
            instruction *ptr = cg->current->cpoplocandcheck(OP_REPEAT,OP_REPEAT);
            if(!ptr)
                error("mismatched `until'");
            // increment this, because we want to save cycles by
            // jumping past the OP_REPEAT (which is a kind of noop)
            ptr++;
            // and output the OP_UNTIL jump
            cg->emit(OP_UNTIL,cg->current->getdiff(ptr));
            cg->current->endloop(); // end the current loop stack entry (see T_ENDWHILE above)
            break;
        }
    case T_BREAK:
        {
            // we want to break out of the topmost loop on the loop stack
            
            // get address we're about to write to
            instruction *op = cg->current->getlocptr();
            // output the break which will be patched later
            cg->emit(OP_BREAK,-100);
            // and this jump as a jump to be patched when the break label is resolved
            LoopData *d = cg->current->loopstack.peekptr();
            if(!d)
                throw ParseException("break with no loop");
            d->breaklabel.jumpFrom(op);
        }
        break;
    case T_CONTINUE:
        {
            // we want to terminate the current iteration of the topmost loop on the loop stack
            // and immediately start the loop code again
            // get address we're about to write to
            instruction *op = cg->current->getlocptr();
            // output the break which will be patched later
            cg->emit(OP_CONTINUE,-100);
            // and this jump as a jump to be patched when the break label is resolved
            LoopData *d = cg->current->loopstack.peekptr();
            if(!d)
                throw ParseException("continue with no loop");
            d->continuelabel.jumpFrom(op);            
        }
        break;
    case T_COMMENT:
        scanComment(true);
        break;
    default:
        error("unexpected token '%s'",tok->getstring());
    }
    
    // see if there's a comment at the end
    scanPossibleComment();
    
    if(tok->getnext()!=T_END)
        error("trailing garbage at end of line");
}

void Compiler::scanComment(bool startofline){
    // go back and get the position of the comment marker
    int pos = tok->getpos()-1;
    
    const char *com = tok->restofline();
    if(!(lana->opFlags & LOP_STRIPCOMMENTS))
        cg->emit(startofline?OP_COMMENT_SOL:OP_COMMENT_EOL,lana->consts->createComment(com,pos));
}

void Compiler::scanPossibleComment(){
    if(tok->getnext() == T_COMMENT){
        scanComment(false);
    } else
        tok->rewind();
}

/// scan a function header.
/// scan position: IDENT =function *** ( a,b,c ..) {returns}

void Compiler::scanFuncHeader(bool isfunc) {
    int t;
    
    if(tok->getnext() != T_OPREN)
        error("expecting (");
    
    // parse args
    for(;;){
        t = tok->getnext();
        if(t == T_CPREN)break;
        else if(t == T_IDENT) {
            // copy parameter into temp LDT
            int c = lana->consts->findOrCreateString(tok->getstring());
            if(cg->current->ldth.numparams==MAXLOCALS)
                error("WAY too many parameters (good grief!)");
            cg->current->params[cg->current->ldth.numparams++] = c;
            
            t = tok->getnext();
            if(t == T_CPREN)break;
            else if(t!=T_COMMA)
                error("expected , or )");
        }
    }
    
    // set the appropriate flag if it's a true function
    if(isfunc)
        cg->current->ldth.flags |= LDTF_RETURNS;
    
    // parse any weird comments at the end of the func def line
    
    t = tok->getnext();
    if(t==T_COMMENT){
        // go back and get the position of the comment marker
        int pos = tok->getpos()-1;
        
        const char *com = tok->restofline();
        if(!(lana->opFlags & LOP_STRIPCOMMENTS))
            cg->emit(OP_COMMENT_EOFD,lana->consts->createComment(com,pos));
        
    } else tok->rewind();
    
    // and now we carry on parsing the function until we get to END
}

/// scan the end of a function
/// scan position: END ***

void Compiler::scanEndFunc() {
    if(!cg->isCompiling()){
        error("must be compiling a function/procedure for `end' to make sense");
    }
    
    cg->emit(OP_END); //terminate the function
    
    /********************** deal with creating an LDT *************/
    
    int ldtsize = sizeof(LDTHeader) + sizeof(short)*(cg->current->ldth.numparams+
                                                     cg->current->ldth.numlocals);
    
    // create space for the LDT in the constant area and a descriptor
    constid ldtdesc = lana->consts->create(CT_LDT,NULL,0,ldtsize);
    
    // get the LDT header, parameter, and locals pointers
    
    ConstDesc *ent = lana->consts->get(ldtdesc);
    LDTHeader *ptr = (LDTHeader *)ent->get();
    short *paramptr = (short *)(ptr+1);
    short *localptr = paramptr+cg->current->ldth.numparams;
    
    // and copy everything over
    
    *ptr = cg->current->ldth;
    memcpy(paramptr,cg->current->params,
           sizeof(short)*cg->current->ldth.numparams);
    memcpy(localptr,cg->current->locals,sizeof(short)*
           cg->current->ldth.numlocals);
    
    // pop the offset off the compiler stack
    
    instruction *opptr = cg->current->cpoplocation();
    
    // compiler stack must now be empty - we must have balanced and
    // closed out all control structures
    if(!cg->current->cempty()) {
        error("mismatch at `end': missing endif/until/endwhile?");
    }
    
    // now write the OP_LOCALS with the ldt offset -- NOT descriptor
    
    *opptr = INST(OP_LOCALS,ldtdesc);
    
    /********************** deal with creating the function ******/
    
    // check it does or doesn't return correctly
    cg->checkReturns((cg->current->ldth.flags & LDTF_RETURNS)?true:false);
    
    
    // turn the function into a constant and note the descriptor. This is a little messy;
    // the function is stored as memory, and a constant is made which just contains a pointer
    // to the memory. That's so the bytecode is still valid (bytecodes must be 32-bit) for
    // the OP_LIT. The bytecode can't contain the ptr, it has to contain an integer.
    //
    // The only reason we're doing this is because a function cannot be a literal otherwise.
    
    bool returns = (cg->current->ldth.flags & LDTF_RETURNS) ? true : false;
    int numparams = cg->current->ldth.numparams;
    int flags =   (returns ? (1<<15) : 0) & numparams;

    const char *funcptr = cg->writeContextToMemory();
                                             
    // write the function pointer out as a constant.
    int funcdesc = lana->consts->create(CT_FUNC,&funcptr,flags,sizeof(funcptr));
    
    // pop the compilation context
    cg->popContext();
    // remember where we allocated space to write the function descriptor
    // literal opcode
    instruction *litlocation = cg->current->cpoplocation();
    
    // and write the function's descriptor into that slot
    *litlocation = INST(OP_LIT,funcdesc);
    
    // now write the assignment which must be there
    cg->emit(OP_SET);
    // and end the stmt-expr, started earlier. This time we really
    // do clean up, even in immediate mode
    cg->emit(OP_ENDESTMT);
    
    // and finally clear the expression parser's stack - we sort of
    // hijacked the shunting yard algorithm, and there's still a OP_SET
    // lurking there in the stack.
    
    cg->current->estack.clearcurr();
    
}

/// The sequence of instructions here is
/// \code
///                  (varref for loop index)
///                  (expression producing iterator)
///                  OP_FOR
/// continuelabel,
/// label1:          ...
///                  ...
///                  OP_NEXT(label1)
/// breaklabel:
///                  OP_ENDFOR
/// \endcode

void Compiler::scanFor(){
    if(!cg->isCompiling())
        error("can only use 'for' inside a function/procedure");
    
    if(tok->getnext()!=T_IDENT)
        error("expected an identifier after 'for'");
    
    // output the index variable
    emitVariableRef(tok->getstring());
    
    if(tok->getnext()!=T_IN)
        error("expected 'in' after 'for' variable");
    
    // output the expression
    if(scanExpr())
        error("cannot use a function expression in 'for'");
    
    cg->current->newloop(); // start loop and set continuelabel, 
    // get that pointer because we're going to muck about with it
    LoopData *loop = cg->current->loopstack.peekptr();
    
    // push the FOR *here* although we jump past it; we still need
    // to match with it and modify it for the jump forwards
    cg->current->cpushhere(); 
    cg->emit(OP_FOR); // output FOR
    // modify the continue so it's *AFTER* the FOR.
    loop->continuelabel.set(cg->current->getlocptr());
}

/// see scanFor() for the sequence of instructions
void Compiler::scanEndFor(){
    instruction *forptr = cg->current->cpoplocandcheck(OP_FOR,OP_FOR);
    if(!forptr)
        error("mismatched 'next'");
    // output the next, with the backward jump - to AFTER the for
    cg->emit(OP_NEXT,cg->current->getdiff(forptr+1));
    // modify the OP_FOR to jump to after the NEXT if the iterator is empty
    *forptr = INST(OP_FOR,cg->current->getdiff(forptr));
    // end the loop, setting the break label and popping the loop stack
    cg->current->endloop();
    // output OP_ENDFOR to clean up
    cg->emit(OP_ENDFOR);
}


void Compiler::scanGoto(){
    if(tok->getnext()!=T_IDENT)
        error("expected identifier after goto");
    
    Label *lab;
    int d = lana->consts->findOrCreateString(tok->getstring());
    
    // first emit the goto marker, which is just there for the recreator. Ugly,
    // but possibly quicker than the alternative.
    
    cg->emit(OP_GOTOMARKER,d);
    
    // now look for the label in the current compiling context's label map
    std::map<int,Label *>::iterator it = cg->current->labels.find(d);
    
    if(it == cg->current->labels.end()) {
        // label does not exist, so create one
        lab = new Label();
        cg->current->labels[d] = lab;
    } else {
        // use the label we found
        lab = it->second;
    }
    
    // now we have our label, add a goto to it. This will either immediately
    // resolve, or will be resolved when the label is set (i.e. a label statement
    // for it is parsed.)
    
    instruction *op = cg->current->getlocptr(); // get address we're about to write to
    cg->emit(OP_GOTOFW,-100); // write a dummy destination
    lab->jumpFrom(op); // generate a jump in this label from the op we just wrote
}
