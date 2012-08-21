#include "tests.h"
#include "lana/hash.h"
#include "lana/value.h"


lana::Hash *hash;

static void setIntByStr(const char *s,int n){
    lana::Value k;
    k.setStrClone(s);
    lana::Value v;
    v.setInt(n);
    
    hash->set(&k,&v);
}

static int getIntByStr(const char *s){
    lana::Value k;
    k.setStrClone(s);
    
    if(hash->find(&k))
        return hash->getval()->d.i;
    else
        return -9999;
}

static void setIntByInt(int s,int n){
    lana::Value k;
    k.setInt(s);
    
    lana::Value v;
    v.setInt(n);
    hash->set(&k,&v);
}

static int getIntByInt(int s){
    lana::Value k;
    k.setInt(s);
    
    if(hash->find(&k))
        return hash->getval()->d.i;
    else
        return -9999;
}

static void del(const char *s){
    lana::Value k;
    k.setStrClone(s);
    hash->del(&k);
}
static void del(int s){
    lana::Value k;
    k.setInt(s);
    hash->del(&k);
}

void TestFixtureLana::testHash(){
    hash = new lana::Hash;
    
    CPPUNIT_ASSERT_EQUAL(-9999,getIntByStr("foo"));
    setIntByStr("foo",20);
    setIntByStr("bar",32);
    CPPUNIT_ASSERT_EQUAL(20,getIntByStr("foo"));
    CPPUNIT_ASSERT_EQUAL(32,getIntByStr("bar"));
    setIntByStr("bar",48);
    CPPUNIT_ASSERT_EQUAL(48,getIntByStr("bar"));
    
    del("bar");
    CPPUNIT_ASSERT_EQUAL(-9999,getIntByStr("bar"));
    
    setIntByStr("bar",47);
    CPPUNIT_ASSERT_EQUAL(47,getIntByStr("bar"));
    
    for(int i=0;i<1000;i++)
        setIntByInt(i,i*3+31);
    
    for(int i=0;i<1000;i++){
        int q = getIntByInt(i);
        CPPUNIT_ASSERT_EQUAL(i*3+31,q);
    }
        
    for(int i=20;i<1000;i++){
        del(i);
    }
    
    for(int i=0;i<100;i++){
        int q = getIntByInt(i);
        CPPUNIT_ASSERT_EQUAL(i<20?i*3+31:-9999,q);
    }
    setIntByInt(2000,2);
        
    
    delete hash;
}
