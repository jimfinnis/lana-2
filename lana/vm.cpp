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
#include "object.h"
#include "iterobj.h"



void VirtualMachine::clearstacks(){
    // make sure all stack is cleared
    while(!xstack.isempty()){
        Value *v = xstack.popptr();
        v->clr();
    }
    for(int i=0;i<vstacknext;i++)
        vstack[i].clr();
    vstackbase=0;
    vstacknext=0;
    stkbase=0;
    cvb.clear();    
    locals = NULL;
}

void VirtualMachine::clearAndFlush(){
    for(int i=0;i<EXSTACKSIZE;i++){
        xstack.stack[i].clr();
    }
    for(int i=0;i<VSTACKSIZE;i++){
        vstack[i].clr();
    }
    xstack.ct=0;
    
    vstackbase=0;
    vstacknext=0;
    stkbase=0;
    cvb.clear();
    locals = NULL;
}



VirtualMachine::~VirtualMachine(){
    clearAndFlush();
}
    

void VirtualMachine::interpret(instruction *start,Session *s){
    ip = start;
    
    const char *err = NULL;
    
    clearAndFlush();
    try {
        run(s);
    } catch(RuntimeException &r) {
          throw r;
    } catch(Exception &e){
        throw RuntimeException(e.what(),
                               getSourceFile(),
                               getSourceLine());
        clearAndFlush();
    }
}


extern float *neareq_epsilon;

