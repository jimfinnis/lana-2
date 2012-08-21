#include "tests.h"


void TestFixtureLana::testEqualityCoerce(){
    try{
        // the obvious cases
        CPPUNIT_ASSERT_BOOLTEST("3==3",true);
        CPPUNIT_ASSERT_BOOLTEST("3==2",false);
        
        CPPUNIT_ASSERT_BOOLTEST("3.0==3.0",true);
        CPPUNIT_ASSERT_BOOLTEST("3.0==2+1",true);
        CPPUNIT_ASSERT_BOOLTEST("3.0==2+0.2",false);
        CPPUNIT_ASSERT_BOOLTEST("2+1==3.0",true);
        CPPUNIT_ASSERT_BOOLTEST("2+0.2==3",false);
        
        // this is done as a float/float comparison
        CPPUNIT_ASSERT_BOOLTEST("3.1==3",false);
        CPPUNIT_ASSERT_BOOLTEST("3==3.1",false);

        CPPUNIT_ASSERT_BOOLTEST("\"foo\"==\"foo\"",true);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"==\"bar\"",false);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"~\"foo\"",true);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"~\"bar\"",false);
        
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"!=\"foo\"",false);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"!=\"bar\"",true);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"!~\"foo\"",false);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"!~\"bar\"",true);
        
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"~\"Foo\"",true);
        CPPUNIT_ASSERT_BOOLTEST("\"foo\"!~\"Foo\"",false);
        
        // this is fine, comparing an integer with a string which is evaluated as an integer
        CPPUNIT_ASSERT_BOOLTEST("3==\"3.0\"",true);
        // as above, but other way around
        CPPUNIT_ASSERT_BOOLTEST("\"3.0\"==3",true);
        
        // these should obviously be false
        CPPUNIT_ASSERT_BOOLTEST("3==\"4.0\"",false);
        CPPUNIT_ASSERT_BOOLTEST("\"4.0\"==3",false);
        
        // but this is the correct behaviour too - because the most specific type is an int, the string
        // is evaluated as an int even though it's interpretable as a float
        CPPUNIT_ASSERT_BOOLTEST("3==\"3.1\"",true);
        CPPUNIT_ASSERT_BOOLTEST("\"3.1\"==3",true);
        
        CPPUNIT_ASSERT_BOOLTEST("\"armadillo\"==0",true);   // string evaluated as integer
        CPPUNIT_ASSERT_BOOLTEST("\"armadillo\"==0.0",true); // string evaluated as float
        
        
        // inverses and nears
        
        CPPUNIT_ASSERT_BOOLTEST("3!=4",true);
        CPPUNIT_ASSERT_BOOLTEST("3!=3",false);
        CPPUNIT_ASSERT_BOOLTEST("3~3",true);
        CPPUNIT_ASSERT_BOOLTEST("3~4",false);
        CPPUNIT_ASSERT_BOOLTEST("3!~3",false);
        CPPUNIT_ASSERT_BOOLTEST("3!~4",true);
        CPPUNIT_ASSERT_BOOLTEST("3.0~3.0",true);
        CPPUNIT_ASSERT_BOOLTEST("3.0~4.0",false);
        CPPUNIT_ASSERT_BOOLTEST("3.0!~3.0",false);
        CPPUNIT_ASSERT_BOOLTEST("3.0!~4.0",true);
        
        
        // some more simple expressions
        
        CPPUNIT_ASSERT_BOOLTEST("7+3==5+5",true);
        CPPUNIT_ASSERT_BOOLTEST("7+3==\"5+5\"",false); // the string will not be evaluated correctly
        CPPUNIT_ASSERT_BOOLTEST("7+3==\"10+2\"",true); // the string will not be evaluated correctly, just as "10"
        
        CPPUNIT_ASSERT_BOOLTEST("7+3.0==5+5",true); // true -  will be evaluated as a float compare and 10.0 == 10
        CPPUNIT_ASSERT_BOOLTEST("7.1+3==5+5",false); // false - will be evaluated as a float compare and 10.1 != 10
    } catch(lana::Exception &e){
        const char *s = ses->getLastLine();
        char buf[1024];
        sprintf(buf,"LINE: %s ERROR: %s",s,e.error);
        CPPUNIT_FAIL(buf);
    }
}
