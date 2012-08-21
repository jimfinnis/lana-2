// check the asserter works!

#include "tests.h"



void TestFixtureLana::testAsserter(){
    ses->feed("assert(true)");
    
    api->setDebug(LDEBUG_SRCDATA);
    ses->feedFile("files/testassert1.l");
    
    ses->feed("foo(2,1)"); // shouldn't assert
    try{
        ses->feed("foo(1,2)"); // should assert
        CPPUNIT_FAIL("assertion expected");
    } catch(AssertException& e){
        CPPUNIT_ASSERT_STREQUAL("files/testassert1.l line 2 assertion failed",e.what());
    }
    
    // try the new assert macro
    
    CPPUNIT_ASSERT_ASSERTS(ses,"foo(1,2)","files/testassert1.l",2);
    api->setDebug(0);
}
