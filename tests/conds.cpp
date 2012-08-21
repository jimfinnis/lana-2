#include "tests.h"

void TestFixtureLana::testConds(){
    
    CPPUNIT_ASSERT_BOOLTEST("-7<10==true",true);
    CPPUNIT_ASSERT_BOOLTEST("7<10==true",true);
    CPPUNIT_ASSERT_BOOLTEST("10<10==false",true);
    CPPUNIT_ASSERT_BOOLTEST("11<10==false",true);
    
    CPPUNIT_ASSERT_BOOLTEST("-7<=10==true",true);
    CPPUNIT_ASSERT_BOOLTEST("7<=10==true",true);
    CPPUNIT_ASSERT_BOOLTEST("10<=10==true",true);
    CPPUNIT_ASSERT_BOOLTEST("11<=10==false",true);
    
    CPPUNIT_ASSERT_BOOLTEST("-7>10==false",true);
    CPPUNIT_ASSERT_BOOLTEST("7>10==false",true);
    CPPUNIT_ASSERT_BOOLTEST("10>10==false",true);
    CPPUNIT_ASSERT_BOOLTEST("11>10==true",true);
    
    CPPUNIT_ASSERT_BOOLTEST("-7>=10==false",true);
    CPPUNIT_ASSERT_BOOLTEST("7>=10==false",true);
    CPPUNIT_ASSERT_BOOLTEST("10>=10==true",true);
    CPPUNIT_ASSERT_BOOLTEST("11>=10==true",true);
    
    // quick check of quick-if
    ses->feed("aa=3");
    ses->feed("if 2<1: aa=6");
    CPPUNIT_ASSERT_INTVAR("aa",3);
    
    CPPUNIT_ASSERT_THROW(ses->feed("foo=(undefined<8)"),lana::RuntimeException);
    
    ses->feedFile("files/conds1.l");
    
    CPPUNIT_ASSERT_INTVAR("t1",12);
    CPPUNIT_ASSERT_INTVAR("t2",3);
    CPPUNIT_ASSERT_INTVAR("t3",7);
    CPPUNIT_ASSERT_INTVAR("t4",27);
    
    
        
}
