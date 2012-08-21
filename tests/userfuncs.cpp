#include "tests.h"

void TestFixtureLana::testUserFunctions(){
    
    try{
        // check that end and return in immediate mode throw exception
        CPPUNIT_ASSERT_THROW(ses->feed("end"),lana::ParseException);
        CPPUNIT_ASSERT_THROW(ses->feed("return"),lana::ParseException);
        
        ses->feed("foo = function(a)");
        ses->feed("  return 2*a");
        ses->feed("end");
        
        // check we can use functions in expressions
        ses->feed("a=foo(3)+foo(4)");
        CPPUNIT_ASSERT_INTVAR("a",14);
        ses->feed("a=(foo(3)+foo(4))*-foo(3)");
        CPPUNIT_ASSERT_INTVAR("a",-84);
        
        // check that not passing an argument barfs
        CPPUNIT_ASSERT_THROW(ses->feed("foo()"),lana::RuntimeException);
        
        // THIS CURRENTLY PRINTS A LINE!
        ses->feed("foo(5)"); // not using returned value is OK - it gets printed
        ses->feed("a=foo(5)");
        CPPUNIT_ASSERT_INTVAR("a",10);
        
        // check the parameters are read in the correct order
        ses->feed("foo = function(a,b,c)");
        ses->feed("  return a*100+b*10+c");
        ses->feed("end");
        ses->feed("a=foo(1,2,3)");
        CPPUNIT_ASSERT_INTVAR("a",123);
                  
        
        
        // simple checks for returning a value when we shouldn't, and not doing that
        // when we should
        
        ses->feed("foo = procedure(a)");
        // can't return a value, we've not specified "returns"
        CPPUNIT_ASSERT_THROW(ses->feed("  return 2*a"),lana::ParseException);
        ses->feed("end"); // have to end the procedure, despite the error
        
        ses->feed("foo = function(a)");
        // must return a value
        CPPUNIT_ASSERT_THROW(ses->feed("  return"),lana::ParseException);
        // still haven't returned a value
        CPPUNIT_ASSERT_THROW(ses->feed("end"),lana::ParseException);
        ses->feed("return a+4");
        ses->feed("end");

        // check a function which sets a global and a local
        
        ses->feed("bar=6"); // global - well, session actually.
        ses->feed("print(bar)");
        CPPUNIT_ASSERT_INTVAR("bar",6);
        ses->feed("foo = procedure()");
        ses->feed("  bar=12"); // set global
        ses->feed("  qux=4"); // set local
        ses->feed("end");
        
        ses->feed("qux=8"); // create global AFTER defining function
        ses->feed("foo()"); // run function
        CPPUNIT_ASSERT_INTVAR("bar",12); // this changes
        CPPUNIT_ASSERT_INTVAR("qux",8);  // this doesn't
        
        ses->feed("foo = procedure(a)");
        ses->feed("  a=a*4");
        ses->feed("end");
        ses->feed("b=4");
        ses->feed("foo(b)");
        CPPUNIT_ASSERT_INTVAR("b",4);  // this doesn't
        
        // now more complex tests, which we'll feed using
        // external files. We'll use a fresh API for this
        
        delete ses;
        delete api;
        api = new lana::API();
        ses = new lana::Session(api);
        
        ses->feedFile("files/userfuncs1.l");
        CPPUNIT_ASSERT_INTVAR("t0",25);
        CPPUNIT_ASSERT_INTVAR("t1",1);
        CPPUNIT_ASSERT_INTVAR("t2",-5);
        CPPUNIT_ASSERT_INTVAR("bar",9756);
        CPPUNIT_ASSERT_INTVAR("fac10",3628800);
        
        // and a test of nested functions
        delete ses;
        delete api;
        api = new lana::API();
        ses = new lana::Session(api);
        AsserterHost *a = new AsserterHost(api);
        
        ses->feedFile("files/userfuncs2.l");
        delete a;
    } catch(lana::Exception &e){
        const char *s = ses->getLastLine();
        char buf[1024];
        sprintf(buf,"LINE: %s ERROR: %s",s,e.what());
        CPPUNIT_FAIL(buf);
    }
}
        
        
