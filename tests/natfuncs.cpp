#include "tests.h"

class TestHost : public lana::Host {
public:
    TestHost(lana::API *a) : lana::Host(a) {
        // takes 1 argument, returns a value
        a->globalNativeHostedMethod("test1",1,true,this,(lana::HOSTMETHOD)&TestHost::test1);
        // takes 2 arguments, returns a value
        a->globalNativeHostedMethod("test2",2,true,this,(lana::HOSTMETHOD)&TestHost::test2);
        a->globalNativeHostedMethod("test3",0,false,this,(lana::HOSTMETHOD)&TestHost::test3);
    }
private:    
    // double the first argument and return it
    void test1(){
        int i = api->popInt(); // using easy interface
        api->pushInt(i*2);
    }
    
    // subtract the second argument from the first
    void test2(){
        int b = api->popFloat();
        int a = api->popFloat();
        api->pushFloat(a-b);
    }
    
    // takes no arguments or values
    void test3(){
        lana::Value *v = api->findOrCreateGlobal("baz");
        v->setStrClone("biscuit");
    }
};


void TestFixtureLana::testNativeFunctions(){
    TestHost *h=new TestHost(api);
    
    ses->feed("bar=test1(5)");
    CPPUNIT_ASSERT_EQUAL(10,ses->getSesVar("bar")->getInt());
    
    ses->feed("bar=test1(bar)");
    CPPUNIT_ASSERT_EQUAL(20,ses->getSesVar("bar")->getInt());
    
    ses->feed("bar=test2(20,3)");
    CPPUNIT_ASSERT_EQUAL(17,ses->getSesVar("bar")->getInt());
    
    // sets a global
    ses->feed("test3()");
    CPPUNIT_ASSERT_STREQUAL("biscuit",api->findGlobal("baz")->getStr());
    delete h;
}

