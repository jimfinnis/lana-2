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
#include "api.h"
#include "session.h"
#include "compiler.h"

Session::Session(API *a){
    api = a;
    lana = a->lana;
    vars = new Vars(lana->consts);
    compiler = new Compiler(this,lana);
}

Session::~Session(){
    delete vars;
    delete compiler;
}

void Session::feed(const char *buf){
    // set the values up to use this instance's area, in case it's been changed
    lana->setValueConsts();
    // pass the string in, with the session
    compiler->feed(buf);
}

void Session::feedFile(const char *fileName){
    lana->setValueConsts();
    compiler->feedFile(fileName);
}

const char *Session::recreate(instruction *op){
    const char *s = lana->recreate(op,this);
    return s;
}

bool Session::awaitingInput(){
    return compiler->awaitingInput();
}

void Session::addListener(event e, Listener *p){
    compiler->addListener(e,p);
}

const char *Session::getLastLine(){
    return compiler->getLastLine();
}

int Session::findOrCreateSesVar(u32 desc){

    int id = vars->find(desc);
    if(id<0)
        id = vars->create(desc);
    return id;
}

int Session::findSesVar(u32 desc){
    return vars->find(desc);
}

Value *Session::getSesVar(int svk){
    Value *v=vars->get(svk);
    if(!v)
        throw Exception("session variable has a key but doesn't exist");
    return v;
}
    
Value *Session::getSesVar(const char *name,bool create){
    int desc = lana->consts->findOrCreateString(name);
    int slot = vars->find(desc);
    if(slot<0){
        if(create)slot = vars->create(desc);
        else return NULL;
    }
    return getSesVar(slot);
}


char *Session::getSesVarName(int svk){
    int desc = vars->getName(svk);
    return lana->consts->getStr(desc);
}
