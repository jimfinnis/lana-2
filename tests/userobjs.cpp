#include "tests.h"
#include "userobjs.h"

int TestObject::testPropertyID;
int TestObject::linkPropertyID;

void TestFixtureLana::testUserObjects(){
    
    // OK, now let's clear everything 
    delete ses;
    delete api;
    api = new lana::API();
    AsserterHost *a = new AsserterHost(api);
    ses = new lana::Session(api);



    TestObject::reg(api);
    ses->feedFile("files/userobjs.l");

    
    // and clear again

    delete a;
    delete ses;
    delete api;
    api = new lana::API();
    ses = new lana::Session(api);
}
