#include "tests.h"

void TestFixtureLana::testAssignVar(){
    try{
        ses->feed("bar=5"); // assign a variable
        CPPUNIT_ASSERT_BOOLTEST("bar==5",true); // make sure we read the value
        
        ses->feed("boz=bar"); // test we assign the value rather than the reference
        // find the new "boz" and make sure it has a value in it.
        lana::Value *v = ses->getSesVar("boz");
        CPPUNIT_ASSERT_EQUAL(lana::Types::vtInteger,v->getType());
        CPPUNIT_ASSERT_BOOLTEST("boz==5",true); // make sure it's the right value
        
        ses->feed("bar=6.4");
        CPPUNIT_ASSERT_BOOLTEST("bar==6",false); // float/float comparison
        
        ses->feed("bar=\"hello world\"");
        CPPUNIT_ASSERT_BOOLTEST("bar==\"hello world\"",true);
        CPPUNIT_ASSERT_BOOLTEST("bar==\"Hello world\"",false); // case independence
        CPPUNIT_ASSERT_BOOLTEST("bar==0",true); // evaluates to zero int
        
        // copy string variable
        ses->feed("boz=bar");
        CPPUNIT_ASSERT_BOOLTEST("boz==\"hello world\"",true);
        CPPUNIT_ASSERT_BOOLTEST("boz==\"Hello world\"",false); // case independence
        
        
    } catch(lana::Exception &e){
        const char *s = ses->getLastLine();
        char buf[1024];
        sprintf(buf,"LINE: %s ERROR: %s",s,e.error);
        CPPUNIT_FAIL(buf);
    }
}
        
        
