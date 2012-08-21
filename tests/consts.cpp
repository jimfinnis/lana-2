#include "tests.h"
#include "lana/language.h"

// this is the value of the first constant after the standard ones
// added by the special properties

// we iterate through this structure, looking for the appropriate
// constant and creating it if it doesn't exist. The number
// on the left is the descriptor index we expect to get back
// from the constant system if the system is working, and reused
// constants are being found.

struct consttest {
    int expectedDesc;
    int type;
    struct {
        int i;
        float f;
        const char *s;
    } d;
} consttests[]=
{
    0,CT_INT,{42,0,0},
    2,CT_FLOAT,{0,3.4f,0},
    4,CT_FLOAT,{0,41.0f,0},
    6,CT_INT,{-1,0,0},
    8,CT_INT,{0,0,0},
    10,CT_STRING,{0,0,"hello"},
    13,CT_FLOAT,{0,9.4f,0},
    15,CT_FLOAT,{0,43.0f,0},
    17,CT_STRING,{0,0,"there world"},
    8,CT_INT,{0,0,0},
    
    21,CT_INT,{1,0,0},
    13,CT_FLOAT,{0,9.4f,0},
    15,CT_FLOAT,{0,43.0f,0},
    13,CT_FLOAT,{0,9.4f,0},
    15,CT_FLOAT,{0,43.0f,0},
    10,CT_STRING,{0,0,"hello"},
    23,CT_STRING,{0,0,"hello world"},
    27,CT_FLOAT,{0,0.0f,0},
    17,CT_STRING,{0,0,"there world"},
    27,CT_FLOAT,{0,0.0f,0},
    2,CT_FLOAT,{0,3.4f,0},
    4,CT_FLOAT,{0,41.0f,0},
    
    0,CT_INT,{42,0,0},
    8,CT_INT,{0,0,0},
    10,CT_STRING,{0,0,"hello"},
    17,CT_STRING,{0,0,"there world"},
    8,CT_INT,{0,0,0},
    21,CT_INT,{1,0,0},
    13,CT_FLOAT,{0,9.4f,0},
    15,CT_FLOAT,{0,43.0f,0},
    10,CT_STRING,{0,0,"hello"},
    23,CT_STRING,{0,0,"hello world"},
    
    27,CT_FLOAT,{0,0.0f,0},
    17,CT_STRING,{0,0,"there world"},
    13,CT_FLOAT,{0,9.4f,0},
    15,CT_FLOAT,{0,43.0f,0},
    13,CT_FLOAT,{0,9.4f,0},
    15,CT_FLOAT,{0,43.0f,0},
    2,CT_FLOAT,{0,3.4f,0},
    4,CT_FLOAT,{0,41.0f,0},
    6,CT_INT,{-1,0,0},
    27,CT_FLOAT,{0,0.0f,0},
    2,CT_FLOAT,{0,3.4f,0},
    4,CT_FLOAT,{0,41.0f,0},
    -1,0,0
};
    
void TestFixtureEmpty::testConstants() {
    Constants c;
    int d;
    consttest *t = consttests;
    int i=0;
    
    // get a base
    int basedesc = c.findOrCreateInt(-9999) + 2; // sizeof(int)/2
    
    
    for(;;t++){
        if(t->expectedDesc<0)break;
        switch(t->type){
        case CT_INT:
            d = c.findOrCreateInt(t->d.i);
            break;
        case CT_FLOAT:
            d = c.findOrCreateFloat(t->d.f);
            break;
        case CT_STRING:
            d = c.findOrCreateString(t->d.s);
            break;
        }
//        printf("added %d\n",d);
    }
    
    i=0;
    for(t = consttests;;t++){
        if(t->expectedDesc<0)break;
        int expDesc = t->expectedDesc + basedesc; // for all the std consts
        printf("processing test %d/%d\n",i++,expDesc);
        
        ConstDesc *e = c.get(expDesc);
        CPPUNIT_ASSERT_EQUAL((int)t->type,(int)e->getType());
        void *ptr = e->get();
        
        switch(t->type){
        case CT_INT:
            CPPUNIT_ASSERT_EQUAL(t->d.i, *(int *)ptr);
            break;
        case CT_FLOAT:
            CPPUNIT_ASSERT_EQUAL(t->d.f, *(float *)ptr);
            break;
        case CT_STRING:
            CPPUNIT_ASSERT(!strcmp((const char *)ptr,t->d.s));
            break;
        }
    }
}
