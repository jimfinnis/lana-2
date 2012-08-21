#ifndef __ASSERTER_H
#define __ASSERTER_H

#include "lana/api.h"
#include <exception>

class AssertException : public std::exception {
public:
    AssertException(const char *file,int line){
        if(file)
            sprintf(buf,"%s line %d assertion failed",file,line);
        else
            sprintf(buf,"??? line %d assertion failed",line);
        
        myLine = line;
        myFile = file;
    }
    AssertException(int actual,int expected,const char *file,int line){
        sprintf(buf,"%s:%d assertion failed, actual:%d, expected:%d",file,line,actual,expected);
    }
    AssertException(const char *actual,const char *expected,const char *file,int line){
        sprintf(buf,"%s:%d assertion failed, actual:%s, expected:%s",file,line,actual,expected);
    }
    
    virtual const char *what () const throw (){
        return buf;
    }
    
    char buf[1024];
    int myLine;
    const char *myFile;
   
};

/// this is a host object which will add native functions for asserts,
/// for later parts of the test suite written entirely in Lana.

class AsserterHost : public lana::Host {
public:
    AsserterHost(lana::API *a) : lana::Host(a) {
        a->setNamePrefix("Assert$");
        a->globalNativeHostedMethod("assert",1,false,this,
                                  (lana::HOSTMETHOD)&AsserterHost::assert);
        a->globalNativeHostedMethod("fail",0,false,this,
                                  (lana::HOSTMETHOD)&AsserterHost::fail);
        a->globalNativeHostedMethod("assertInt",2,false,this,
                                  (lana::HOSTMETHOD)&AsserterHost::assertInt);
        a->globalNativeHostedMethod("assertStr",2,false,this,
                                  (lana::HOSTMETHOD)&AsserterHost::assertStr);
    }
private:    
    void assert(){ // condition
        int cond = api->popBool();
        if(!cond)
            throw AssertException(api->getSourceFile(),api->getSourceLine());
    }
    
    void fail(){
            throw AssertException(api->getSourceFile(),api->getSourceLine());
    }
    
    void assertInt(){ //expected,actual
        int actual = api->popInt();
        int expected = api->popInt();
        
        if(actual!=expected)
            throw AssertException(actual,expected,api->getSourceFile(),api->getSourceLine());
    }
    void assertStr(){ //expected,actual
        const char *actual = api->popStr();
        const char *expected = api->popStr();
        
        if(strcmp(actual,expected))
            throw AssertException(actual,expected,api->getSourceFile(),api->getSourceLine());
    }
};

#endif /* __ASSERTER_H */
