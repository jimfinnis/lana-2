#include "tests.h"

static void qqq(lana::Session *ses,const char *s){
    CPPUNIT_ASSERT_THROW(ses->feed(s),lana::ParseException);
}

void TestFixtureLana::testRecovery(){
//    api->setDebug(0xffffffff);
    qqq(ses,	"for x in");
    ses->feed(	"q=function()");
    qqq(ses,	"   for x");
    qqq(ses,    "   for x in range(0,");
    ses->feed(  "   ct=0");
    ses->feed(  "   for x in range(0,10)");
    qqq(ses,    "      ct+=10");
    ses->feed(  "      ct=ct+x");
ses->feed("print(ct)");
    ses->feed(  "   endfor");
    ses->feed(  "   return ct");
    ses->feed(  "end");
    ses->feed(  "t1=q()");
    CPPUNIT_ASSERT_INTVAR("t1",45);
    api->setDebug(0);
}
