static const char rcsid[] = "@(#) : $Id$";

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
#include "stack.h"
#include "session.h"

StackableStack<char *,128> recstack;

/// generate a string containing enough spaces to get from the end
/// of the given string to the position p

void spaces(char *buf,char *s,int p,int buflen){
    int len = strlen(s);
    int i=0;
    if(len>10 && p<len+1)p=len+1;
    while(len<p && i<buflen){
        buf[i++]=' ';
        buf[i]=0;
        len++;
    }
    buf[i]=0;
}

void Language::recpush(char *s){
    try{
        recstack.push(strdup(s));
    } catch(StackException &s){
        throw RecreateException("out of room in recreator stack");
    }
    
}

char *Language::recpop(){
    try{
        return recstack.pop();
    }catch(StackException &s){
        throw RecreateException("stack underflow in recreator stack");
    }
}
char *Language::recpeek(int n){
    try{
        return recstack.peek(n);
    }catch(StackException &s){
        throw RecreateException("stack underflow in recreator stack");
    }
}


void Language::recflush(Growable *g){
    SimpleStack<char *,128> *stk = recstack.getstack();
    
    for(int i=0;i<stk->ct;i++){
        char *s = stk->stack[i];
        g->write(s);
        free(s);
    }
    recstack.clearcurr();
}

