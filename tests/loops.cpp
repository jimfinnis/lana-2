#include "tests.h"

void TestFixtureLana::testLoops(){
    // loop statements outside functions are not valid
    CPPUNIT_ASSERT_THROW(ses->feed("while 1"),lana::ParseException);
    CPPUNIT_ASSERT_THROW(ses->feed("endwhile"),lana::ParseException);
    CPPUNIT_ASSERT_THROW(ses->feed("repeat"),lana::ParseException);
    CPPUNIT_ASSERT_THROW(ses->feed("until 1"),lana::ParseException);
    CPPUNIT_ASSERT_THROW(ses->feed("break"),lana::ParseException);
    CPPUNIT_ASSERT_THROW(ses->feed("continue"),lana::ParseException);
    
    ses->feedFile("files/loops.l");
    ses->feedFile("files/nestedloops.l");
    ses->feedFile("files/range.l");
}
        
        
