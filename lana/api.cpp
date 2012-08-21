#include "language.h"
#include "vm.h"
#include "iterobj.h"
#include "api.h"

#include <stdio.h>

using namespace lana;

namespace lana {
class Host *registerCoreHost(API *,Language *);
void libcoreSetArgcArgv(API *a,class LibCoreHost *h,int argc,char *argv[]);
}

static bool apiOpen = false;

API::API(){
    if(apiOpen)
        throw Exception("Lana API already open!");
    apiOpen = true;
    
    new Language(this); // this->lana = returned value inside this ctor.
    vm = lana->vm;
    setDebug(LDEBUG_SRCDATA);
    prefix="";
    
    // register core library - see libcore.cpp
    coreLib=registerCoreHost(this,lana);
    
}

API::~API(){
    cycle->detect();
    delete coreLib;
    delete lana;
    apiOpen = false;
}

const char *API::getVersion(){
    return LANA_API_VERSION;
}
    

Value *API::getTempValue(){
  return vm->cvb.alloc();
}

void API::error(const char *s){
    vm->error(s);
}

void API::clearGlobals(){
    lana->globs->clear();
}

Value *API::findGlobal(const char *name){
    int id = lana->globs->find(name);
    if(id<0)
        return NULL;
    return lana->globs->get(id);
}

Value *API::findOrCreateGlobal(const char *name){
    // does this name exist? If not, make it. Get the
    // descriptor index for the name constant.
    int desc = lana->consts->findOrCreateString(name);
    // look for a global of that name
    int id = lana->globs->find(desc);
    if(id<0)
        // if it doesn't exist, make it
        id = lana->globs->create(desc);
    // return a pointer to it
    return lana->globs->get(id);
}

int API::getID(const char *name){
    return lana->registerID(name);
}

int API::setDebug(int flags){
    return lana->setDebug(flags);
}
int API::setFlags(int flags){
    return lana->setFlags(flags);
}

NativeFuncData *API::registerNativeMethod(const char *nm,
                                       int argc,
                                       bool returns,HOSTMETHOD m,
                                       class Host *h) {
    return lana->natFuncs.addMethod(getPrefixedName(nm),argc,returns,m,h);
}

void API::globalNativeFunctionOrMethod(const char *name,
                                       NativeFuncData *nd) {
    Value *v = lana->registerGlobalVariable(name);
    v->setNativeFunctionRef(nd);
}

void API::globalNativeHostedMethod(const char *name,int argc,bool returns,
                             Host *h,HOSTMETHOD m) {
    NativeFuncData *nd = registerNativeMethod(name,argc,returns,m,
                                                h);
    globalNativeFunctionOrMethod(name,nd);
}
    

void API::globalNativeFunction(const char *name,int argc,bool returns,
                                       VOIDFUNC f){
    NativeFuncData *nd = lana->natFuncs.addFunction(getPrefixedName(name),argc,returns,f,this);
    globalNativeFunctionOrMethod(name,nd);
}


int API::popInt(){
    Value *v = vm->popval();
    return v->getInt();
}
void API::pushInt(int i){
    Value *v = vm->pushptr();
    return v->setInt(i);
}
    
float API::popFloat(){
    Value *v = vm->popval();
    return v->getFloat();
}

void API::pushFloat(float f){
    Value *v = vm->pushptr();
    return v->setFloat(f);
}
    
char *API::popStr(){
    Value *v = vm->popval();
    return v->getStr();
}

Value *API::pushRaw(){
    return vm->pushptr();
}

Value *API::popRaw(){
    return vm->popval();
}
Value *API::popRawWithoutDeref(){
    return vm->popvalnoderef();
}

void API::pushStr(char *s){
    Value *v = vm->pushptr();
    return v->setStrClone(s);
}


int API::popBool(){
    Value *v = vm->popval();
    try {
        return v->getBool();
    } catch (Exception &e) {
        vm->error("boolean expected (by native function), got type '%s'",
                  v->type->getName(false));
    }
}

void API::pushBool(bool b){
    Value *v = vm->pushptr();
    return v->setBool(b);
}

class Object *API::popObj(){
    Value *v = vm->popval();
    return v->getObj();
}

void API::pushObj(class Object *o){
    Value *v = vm->pushptr();
    return v->setObj(o);
}



void API::resetInstructionCount(){
    vm->resetInstructionCount();
}
int API::getInstructionCount(){
    return vm->getInstructionCount();
}

int API::getSourceLine(){
    return vm->getSourceLine();
}

const char *API::getSourceFile(){
    return vm->getSourceFile();
}

void API::setArgcArgv(int argc,char **argv){
    libcoreSetArgcArgv(this,(class LibCoreHost *)coreLib,argc,argv);
    
}