char *Language::recreate(instruction *p,Session *ses){
    char *t,*t2;
    
    Growable *g = new Growable(1024,1024,1);
    recreateQuickIf = false;
    
    
    
    // for each instruction, go round the loop...
    
    while(INSTOP(*p)!=OP_END){
        switch(INSTOP(*p)){
        case OP_DUMMY:
            throw RecreateException("dummy found on recreator stack - someone's not done enough checking!");
        case OP_LABEL:
            tmpgrow->clear();
            tmpgrow->write(consts->getStr(INSTDATA(*p)));
            tmpgrow->write(":");
            recpush((char *)tmpgrow->get(0,1));
            {
                //temporarily turn off indenting!
                int old = recreateIndent;
                recreateIndent = 0;
                recreateStmtEnd(g,p+1); // no indents!
                recreateIndent = old;
            }
            break;
        case OP_GOTOMARKER:
            tmpgrow->clear();
            tmpgrow->write("goto ");
            tmpgrow->write(consts->getStr(INSTDATA(*p)));
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            break;
        case OP_GOTOFW:
        case OP_GOTOBK:
            break;
        case OP_PROPREF:
            recreatePropRef(INSTDATA(*p));
            break;
        case OP_IMMED:{
            char buf[128];
            sprintf(buf,"%d",INSTDATA(*p));
            recpush(buf);
            break;
        }
        case OP_LITIDENT:{
            char buf[128];
            sprintf(buf,"`%s",consts->getStr(INSTDATA(*p)));
            recpush(buf);
            break;
        }
        case OP_LIT:
            recreateLiteral(INSTDATA(*p),p+1,ses);
            break;
        case OP_VARREFLOC:
            recpush(getLocalName(INSTDATA(*p)));
            break;
        case OP_VARREFPRM:
            recpush(getParamName(INSTDATA(*p)));
            break;
        case OP_VARREFGLB:
    	  recpush(getGlobalName(INSTDATA(*p)));
	  break;	    

        case OP_VARREFSES:
            recpush(ses->getSesVarName(INSTDATA(*p)));
            break;
        case OP_TRUE:
            recpush("true");
            break;
        case OP_FALSE:
            recpush("false");
            break;
        case OP_RETURN:
            tmpgrow->clear();
            if(INSTDATA(*p)){
                tmpgrow->write("return ");
                t=recpop();
                tmpgrow->write(t);
                free(t);
            }else{
                tmpgrow->write("return");
            }
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            break;
        case OP_NEGATE:
            recreateUnOp("-");
            break;
        case OP_NOT:
            recreateUnOp("!");
            break;
        case OP_BITNOT:
            recreateUnOp("@");
            break;
        case OP_IF:
            tmpgrow->clear();
            tmpgrow->write("if ");
            t=recpop();
            tmpgrow->write(t);
            free(t);
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            recreateIndent++;
            break;
        case OP_QUICKIF:
            tmpgrow->clear();
            tmpgrow->write("if ");
            t=recpop();
            tmpgrow->write(t);
            free(t);
            tmpgrow->write(" : ");
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateQuickIf = true;
            break;
            
        case OP_ELSEIF:
            recreateIndent--;
            tmpgrow->clear();
            tmpgrow->write("elseif ");
            t=recpop();
            tmpgrow->write(t);
            free(t);
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            recreateIndent++;
            break;
        case OP_ELSE:
            recreateIndent--;
            recpush("else");
            recreateStmtEnd(g,p+1);
            recreateIndent++;
            break;
        case OP_ENDIF:
            recreateIndent--;
            recpush("endif");
            recreateStmtEnd(g,p+1);
            break;
        case OP_WHILE:
            tmpgrow->clear();
            tmpgrow->write("while ");
            t=recpop();
            tmpgrow->write(t);
            free(t);
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            recreateIndent++;
            break;
        case OP_ENDWHILE:
            recreateIndent--;
            recpush("endwhile");
            recreateStmtEnd(g,p+1);
            break;
        case OP_FOR:
            t2=recpop(); // iter
            t=recpop(); // var
            tmpgrow->clear();
            tmpgrow->write("for ");
            tmpgrow->write(t);
            tmpgrow->write(" in ");
            tmpgrow->write(t2);
            free(t);free(t2);
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            recreateIndent++;
            break;
            
        case OP_NEXT:
            recreateIndent--;
            recpush("endfor");
            recreateStmtEnd(g,p+1);
            break;
        case OP_BREAK:
            recpush("break");
            recreateStmtEnd(g,p+1);
            break;
        case OP_CONTINUE:
            recpush("continue");
            recreateStmtEnd(g,p+1);
            break;
        case OP_REPEAT:
            recpush("repeat");
            recreateStmtEnd(g,p+1);
            recreateIndent++;
            break;
        case OP_UNTIL:
            recreateIndent--;
            tmpgrow->clear();
            tmpgrow->write("until ");
            t=recpop();
            tmpgrow->write(t);
            free(t);
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            break;
        case OP_GET:
            // leave the variable name on the stack
            break;
            
            // these instructions are actually statements, which 
            // write stuff out
            
        case OP_SET:
            recreateAssign();
            //            recflush(g);
            break;
        case OP_CALL:
            recreateFuncCall(INSTDATA(*p));
            break;
        case OP_PAREN:
            tmpgrow->clear();
            tmpgrow->write("(");
            t=recpop();
            tmpgrow->write(t);
            free(t);
            tmpgrow->write(")");
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            break;
        case OP_SQB:
            tmpgrow->clear();
            t2 = recpop(); // the index expression
            t=recpop(); // the array expression
            tmpgrow->write(t);
            tmpgrow->write("[");
            tmpgrow->write(t2);
            tmpgrow->write("]");
            tmpgrow->terminate();
            free(t);
            free(t2);
            recpush((char *)tmpgrow->get(0,1));
            break;
        case OP_THIS:
            recpush("this");
            break;
        case OP_ENDESTMT:
        case OP_ENDESTMT2:
            recreateStmtEnd(g,p+1); // weird case
            break;
        case OP_END: // actually the end case
        case OP_JMPELSEIF:
        case OP_STARTESTMT:
        case OP_ENDFOR:
        case OP_SRCLINE:
        case OP_SRCFILE:
            break;
        default:
            {
                BinaryOperator *op = BinaryOperator::getbinopbyop(INSTOP(*p));
                if(op)
                    recreateBinop(op->symbol);
                else
                    throw RecreateException(NULL).set("code unknown %d",INSTOP(*p));
            }
            break;
        case OP_SPECIAL:
            switch(INSTDATA(*p)){
            case 0:
                recpush("dumplocs");
                break;
            case 1:
                recpush("breakpoint");
            }
            recreateStmtEnd(g,p+1);
            break;
        case OP_COMMENT_SOL:
            tmpgrow->clear();
            recreateComment(tmpgrow,INSTDATA(*p));
            tmpgrow->terminate();
            recpush((char *)tmpgrow->get(0,1));
            recreateStmtEnd(g,p+1);
            break;
        case OP_COMMENT_EOL:
        case OP_COMMENT_EOFD:
            break;
        case OP_BLANKLINE:
            recpush("");
            recreateStmtEnd(g,p+1);
            break;
        }
        p++;
    }
    
    //    recflush(g);
    g->terminate();
    char *out = strdup((char *)g->get(0,1));
    delete g;
    return out;
}

