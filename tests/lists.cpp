#include "tests.h"
#include "lana/arraylist.h"

void TestFixtureEmpty::testLists() {
    printf("raw list test removed");
}

void TestFixtureLana::testListObjects(){
    ses->feedFile("files/lists.l");
}
