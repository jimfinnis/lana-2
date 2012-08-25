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


/// the epsilon value for OP_NEAREQ etc. It's a global for speed.
/// It's bound to a global variable "arithEpsilon"
float *neareq_epsilon;

namespace lana {
extern TokenRegistry tokens[];
}

Language::Language(class API *a){
    eofdcomment=NULL;
    currentRecreateLDT = NULL;
    api = a;
    a->lana = this; // hack, so createTypes() will work.
    
    
    tok = new Tokeniser();
    tok->init();
    tok->settokens(tokens);
    consts = new Constants();
    globs = new GlobalVars(consts);
    recreateIndent = 0;
    debugFlags = 0;
    opFlags = 0;
    tmpgrow = new Growable(1024,1024,1);
    vm = new VirtualMachine(this); // after everything else
    a->cycle = &cycle;
    Value::setConsts(consts);
    Types::createTypes(a);
    
    // create and set initial epsilon
    Value *v = registerGlobalVariable("arithEpsilon");
    v->setFloat(1.0e-3);
    neareq_epsilon = &(v->d.f);
}


Language::~Language(){
    cycle.detect();
    
    delete tok;
    delete globs;
    delete consts;
    delete vm;

    delete tmpgrow;
    
    Types::deleteTypes();
}




void Language::dprintf(const char *s,...)
{
    char buf[256];
        
    va_list args;
    va_start(args,s);
        
    vsprintf(buf,s,args);
    printf("LANA> %s\n",buf);
    va_end(args);
}




Value *Language::registerGlobalVariable(const char *name){
    // create a global
    int desc = consts->findOrCreateString(name);
    int id = globs->find(desc);
    if(id>=0)
        throw Exception(NULL).set("global %s already exists",name);
    id = globs->create(desc);
    Value *v = globs->get(id);
    globs->setSystemGlobMarker();
    
    return v;
}
    
void Language::dumpCode(instruction *firstinst,int size,Session *s){
    for(int i=0;i<size;i++){
        printf("%s\n",dumpInst(firstinst+i,s));
    }
}    

constid Language::registerID(const char *id){
    return consts->findOrCreateString(id);
}
    


/// this is here just so that AC_CHECK_LIB can check for it

extern "C" {
void is_lana_present() {
}

}