void VirtualMachine::run(Session *ses){
    Value *a,*b,*c;
    Object *o;
    int j;
    int df = lana->debugFlags;
    curSession = ses;
    for(;;){
        instct++;
        if(df & LDEBUG_TRACE){
            const char *s = getSourceFile();
            if(line>=0)
                printf("%2d EXEC %-45s\t%20s:%3d\t%s\n",retstack.ct,lana->dumpInst(ip,ses),s?s:"??",line,
                       stkDump());
            else
                printf("%2d EXEC %-45s\t%20s\t%s\n",retstack.ct,lana->dumpInst(ip,ses),s?s:"??",
                       stkDump());
        }
        instruction op = *ip++;
        switch(INSTOP(op)){
        case OP_RETURN:
            if(INSTDATA(op)) {
                // if we do actually return a value, make sure it's dereferenced!
                Value v = *popval();
                *xstack.pushptr() = v;
            }
        case OP_END:
            if(rpop()){  // exit if out of stack!
                curSession = NULL;
                return;
            }
            break;
        case OP_FOR:
            // we look at the iterator and var but keep
            // them on the stack
            a = xstack.peekptr();
            if(!a)
                error("stack underflow");
            // get the variable - note that if this is an implicit values() call, i.e. what's on the
            // stack is not an iterator object, we must change the value of the reference on the stack, NOT
            // the contents of the referred to variable, into an IterObj.
            b=a->deref(); // dereference into b
            if(b->type != Types::vtIterObj) { // not an iterator object already
                // here, we fudge up an iterator object out
                // of the object if we can - and it's a value iterator.
                IteratorObject *iterator = IteratorObject::create(lana->getAPI(),b,false);
                a->setIterObj(iterator); // and we use that from now on, on the STACK - we do NOT set 'a', the referred to value.
            } else
                a=b; // use the dereffed value
            
            a->d.iterobj->first(); // start iterator
            // are we already done (i.e iterator empty?)
            // if so, skip
            if(a->d.iterobj->isDone())
                ip+=INSTDATA(op)-1;
            else {
                // get the loop var but keep it on the stack
                b = xstack.peekptr(1); // the loop index ref
                // leave this as a ref, do not deref
                
                // now store the current (first) item in the varref
                c = a->d.iterobj->current();
                b->store(c);
            }
            break;
        case OP_NEXT:
            // get iterator and index reference again
            a = xstack.peekptr();
            if(!a)
                error("stack underflow");
            a=a->deref();
            if(a->getType() != Types::vtIterObj)
                error("can only iterate an iterator object");
            
            a->d.iterobj->next(); // step the iterator
            // if not done, jump back to just after the FOR
            if(!(a->d.iterobj->isDone())) {
                ip-=INSTDATA(op)+1;
                b = xstack.peekptr(1); // the loop index ref
                c = a->d.iterobj->current(); // get iterator value
                b->store(c); // store the value
            }
            break;
        case OP_ENDFOR:
            // just drop the loop index ref and iterator obj,
            // which have been lying around on the stack all
            // through the loop. This needs to be separate from
            // the OP_NEXT because a lana "break" will jump here.
            xstack.popptr(1); // pop 1 extra item, i.e. 2
            break;
        case OP_THIS:
            if(!thisptr)
                error("cannot use 'this' outside a method function/procedure");
            a = xstack.pushptr();
            a->setObj(thisptr);
            break;
        case OP_SQB:
            a = popval(); // the index
            b = popval(); // the item
            c = xstack.pushptr();
            if(!b->type->makeSQBRef(c,b,a))
                error("cannot use x[] when x is %s",b->type->getName());
            break;
        case OP_TRUE:
            a = xstack.pushptr();
            a->setBool(true);
            break;
        case OP_FALSE:
            a = xstack.pushptr();
            a->setBool(false);
            break;
        case OP_LITIDENT:
        case OP_IMMED:
            a = xstack.pushptr();
            a->setInt(INSTDATA(op));
            break;
        case OP_LIT:
            {
                char *p;
                // what's in the code is a descriptor - we need
                // to get the constant to which it refers
                
                a = xstack.pushptr();
                constid id = INSTDATA(op);
                ConstDesc *e = consts->get(INSTDATA(op));
                if(!e)
                    error("no literal");
                
                ConstType t = e->getType();
                switch(t){
                case CT_FUNC:
                    // it's a pointer to a function, we stack the function's
                    // offset in the CDT
                    a->setFunc(id);
                    break;
                case CT_STRING:
                    // it's a string, we stack the string's offset in the CDT
                    a->setStrConst(id);
                    break;
                case CT_INT:
                    a->setInt(*(int *)e->get());
                    break;
                case CT_FLOAT:
                    a->setFloat(*(float *)e->get());
                    break;
                case CT_LDT:
                    p = (char *)e->get();
                    a->setOther(Types::vtLDT,p);
                    break;
                case CT_COMMENT:
                    error("really should run LIT(commentID)");
                default:
                    error("bad const type in OP_LIT");
                }
            }
            break;        
        case OP_LOCALS:
            {
                LDTHeader *h = (LDTHeader *)consts->get(INSTDATA(op))->get();
                
                // make room for params and locals
                vstackbase = vstacknext;
                vstacknext += h->numlocals+h->numparams;
                if(df & LDEBUG_TRACE)
                    printf("Locals : %d locals, %d parameters\n",h->numlocals,h->numparams);
                
                // pop params into first part of that space
                int paramtop = vstacknext-h->numlocals;
                for(int i=0;i<h->numparams;i++){
                    vstack[paramtop-(i+1)] = *popval();
                    if(df & LDEBUG_TRACE)
                        printf("param n-%d : %s\n",i,a->deref()->repr());
                }
                
                // set up the locals pointer
                locals = vstack+vstackbase;
                
                // finally, drop the unneeded function pointer
                xstack.popptr();
            }
            break;
        case OP_VARREFLOC:
        case OP_VARREFPRM:
            {
                int n = INSTDATA(op);
                a = locals+n;
                b = xstack.pushptr();
                b->setOther(Types::vtRef,(void *)a);
            }
            break;
        case OP_VARREFGLB:
            {
                int n = INSTDATA(op);
                a = globs->get(n);
                b = xstack.pushptr();
                b->setOther(Types::vtRef,(void *)a);
            }
            break;
        case OP_VARREFSES:
            {
                int n = INSTDATA(op);
                a = ses->getSesVar(n);
                b = xstack.pushptr();
                b->setOther(Types::vtRef,(void *)a);
            }
            break;
        case OP_SET:
            a = popval();
	    if(a->type == Types::vtNativeMethodRef)
	      throw Exception("cannot store a reference to a native method in user code");
            b = xstack.popptr(); // ref to write it to
            b->store(a); // store
            break;
        case OP_STARTESTMT:
            exprstackct = xstack.ct;
            break;
        case OP_ENDESTMT:
    	    // make sure the execution stack is clear.
	     while(xstack.ct>exprstackct){
                Value *v = xstack.popptr();
                v->clr();
            }
	     // fall through.
        case OP_ENDESTMT2: // dummy version of the above for recreation purposes
	  cvb.clear(); // actually, we'd best do this always.
            break;
            
        case OP_DUMMY:
  	  break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_MOD:
        case OP_DIV:
            b=popval();
            a=popval(); // note reverse order
            a->type->doBinArithOp(xstack.pushptr(),INSTOP(op),a,b);
            break;
        case OP_EQUALS:
        case OP_NEQUALS:
        case OP_NEAREQ:
        case OP_NNEAREQ:
        case OP_LT:
        case OP_LTE:
        case OP_GT:
        case OP_GTE:
            b=popval();
            a=popval(); // note reverse order
            a->type->doBinComparisonOp(xstack.pushptr(),INSTOP(op),a,b);
            break;
        case OP_LOGAND:
            a=popval();
            b=popval();
            c=xstack.pushptr();
            c->setBool(a->getBool() && b->getBool());
            break;
        case OP_LOGOR:
            a=popval();
            b=popval();
            c=xstack.pushptr();
            c->setBool(a->getBool() || b->getBool());
            break;
        case OP_BITAND:
            a=popval();
            b=popval();
            c=xstack.pushptr();
            c->setInt(a->getInt() & b->getInt());
            break;
        case OP_BITOR:
            a=popval();
            b=popval();
            c=xstack.pushptr();
            c->setInt(a->getInt() | b->getInt());
            break;
        case OP_XOR:
            a=popval();
            b=popval();
            c=xstack.pushptr();
            c->setInt(a->getInt() ^ b->getInt());
            break;
        case OP_BITNOT:
            a=popval();
            b=xstack.pushptr();
            j=a->getInt();
            j = ~j;
            b->setInt(j);
            break;
        case OP_NEGATE:
            a=popval();
            b=xstack.pushptr();
            if(!a->type->negate(a,b))
                error("invalid value for unary minus: %s",a->type->getName());
            break;
        case OP_NOT:
            a=popval();
            b=xstack.pushptr();
            if(!a->type->unarynot(a,b))
                error("invalid value for unary not: %s",a->type->getName());
            break;
        case OP_CALL:
            doFuncCall(op);
            df = lana->debugFlags; // a bit of a waste, but a native call can change it
            break;
        
        case OP_ELSEIF:
        case OP_IF:
        case OP_QUICKIF:
        case OP_WHILE:
            try {
            if(!popval()->getBool())
                ip += INSTDATA(op)-1;
            } catch (Exception &e) {
                error(e.what());
            }
            break;
        case OP_ELSE:
        case OP_JMPELSEIF:
            ip += INSTDATA(op)-1;
            break;
       case OP_ENDIF:
            break;
            
            
        case OP_ENDWHILE:
            ip -= INSTDATA(op)+1;
            break;
            
        case OP_UNTIL:
            try {
                if(!popval()->getBool())
                    ip -= INSTDATA(op)+1;
            } catch (Exception &e) {
                error(e.what());
            }
            break;
        case OP_PROPREF:
            a=popval();
            b=xstack.pushptr();
            if(!a->type->makePropRef(b,a,INSTDATA(op)))
                error("cannot get non-standard property of non-object");
            break;
        default:
            error("not yet implemented: %d at %lx",INSTOP(op),ip-1);
            
            // no-ops
        case OP_REPEAT:
        case OP_PAREN:
        case OP_BLANKLINE:
        case OP_COMMENT_SOL:
        case OP_COMMENT_EOL:
        case OP_COMMENT_EOFD:
        case OP_GOTOMARKER:
        case OP_LABEL:
            break;
            
        case OP_GOTOFW:
        case OP_BREAK:
            ip += INSTDATA(op)-1;
            break;
        case OP_GOTOBK:
        case OP_CONTINUE:
            ip -= INSTDATA(op)+1;
            break;
            
            // oddities
        case OP_SPECIAL:
            doSpecial(INSTDATA(op));
            break;
        case OP_SRCFILE:
            file = INSTDATA(op);
            break;
        case OP_SRCLINE:
            line = INSTDATA(op);
            break;
        }
    }
    curSession = NULL;
}