void Language::recreateComment(Growable *g,int d) {
    ConstDesc *e = consts->get(d);
    char *mem = (char *)e->get();
    int pos = *(short*)mem;
    mem+=sizeof(short);
    
    char buf[256];
    if(g->getOffset())
        spaces(buf,(char *)g->get(0,1),pos,256);
    else
        spaces(buf,"",pos,256);
    g->write(buf);
    g->write("#");
    g->write(mem);
}

void Language::recreateStmtEnd(Growable *g,instruction *next){
    char *t;
    tmpgrow->clear();
    
    if(recreateQuickIf){
        recreateQuickIf = false;
        t = recpop(); // the statement itself
        char *p = recpop(); // the if, condition, and colon
        tmpgrow->write(indents());
        tmpgrow->write(p);
        tmpgrow->write(t);
        free(p);free(t);
    } else {
        t=recpop();
        tmpgrow->write(indents());
        tmpgrow->write(t);
        free(t);
    }
    
    if(INSTOP(*next)==OP_COMMENT_EOL){
        recreateComment(tmpgrow,INSTDATA(*next));
    }
    tmpgrow->write("\n");
    tmpgrow->terminate();
    recpush((char *)tmpgrow->get(0,1));
    recflush(g);
}    

void Language::recreatePropRef(int n){
    const char *name = consts->getStr(n);
    char *lvalue = recpop();
    char *t = (char *)malloc(strlen(name)+strlen(lvalue)+2);
    sprintf(t,"%s.%s",lvalue,name);
    recpush(t);
    free(t);
    free(lvalue);
}

void Language::recreateFuncCall(int argc){
    char *s;
    // we're going to write a function call now
    tmpgrow->clear();
    
    // get the function variable from somewhere under the stack
    s=recpeek(argc);
    tmpgrow->write(s); // output the func name
    free(s);
    
    tmpgrow->write("(");
    
    int i;
    char *argv[128];
    
    // pop off the arguments, we have to do this in reverse order
    for(i=0;i<argc;i++)
        argv[i]=recpop();
    recpop(); // extra pop to get rid of the variable
    
    // output the args, and output
    for(int i=0;i<argc;i++){
        s=argv[(argc-1)-i];
        tmpgrow->write(s);
        free(s);
        if(i!=argc-1)
            tmpgrow->write(",");
    }
    tmpgrow->write(")");
    tmpgrow->terminate();
    recpush((char *)tmpgrow->get(0,1));
}

void Language::recreateAssign(){
    char *rvalue = recpop();
    char *lvalue = recpop();
    char *t = (char *)malloc(strlen(rvalue) + strlen(lvalue) + 5 + recreateIndent);
    
    sprintf(t,"%s = %s",lvalue,rvalue);
    recpush(t);
    free(t);
    free(rvalue);
    free(lvalue);
}

static char varbuf[128];
char *Language::getLocalName(int n){
    if(currentRecreateLDT){
        // takes advantage of the fact that the local numbers
        // follow the parameter numbers
        short *paramptr = (short*)(currentRecreateLDT+1);
        return consts->getStr(paramptr[n]);
    }
    else {
        sprintf(varbuf,"(local$%d)",n);
        return varbuf;
    }
}
char *Language::getParamName(int n){
    if(currentRecreateLDT){
        short *paramptr = (short*)(currentRecreateLDT+1);
        return consts->getStr(paramptr[n]);
    }
    else {
        sprintf(varbuf,"(param$%d)",n);
        return varbuf;
    }
}
char *Language::getGlobalName(int n){
    int namedesc = globs->getName(n);
    return consts->getStr(namedesc);
}

void Language::recreateBinop(const char *op){
    char *b = recpop();
    char *a = recpop();
    
    char *t= (char *)malloc(strlen(a)+strlen(b)+10);
    sprintf(t,"%s%s%s",a,op,b);
    recpush(t);
    free(a);
    free(b);
    free(t);
}


