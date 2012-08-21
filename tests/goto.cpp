#include "tests.h"

void TestFixtureLana::testGoto(){
    // label outside a function is not valid
    CPPUNIT_ASSERT_THROW(ses->feed("foo:"),lana::ParseException);
    // goto outside a function is not valid
    CPPUNIT_ASSERT_THROW(ses->feed("goto foo"),lana::ParseException);
    
    // label must be end of line
    ses->feed("qqq=procedure()");
    CPPUNIT_ASSERT_THROW(ses->feed("lab: a=1"),lana::ParseException);
   
    ses->feedFile("files/goto.l");
}
