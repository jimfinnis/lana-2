#include "tests.h"

void TestFixtureLana::testArithmetic(){
    try{
        CPPUNIT_ASSERT_BOOLTEST("3+4==7",true);
        CPPUNIT_ASSERT_BOOLTEST("-7+10==3",true);
        CPPUNIT_ASSERT_BOOLTEST("7+-10==-3",true);
        
        // subtraction
        CPPUNIT_ASSERT_BOOLTEST("7-2==5",true);
        CPPUNIT_ASSERT_BOOLTEST("7-10==-3",true);
        CPPUNIT_ASSERT_BOOLTEST("1-1==1",false);
        CPPUNIT_ASSERT_BOOLTEST("3-1-1-1==0",true);
        CPPUNIT_ASSERT_BOOLTEST("4--2==6",true);
        CPPUNIT_ASSERT_BOOLTEST("7.1-2==5.1",true);
        CPPUNIT_ASSERT_BOOLTEST("7-10==-3.0",true);
        CPPUNIT_ASSERT_BOOLTEST("1-1.1==1",false);
        CPPUNIT_ASSERT_BOOLTEST("1-1.1==-0.1",false); // floating point rounding errors...
        CPPUNIT_ASSERT_BOOLTEST("1-1.1!=-0.1",true); // floating point rounding errors...
        
        CPPUNIT_ASSERT_BOOLTEST("1-1.1~-0.1",true); // near equality test
        CPPUNIT_ASSERT_BOOLTEST("1-1.1!~-0.11",true); // near equality test
        
        CPPUNIT_ASSERT_BOOLTEST("4--2.5==6.5",true);
        
        // multiplication
        CPPUNIT_ASSERT_BOOLTEST("40*123==4920",true);
        CPPUNIT_ASSERT_BOOLTEST("-10.6*-11~116.6",true);
        CPPUNIT_ASSERT_BOOLTEST("10*-11.6~-116.0",true);
        CPPUNIT_ASSERT_BOOLTEST("-0.43*0.32~-0.1376",true);
        
        // division
        CPPUNIT_ASSERT_BOOLTEST("6/2==3",true);
        CPPUNIT_ASSERT_BOOLTEST("6/2/3==1",true);
        CPPUNIT_ASSERT_BOOLTEST("6/0.25==24",true);
        CPPUNIT_ASSERT_BOOLTEST("6/-7~-0.857142857",false); // integer divide
        CPPUNIT_ASSERT_BOOLTEST("6.0/-7~-0.857142857",true); 
        CPPUNIT_ASSERT_BOOLTEST("6.0/-7.0~-0.857142857",true);
        CPPUNIT_ASSERT_BOOLTEST("-6/-7.0~0.857142857",true);
        CPPUNIT_ASSERT_BOOLTEST("-6/7.0~-0.857142857",true);
        
        // addition and multiplication
        CPPUNIT_ASSERT_BOOLTEST("4*3+1+1+1==15",true);
        CPPUNIT_ASSERT_BOOLTEST("1+4+4*2+1+1+1==16",true);
        CPPUNIT_ASSERT_BOOLTEST("4+3*7+2==27",true);
        CPPUNIT_ASSERT_BOOLTEST("4*3+7*2==26",true);
        CPPUNIT_ASSERT_BOOLTEST("4.8+3*7+2~27.8",true);
        CPPUNIT_ASSERT_BOOLTEST("4*3+7*2.1~26.7",true);
        
        // subtraction and division
        CPPUNIT_ASSERT_BOOLTEST("4/2-1-1==0",true);
        CPPUNIT_ASSERT_BOOLTEST("1-4-5/2-1-1==-7.5",false);
        CPPUNIT_ASSERT_BOOLTEST("1-4-5.0/2-1-1==-7.5",true);
        
        // div and mul
        // addition and division
        CPPUNIT_ASSERT_BOOLTEST("4/2*10/5==4",true);
        
        // lots
        CPPUNIT_ASSERT_BOOLTEST("4+5*3+1-4/2==18",true);
        CPPUNIT_ASSERT_BOOLTEST("5--3*-2/4+6~9.5",false);
        CPPUNIT_ASSERT_BOOLTEST("5.0--3*-2/4+6~9.5",false);
        CPPUNIT_ASSERT_BOOLTEST("5--3*-2/4.0+6~9.5",true);
        
        // brackets
        
        // check syntax errors
        CPPUNIT_ASSERT_THROW(ses->feed("foo=-(-)==-4"),lana::ParseException);
        CPPUNIT_ASSERT_THROW(ses->feed("foo=()==-4"),lana::ParseException);
        
        CPPUNIT_ASSERT_THROW(ses->feed("foo=undefined"),lana::RuntimeException);
        
        CPPUNIT_ASSERT_BOOLTEST("-(4)==-4",true);
        CPPUNIT_ASSERT_BOOLTEST("-(4+3)==-7",true);
        CPPUNIT_ASSERT_BOOLTEST("-(4+3)+2==-5",true);
        CPPUNIT_ASSERT_BOOLTEST("-(4+3)*2==-14",true);
        CPPUNIT_ASSERT_BOOLTEST("(5+2)*3==21",true);
        CPPUNIT_ASSERT_BOOLTEST("5*(2+3)*2==50",true);
        CPPUNIT_ASSERT_BOOLTEST("5*(3-10)-2*(3-8)==-25",true);
    } catch(lana::Exception &e){
        const char *s = ses->getLastLine();
        char buf[1024];
        sprintf(buf,"LINE: %s ERROR: %s",s,e.error);
        CPPUNIT_FAIL(buf);
    }
}
        
        
