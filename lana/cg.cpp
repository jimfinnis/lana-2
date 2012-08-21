#include "cg.h"
#include "value.h"
#include "consts.h"
#include "growable.h"
#include "opcodes.h"
#include "language.h"
#include "compiler.h"
#include "session.h"
#include <stdio.h>
#include <stdarg.h>

CodeGenContext::CodeGenContext() {
    code = new Growable(1024,1024,1); 
    clear();
}

CodeGenContext::~CodeGenContext() {
    clear();
    delete code;
}

CodeGen::~CodeGen(){
    clearall();
    delete current;
}

void CodeGenContext::clear()
{
    estack.clearall();
    code->clear();
    cstack.clear();
    loopstack.clear();
    
    std::map<int,Label *>::const_iterator end = labels.end();
    for(std::map<int,Label *>::const_iterator it = labels.begin();
        it!=end;++it){
        delete it->second;
    }
    labels.clear();
    
    ldth.init(); // clear all locals and params
}

void CodeGenContext::epush(int t,int p){
    ExprItem item;
    item.type = t;
    
    item.precedence = p;
    
    try{
        estack.push(item);
    } catch(StackException &s) {
        throw ParseException("overly complex expression");
    }
    /*    
       SimpleStack<ExprItem,128> *es = estack.getstack();
       
       printf("push tok %d: now ",t);
       for(int i=0;i<es->ct;i++){
       printf("%d ",es->stack[i]);
       }
       printf("\n");
     */
}

ExprItem *CodeGenContext::epop() {
    
    ExprItem *pi = estack.popptrnoex();
    /*    
       SimpleStack<ExprItem,128> *es = estack.getstack();
       printf("pop tok %d: now ",pi->type);
       for(int i=0;i<es->ct;i++){
       printf("%d ",es->stack[i]);
       }
       printf("\n");
     */  
    return pi;
}

ExprItem *CodeGenContext::epeek() {
    return estack.peekptr();
}


void CodeGen::pushestack() {
    current->estack.pushstack();
}

void CodeGen::popestack() {
    current->estack.popstack();
}

bool CodeGenContext::cempty(){
    return cstack.isempty();
}

void CodeGenContext::cpush(int n){
    try {
        cstack.push(n);
    } catch(StackException &e) {
        throw ParseException("cstack overflow");
    }
}    

void CodeGenContext::cpushhere(){
    cpush(getloc());
}

int CodeGenContext::getloc(){
    return code->getOffset()/sizeof(instruction);
}

instruction *CodeGenContext::getlocptr(){
    return (instruction *)code->get(code->getOffset(),0);
}

instruction *CodeGenContext::cpoplocation(){
    int n = cpop();
    return (instruction *)code->get(n*sizeof(instruction),0);
}

instruction *CodeGenContext::cpoplocandcheck(int opmin,int opmax){
    va_list ap;
    
    instruction *p = cpoplocation();
    int op = INSTOP(*p);
    if(op<opmin || op>opmax)
        return NULL;
    return p;
}

int CodeGenContext::getdiff(instruction *op){
    // get current location
    instruction *cur = (instruction *)code->get(code->getOffset(),0);
    // return the difference from op to cur
    return cur-op;
}


int CodeGenContext::cpop(){
    try {
        return cstack.pop();
    } catch(StackException &e) {
        throw ParseException("stack underflow in cstack - mismatched control structure?");
    }
    
}

int CodeGenContext::cpeek(){
    try {
        return cstack.peek();
    } catch(StackException &e) {
        throw ParseException("stack underflow in cstack - mismatched control structure?");
    }
}

instruction *CodeGenContext::getPtr(int offset) {
    return (instruction *)code->get(offset,0);
}

void CodeGenContext::newloop(){
    LoopData *d = loopstack.pushptr();
    d->continuelabel.set(getlocptr());
}

void CodeGenContext::endloop(){
    LoopData *d = loopstack.popptr();
    d->breaklabel.set(getlocptr());
    d->reset();
}


void CodeGenContext::saveSnapshot(){
    // record the output pointer. This is the point to which we might need
    // to rewind
    code->saveSnapshot();
    snaplabels=labels;
    snaploopstack=loopstack;
    snapcstack=cstack;
}

void CodeGenContext::restoreSnapshot(){
    estack.clearall();
    
    code->restoreSnapshot();
    
    // now a hideous thing; we need to remove all labels which
    // are in "labels" but not in "snaplabels", since they were
    // created later. This will delete any labels created during
    // an statement which later proved to have syntax errors.
    //
    // To be honest, this probably isn't entirely necessary at the
    // moment, because the only time this will happen is if an error
    // occurs after a label is created using "label:" statement but
    // before moving on to the next statement. There aren't many ways
    // this can happen - I believe the only way is when there's garbage
    // after the ":" . But there might be more stuff later.
    
    std::map<int,Label *>::const_iterator end = labels.end();
    for(std::map<int,Label *>::const_iterator it = labels.begin();
        it!=end;++it){
        // it's in labels, is it in snaplabels?
        if(snaplabels.find(it->first)==snaplabels.end()){
            delete it->second;
        }
    }
    labels=snaplabels;
    loopstack=snaploopstack;
    cstack=snapcstack;
}



CodeGen::CodeGen(Language *l,Session *s){
    stackct = 0;
    current = new CodeGenContext();
    ses = s;
    lana = l;
}

void CodeGen::clear(){
    current->clear();
}
void CodeGen::clearall(){
    while(stackct){
        popContext();
    }
    clear();
}

void CodeGen::pushContext(){
    if(stackct==8)
        throw ParseException("out of context stack in compiler");
    stack[stackct++] = current;
    current = new CodeGenContext();
}

void CodeGen::popContext(){
    if(!stackct)
        throw ParseException("stack underflow in compiler");
    stackct--;
    delete current;
    current = stack[stackct];
}

const char *CodeGen::writeContextToMemory(){
    
    int size = current->code->getOffset();
    int *ptr = (int *)malloc(size+sizeof(int));
    
    // write the size as a header
    *ptr = size;
    
    // copy the code to just after the header
    memcpy(ptr+1,current->code->get(0,0),size);
    
    if(lana->debugFlags & LDEBUG_DUMP){
        printf("Function object dump: \n");
        lana->dumpCode((instruction *)current->code->get(0,0),
                       current->code->getOffset()/sizeof(instruction),ses);
    }
    
    return (const char *)ptr;
}

void CodeGen::saveSnapshot(){
    current->saveSnapshot();
}

void CodeGen::restoreSnapshot(){
    current->restoreSnapshot();
}

void CodeGen::emit(int op,int n){
    instruction *p = (instruction *)current->code->allocget(4);
    extern const char *opcodes[];
    *p = INST(op,n);
    if(lana->debugFlags & LDEBUG_EMIT)
        printf("EMIT %x: %s\n",p,lana->dumpInst(p,ses));
}


void CodeGen::checkReturns(bool rets){
    instruction *p = (instruction *)current->code->get(0,0);
    for(;;){
        switch(INSTOP(*p)){
        case OP_END:
            if(rets)ses->compiler->error("end of function without returning a value");
            return;
        case OP_RETURN:
            if(INSTDATA(*p)){
                if(!rets)ses->compiler->error("function returns a value when not declared as 'returns'");
            }else{
                if(rets)ses->compiler->error("return in function without returning a value");
            }
            return;
        }
        // must add other stuff here!
        p++;
    }
    
}

