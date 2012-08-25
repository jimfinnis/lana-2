#include "tests.h"

void TestFixtureLana::testSerialisation(){
    
    // test invalidity of save/load in functions
    ses->feed("sark = procedure()");
    CPPUNIT_ASSERT_THROW(ses->feed("save \"\""),lana::ParseException);
    CPPUNIT_ASSERT_THROW(ses->feed("load \"\""),lana::ParseException);
    ses->feed("end");
    
    
    // OK, now let's clear everything 
    delete ses;
    delete api;
    api = new lana::API();
    ses = new lana::Session(api);
//    api->setDebug(0xfff);
    
    // and create some stuff.
    
    ses->feedFile("files/testser1.l");
    // make a tree
    ses->feed("$spoon=make()");
    // save it
    ses->feed("savevar $spoon \"tmp1\"");
    printf("Saved data\n");
    // now delete the spoon global
    ses->feed("$spoon=0");
    ses->feed("gc()");
    printf("Deleted old copy\n");
    // and load in the one we just saved
    ses->feed("load \"tmp1\"");
    printf("Loaded data, comparing with new tree...\n");
    // now compare that with a fresh tree, making sure they're equal
    CPPUNIT_ASSERT_BOOLTEST("make().equal($spoon)",true);
    
    
    
}