void VirtualMachine::error(const char *s,...)
{
    char ebuf[1024]; 
    
    va_list args;
    va_start(args,s);
    
    vsnprintf(ebuf,256,s,args);
    clearAndFlush();
    throw RuntimeException(ebuf,getSourceFile(),getSourceLine());
    
    va_end(args);
}

void VirtualMachine::rpush(){
    ReturnData *r = retstack.pushptr();
    r->ret = ip;
    r->thisptr = thisptr;
    r->vstackbase = vstackbase;
    r->vstacknext = vstacknext;
    r->stkbase = stkbase;
    r->file = file;
    r->line = line;
}

bool VirtualMachine::rpop(){
    ReturnData *r = retstack.popptrnoex();
    if(!r)
        return true;
    ip = r->ret;
    
    file = r->file;
    line = r->line;
    vstackbase = r->vstackbase;
    vstacknext = r->vstacknext;
    locals = vstack+vstackbase;
    stkbase = r->stkbase;
    
    // coming out of a method releases a reference
    if(thisptr)
        thisptr->decRefCt();
       
    thisptr = r->thisptr;
    return false;
}


char *VirtualMachine::stkDump(){
    static char stbuf[1024];
    stbuf[0]=0;
    for(int i=0;i<xstack.ct;i++){
    Value *v = xstack.stack+i;
//        if(v->type == Types::vtRef){
//            snprintf(stbuf+strlen(stbuf),1024,"r:%s",
//                    v->debugDerefRepr());
//        } else
            strcat(stbuf,v->repr());
        strcat(stbuf," ");
    }
    return stbuf;
}