void Language::recreateLiteral(int n,instruction *next,Session *ses){
    // get the descriptor
    ConstDesc *e = consts->get(n);
    if(!e)
        throw RecreateException(NULL).set("internal - bad constant %d",n);
    
    char *t;
    char *ptr = (char *)e->get();
    
    switch(e->getType()){
    case CT_STRING:
        n = strlen(ptr);
        t = (char*)malloc(n+3);
        t[0] = '"';
        strcpy(t+1,ptr);
        t[n+1]='"';
        t[n+2]=0;
        break;
    case CT_INT:
        t=(char *)malloc(128);
        sprintf(t,"%d",*(int *)ptr);
        break;
    case CT_FLOAT:
        t=(char *)malloc(128);
        sprintf(t,"%f",*(float *)ptr);
        break;
    case CT_FUNC:
        {
            // a pointer to a size followed by an instruction
            int *sizePtr = *(int **)ptr;
            instruction *instPtr = (instruction *)sizePtr+1;
            t=recreateFunction(instPtr,e->size,ses);
            break;
        }
    case CT_LDT:
        t=strdup("LDT");
        break;
    case CT_COMMENT:
    default:
        t=strdup("");
        break;
    }
    
    recpush(t);
    free(t);
}

char *Language::indents(){
    static char f[1024];
    int i = recreateIndent*4;
    if(i == 1024)
        throw RecreateException("indent level too high");
    memset(f,' ',i);
    f[i]=0;
    
    return f;
}

void Language::setRecreateLDT(constid id){
    currentRecreateLDT = (LDTHeader *)consts->get(id)->get();
}


char *Language::recreateFunction(instruction *ptr,int size,Session *ses){
    // save this; it's going to get changed for the function we're about to parse
    
    LDTHeader *oldldt = currentRecreateLDT;
    
    // first, the first instruction should be OP_LOCALS
    
    if(INSTOP(*ptr)==OP_LOCALS){
        // extract the LDT so we can recreate globals etc.
        int ldtoffset = INSTDATA(*ptr++);
        setRecreateLDT(ldtoffset);
    } else
        currentRecreateLDT = NULL;
    
    // get the code block. Careful with the recursion!
    recreateIndent++;
    recstack.pushstack();
    char *code = recreate(ptr,ses); // we have removed the OP_LOCALS
    recreateIndent--;
    recstack.popstack();
    
    // wrap the code block in "function(...) ... end"
    
    // deal with any End Of Function Description comment first.
    // a bit filthy - if there's an EOFD, it will
    // follow the OP_LOCALS.
    
    if(INSTOP(*ptr)==OP_COMMENT_EOFD)
        eofdcomment=ptr;
    
    Growable *g = new Growable(1024,1024,1);
    if(currentRecreateLDT->flags & LDTF_RETURNS)
        g->write("function(");
    else
        g->write("procedure(");
    
    int i,num;
    
    if(currentRecreateLDT){
        short *paramptr = (short*)(currentRecreateLDT+1);
        short *localptr = paramptr+currentRecreateLDT->numparams;
        
        // output parameter list 
        int num = currentRecreateLDT->numparams;
        for(i=0;i<num;i++){
            const char *name = consts->getStr(paramptr[i]);
            g->write(name);
            if(i!=num-1)
                g->write(", ");
        }
        
        g->write(")");
    } else {
        g->write(")");
    }
    
    if(eofdcomment){
        recreateComment(g,INSTDATA(*eofdcomment));
        eofdcomment=NULL;
    }
    g->write("\n");   
    
    g->write(code);
    g->write(indents());
    g->write("end");
    free(code);
    
    char *out = strdup((char *)g->get(0,1));
    delete g;
    
    // restore the old LDT
    currentRecreateLDT = oldldt;
    return out;
}

void Language::recreateUnOp(const char *op){
    char *t;
    tmpgrow->clear();
    tmpgrow->write(op);
    t=recpop();
    tmpgrow->write(t);
    free(t);
    tmpgrow->terminate();
    recpush((char *)tmpgrow->get(0,1));
    
}
