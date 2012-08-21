#include "tests.h"
#include "lana/growable.h"

void TestFixtureLana::testGrowable(){
    lana::Growable *g = new lana::Growable(32,32,4);
    
    int offset = g->allocate(100);
    char *s;
    s = (char *)g->get(0,0);
    s = (char *)g->get(0,100);
    
    try{
        char *mem = (char *)g->get(0,200);
        CPPUNIT_FAIL("exception expected");
    } catch(lana::GrowableException &e){
        CPPUNIT_ASSERT_STREQUAL("bad offset requested in growable",e.what());
    }
    
    delete g;
    
    
}
