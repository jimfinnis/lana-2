#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestListener.h>
#include <cppunit/Test.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <stdio.h>
#include <stdlib.h>

#include <string>

#include "tests.h"


class JimListener : public CppUnit::TestListener {
    virtual void startTest(CppUnit::Test *test){
        printf("    Starting %s\n",test->getName().c_str());
    }
};



// the test driver 

int main(int argc,char *argv[]){
    // get top level suite
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    // add the test
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    
    // change the outputter
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(),
                                                       std::cerr));
    
    CppUnit::TestResult& r = runner.eventManager();
    
    JimListener jl;
    r.addListener(&jl);
    
    // run the tests
    bool wasOK;
    wasOK = runner.run("",false,true,false);
    
    return wasOK  ? 0 : 1;
}

void TestFixtureLana::_BOOLTEST(const char *test,bool equal,CppUnit::SourceLine line){
    
    lana::Value *v = api->findOrCreateGlobal("t674");
    char buf[256];
    sprintf(buf,"t674=(%s)",test);
    ses->feed(buf);
    
    bool r = v->getBool();
    sprintf(buf,"check t674=(%s) failed",test);
    ::CppUnit::Asserter::failIf(equal?!r:r,buf,line);
}

void TestFixtureLana::_INTVARTEST(const char *var,int val,CppUnit::SourceLine line){
    char buf[256];
    lana::Value *v = ses->getSesVar(var);
    if(v==NULL) {
        sprintf(buf,"global '%s' does not exist",var);
        ::CppUnit::Asserter::fail(buf,line);
    }
    
    int r = v->getInt();
    sprintf(buf,"check %s=%d failed, actual value is %d",var,val,r);
    ::CppUnit::Asserter::failIf(r!=val,buf,line);
}


//
//
//

CPPUNIT_TEST_SUITE_REGISTRATION(TestFixtureEmpty);
CPPUNIT_TEST_SUITE_REGISTRATION(TestFixtureLana);

void TestFixtureEmpty::setUp(){}

void TestFixtureEmpty::tearDown(){}

void TestFixtureLana::setUp(){
    api = new lana::API;
    ses = new lana::Session(api);
    h =new AsserterHost(api);
}
void TestFixtureLana::tearDown(){
    delete ses;
    delete api;
    delete h;
}


