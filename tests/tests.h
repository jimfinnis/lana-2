#ifndef __TESTS_H
#define __TESTS_H

#include <stdio.h>
#include <string.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>



#include "lana/api.h"
#include "lana/debug.h"
#include "lana/flags.h"

#include "asserter.h"

/// test fixture containing no data - a null fixture

class TestFixtureEmpty : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestFixtureEmpty);
    CPPUNIT_TEST(testConstants);
    CPPUNIT_TEST(testLists);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp();
    void tearDown();
    
    void testConstants();
    void testLists();
};

/// test fixture consisting of a single Lana interpreter,
/// probably, although at the moment we're not using it
/// like that!

class TestFixtureLana : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestFixtureLana);
    CPPUNIT_TEST(testGlobals);
    CPPUNIT_TEST(testGlobAssign);
    CPPUNIT_TEST(testValues);
    CPPUNIT_TEST(testHash);
    CPPUNIT_TEST(testGrowable);
    CPPUNIT_TEST(testPool);
    CPPUNIT_TEST(testRecovery);
    CPPUNIT_TEST(testEqualityCoerce);
    CPPUNIT_TEST(testAssignVar);
    CPPUNIT_TEST(testArithmetic);
    CPPUNIT_TEST(testStrings);
    CPPUNIT_TEST(testNativeFunctions);
    CPPUNIT_TEST(testUserFunctions);
    CPPUNIT_TEST(testConds);
    CPPUNIT_TEST(testAsserter);
    CPPUNIT_TEST(testNestedConds);
    CPPUNIT_TEST(testLoops);
    CPPUNIT_TEST(testGoto);
    CPPUNIT_TEST(testObjects);
    CPPUNIT_TEST(testDicts);
    CPPUNIT_TEST(testSessions);
    CPPUNIT_TEST(testSerialisation);
    CPPUNIT_TEST(testUserObjects);
    CPPUNIT_TEST(testListObjects);
    CPPUNIT_TEST_SUITE_END();
    
    
private:
    void _BOOLTEST(const char *test,bool equal,CppUnit::SourceLine line);
    void _INTVARTEST(const char *var,int val,CppUnit::SourceLine line);
    lana::API *api;
    lana::Session *ses;
    class AsserterHost *h;
public:
    void setUp();
    void tearDown();
    
    void testValues();
    void testHash();
    void testGrowable();
    void testGlobals();
    void testPool();
    void testGlobAssign();
    void testAssignVar();
    void testArithmetic();
    void testEqualityCoerce();
    void testNativeFunctions();
    void testUserFunctions();
    void testConds();
    void testRecovery();
    void testAsserter();
    void testNestedConds();
    void testLoops();
    void testGoto();
    void testStrings();
    void testDicts();
    void testObjects();
    void testSessions();
    void testSerialisation();
    void testUserObjects();
    void testListObjects();
};

inline void checkStrEqual(const char *a,
                          const char *b,
                          CppUnit::SourceLine line){
    if(!strcmp(a,b))return;
    ::CppUnit::Asserter::failNotEqual(a,b,line);
}

// asserts that two strings are equal

#define CPPUNIT_ASSERT_STREQUAL(a,b) \
	checkStrEqual(a,b,CPPUNIT_SOURCELINE())

// perform the expression in "test", assign the result to the global "t674"
// and assert that foo=equal. For example, 
//   CPPUNIT_ASSERT_BOOLTEST("4==2+2",true);
// will make Lana interpret "t674=(4==2+2)" and check that the value of t674 is
// true (i.e. 1)

#define CPPUNIT_ASSERT_BOOLTEST(test,equal) \
	_BOOLTEST(test,equal,CPPUNIT_SOURCELINE())

// assert that a variable equals a value
#define CPPUNIT_ASSERT_INTVAR(var,val) \
	_INTVARTEST(var,val,CPPUNIT_SOURCELINE())



// assert that this line asserts in lana using the internal asserter host 

#define CPPUNIT_ASSERT_ASSERTS(ses,line,efile,eline)  _ASSERTASSERTS(ses,line,efile,eline,CPPUNIT_SOURCELINE())

inline void _ASSERTASSERTS(lana::Session *ses,const char *line,
                           const char *expectedFile,
                           int expectedLine,
                           CppUnit::SourceLine n) {
    char buf[1024];
    try {
        ses->feed(line);
    } catch(AssertException& e){
        if(strcmp(e.myFile,expectedFile)){
            sprintf(buf,"assertion caught, but file wrong - expected %s, actual %s",expectedFile,e.myFile);
            ::CppUnit::Asserter::fail(buf,n);
        }
        else if(e.myLine != expectedLine){
            sprintf(buf,"assertion caught, but line wrong - expected %d, actual %d",expectedLine,e.myLine);
            ::CppUnit::Asserter::fail(buf,n);
        }
            
        return;
    }
    sprintf(buf,"assertion not caught in '%s'",line);
    ::CppUnit::Asserter::fail(buf,n);
}


#endif /* __TESTS_H */