void VirtualMachine::doFuncCall(instruction op){
    // first get the argument count
    int argc = INSTDATA(op);
    
    // now get a reference to the function. Because of the nature
    // of the parser, this is BELOW the arguments. Messy, but modifying
    // the parser is messier.
    
    Value *fv = xstack.peekptr(argc);
    if(!fv)
        error("no function on stack in call");
    
    // before we dereference we see if it's a reference to a property.
    // If so, we set the new thisptr to the property's object.
    
    Object *newthis;
    if(fv->type==Types::vtPropRef)
        newthis = fv->d.o;
    else
        newthis = NULL;
    
    // now we can dereference
    fv = fv->deref();
    
    if(fv->type == Types::vtNativeFunctionRef){
        // One of the native C++ code options.
        // this is a reference to something which looks to lana like a function,
        // it might just be a plain C++ function, or might actually call a method in an object
        NativeFuncData *d = (NativeFuncData*)(fv->getPtr());
        if(d->argc!=argc)
            error("expected %d arguments, got %d",d->argc,argc);
        
        // we let the function run, popping its arguments off and pushing a return
        // value on (perhaps)
        
        if(d->ismethod){
            if(d->h) //a "hosted function", a bit like a delegate
                (d->h->*(d->d.m))();
            else
                error("badly created hosted function ref with null host");
        } else 
            // it's just a function, taking the API as a pointer.
            (*(d->d.f))((class API *)(d->h));
        
        // if the function returned a value, we swap the top two elements
        // on the stack so that the function ref is on top.
        if(d->returns)
            xstack.swap();
        
        // We then drop the function ref, leaving the return value if there was one.
        xstack.popptr();
    } else if(fv->type == Types::vtNativeMethodRef){
        // The other native C++ code option. In this case, it's a reference to something
        // which is a method in a C++ object. The data contains both the object and the native func data.
        // BUT we ignore the object if we're a propref because we don't want to call the object for which the
        // property was originally created, or inheritance won't work.
        Object *o = newthis?newthis:fv->d.o;
               
        NativeFuncData *d = fv->d2.nd;
        if(d->argc!=argc)
            error("expected %d arguments, got %d",d->argc,argc);
        
        // we let the function run, popping its arguments off and pushing a return
        // value on (perhaps)
        
        (o->*(d->d.m))();
        
        // if the function returned a value, we swap the top two elements
        // on the stack so that the function ref is on top.
        if(d->returns)
            xstack.swap();
        
        // We then drop the function ref, leaving the return value if there was one.
        xstack.popptr();
    } else if(fv->type == Types::vtFunction){  
        int *sizePtr = (int *)fv->getPtr(); // skip size data
        instruction *p = (instruction *)(sizePtr+1);
        
        // this bit is a bit slow and revolting. See how it goes, it's just an extra check.
        // get the value of argc from the OP_LOCALS instruction, the first in the function
        if(INSTOP(p[0])!=OP_LOCALS){
            error("bad initial opcode in user function");
            
        }
        LDTHeader *ldt = (LDTHeader *)consts->get(INSTDATA(p[0]))->get();
        if(ldt->numparams!=argc)
            error("expected %d arguments, got %d",ldt->numparams,argc);
        
        // now the check's done - we can push the context
        stkbase=xstack.ct;
        rpush();
        thisptr = newthis;
        
        if(thisptr)
            thisptr->incRefCt(); // going into a method implies a 'this' reference
        
        // and OP_LOCALS when it runs will do the argument/funcptr pop. All we need
        // to do now is change IP
        ip = p;
        
    }
    else {
        if(!fv->type)
            error("call of undefined function");
        else
            error("cannot call a non-function (type is '%s')",fv->type->getName());
    }
}


void VirtualMachine::doSpecial(int spec){
    switch(spec){
    case 0: // dump locals
        printf("LOCALS DUMP\n");
        for(int i=vstackbase;i<vstacknext;i++){
            Value *v = vstack+i;
            printf("Local/param %d = %s\n",i,v->deref()->repr());
        }
        break;
    case 1: // breakpointer
        printf("Break\n");
        break;
    case 2:
        // clear stack and flush locals
        clearAndFlush();
    }
}


const char *VirtualMachine::getSourceFile(){
    static char sfbuf[1024];
    if(file==Constants::NOTFOUND) {
        return "<<immediate>>";
    }
    
    return (const char *) consts->get(file)->get();
}

RuntimeException::RuntimeException(const char *e,const char *fn,int l) {
    char tmp[512];
    
    if(e){
        strcpy(tmp,e);e=tmp;  // to stop bloody exp-ptrcheck complaining.
    }
    
    if(fn)
        strncpy(fileName,fn,1024);
    else
        strcpy(fileName,"???");
    line = l;
    
    snprintf(error,1024,"%s(%d):  %s",fileName,line,
           e?e:"unknown");
   
}
