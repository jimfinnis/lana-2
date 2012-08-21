#include "tests.h"

// actually tests globals and sessions

void TestFixtureLana::testGlobals(){
    lana::Value *v;
    
    // test globals
    
    CPPUNIT_ASSERT(!api->findGlobal("foo"));
    v = api->findOrCreateGlobal("foo");
    CPPUNIT_ASSERT(v);
    CPPUNIT_ASSERT(api->findGlobal("foo"));
    
    v->setStrClone("hello");
    v = api->findGlobal("foo");
    CPPUNIT_ASSERT_STREQUAL("hello",v->getStr());
    
    v = api->findOrCreateGlobal("bar");
    CPPUNIT_ASSERT(v!=api->findGlobal("foo"));
    v->setInt(4036);
    
    CPPUNIT_ASSERT_EQUAL(4036,api->findGlobal("bar")->getInt());
    CPPUNIT_ASSERT_STREQUAL("hello",api->findGlobal("foo")->getStr());
}

void TestFixtureLana::testGlobAssign(){
    try{
        // create in sessions
        
        ses->feed("foo=5");
        lana::Value *v = api->findGlobal("foo");
        CPPUNIT_ASSERT(!v); // shouldn't be a global
        CPPUNIT_ASSERT_EQUAL(5,ses->getSesVar("foo")->getInt());
        
        ses->feed("bar=foo");
        CPPUNIT_ASSERT_EQUAL(5,ses->getSesVar("bar")->getInt());
        
        ses->feed("foo=3.2");
        CPPUNIT_ASSERT_EQUAL(3.2f,ses->getSesVar("foo")->getFloat());
        
        ses->feed("bloop=\"bing\"");
        CPPUNIT_ASSERT_STREQUAL("bing",ses->getSesVar("bloop")->getStr());
    } catch(lana::Exception &e){
        const char *s = ses->getLastLine();
        char buf[1024];
        sprintf(buf,"LINE: %s ERROR: %s",s,e.error);
        CPPUNIT_FAIL(buf);
    }
}

