#include "tests.h"
#include "lana/language.h"

void TestFixtureLana::testValues() {
    Value v;
    // smoke tests and conversions
    
    v.setInt(45);
    CPPUNIT_ASSERT_EQUAL(45,v.getInt());
    CPPUNIT_ASSERT_EQUAL(45.0f,v.getFloat());
    CPPUNIT_ASSERT_STREQUAL("45",v.getStr());
    v.setFloat(45.0f);
    CPPUNIT_ASSERT_EQUAL(45.0f,v.getFloat());
    CPPUNIT_ASSERT_EQUAL(45,v.getInt());
    CPPUNIT_ASSERT_STREQUAL("45.000000",v.getStr());
    v.setStrClone("hello");
    CPPUNIT_ASSERT_STREQUAL("hello",v.getStr());
    // nonnumeric strings evaluate to zero
    CPPUNIT_ASSERT_EQUAL(0,v.getInt());
    CPPUNIT_ASSERT_EQUAL(0.0f,v.getFloat());
    
    // check that weird types show the correct hex debug value
    v.setOther(Types::vtLDT,(void *)0x1234);
    CPPUNIT_ASSERT_STREQUAL("LDT:00001234",v.getStr());
    
    // check string to float and integer conversion
    // in more detail
    
    v.setStrClone("45.9");
    CPPUNIT_ASSERT_EQUAL(45.9f,v.getFloat());
    CPPUNIT_ASSERT_EQUAL(45,v.getInt());
    
    v.setStrClone("45");
    CPPUNIT_ASSERT_EQUAL(45,v.getInt());
    CPPUNIT_ASSERT_EQUAL(45.0f,v.getFloat());
    
    v.setStrClone("-45.9");
    CPPUNIT_ASSERT_EQUAL(-45.9f,v.getFloat());
    CPPUNIT_ASSERT_EQUAL(-45,v.getInt());
    
    // check string to float and integer conversion
    // with rounding
    
    v.setFloat(-45.5993485f);
    CPPUNIT_ASSERT_STREQUAL("-45.599350",v.getStr());
    v.setFloat(-45.5121200f);
    CPPUNIT_ASSERT_STREQUAL("-45.512119",v.getStr());
    v.setFloat(45.5993485f);
    CPPUNIT_ASSERT_STREQUAL("45.599350",v.getStr());
    v.setFloat(45.5121200f);
    CPPUNIT_ASSERT_STREQUAL("45.512119",v.getStr()); //hmm..
    
    char *foo = strdup("hello");
    v.setStrClone(foo);
    foo[0]='w';
    CPPUNIT_ASSERT_STREQUAL("hello",v.getStr());
    
    // but NOT copied on read
    
    char *bar = (char *)v.getStr();
    bar[0] = 'w';
    CPPUNIT_ASSERT_STREQUAL("wello",v.getStr());
    free(foo);
}
