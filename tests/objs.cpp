#include "tests.h"
#include "lana/object.h"

void TestFixtureLana::testObjects(){
    
//    api->setDebug(0xffffffff);
    
    ses->feed("create().foo=1");
    
    
    CPPUNIT_ASSERT_THROW(ses->feed("blart.create()"),lana::RuntimeException);
    
    ses->feedFile("files/simpleobj.l");
    ses->feedFile("files/simpleclone.l");
    ses->feedFile("files/usermethods.l");
}
