#include "tests.h"

const char *prog1[]={
    "$Q=4",
    "f = function(a,b)",
    "   return a+b",
    "end",
    "assert(f(3,4)==7)",
    NULL
};

const char *prog2[]={
    "k=0",
    "f = procedure(a)",
    "   k=100*a",
    "end",
    "assert($Q==4)",
    "f(2)",
    "assert(k==200)",
    NULL
};

const char *prog3[]={
    "f=5",
    "k = function(a)",
    "   return 10*a+f",
    "end",
    "assert(k(2)==25)",
    "assert($Q==4)",
    NULL
};




void TestFixtureLana::testSessions(){
    // create three sessions
    
    lana::Session s1(api);
    lana::Session s2(api);
    lana::Session s3(api);
    
    ses->feedFile("files/strings.l");
    
    // set up three simple programs, and feed them to the sessions
    // in an interleaved way. They should not interfere with each
    // other because their session variables are private.
    
    int ct1=0,ct2=0,ct3=0;
    for(int i=0;i<20;i++){
        if(prog1[ct1])
            s1.feed(prog1[ct1++]);
        if(prog2[ct2])
            s2.feed(prog2[ct2++]);
        if(prog3[ct3])
            s3.feed(prog3[ct3++]);
    }
}


