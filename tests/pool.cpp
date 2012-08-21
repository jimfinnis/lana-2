#include "tests.h"
#include "lana/pool.h"
void TestFixtureLana::testPool(){
    lana::Pool<int,4> pool;
    int d[32];
    
    d[0]=pool.alloc();
    pool.free(d[0]);
    
    for(int i=0;i<32;i++){
//        printf("alloc\n");
        d[i]=pool.alloc();
    }
    
    pool.free(d[0]);
    pool.free(d[12]);
    pool.free(d[4]);
    pool.free(d[6]);
    d[12]=pool.alloc();
    d[4]=pool.alloc();
    pool.free(d[8]);
    pool.free(d[10]);
    pool.free(d[12]);
    CPPUNIT_ASSERT_THROW(pool.free(d[12]),lana::PoolException);
}
