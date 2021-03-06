#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "language.h"
#include "object.h"
#include "cycle.h"

using namespace lana;


/** A description of the algorithm: 
 * - For each container object, set gc_refs equal to the object's reference count.
 * - For each container object, find which container objects it references and decrement the referenced container's gc_refs field.
 * - All container objects that now have a gc_refs field greater than one are referenced from outside the set of container objects. We cannot free these objects so we move them to a different set.
 * - Any objects referenced from the objects moved also cannot be freed. We move them and all the objects reachable from them too.
 * - Objects left in our original set are referenced only by objects within that set (ie. they are inaccessible from Python and are garbage). We can now go about freeing these objects.
 */
void CycleDetector::detect(){
    GarbageCollected *p,*q;
    
    newlist.reset(); // clean the new list
    
    for(p=mainlist.head();p;p=mainlist.next(p)){
        dfprintf("List ent : %lx (next %lx)\n",p,mainlist.next(p));
        p->gc_refs = p->refct;
    }
    
    // for each item, decrement the gc_refs of any items I point to.
    // You'll notice three calls doing the work - two are general purpose, assuming
    // the item can create value and key iterators. The other is for each object to extend,
    // and is used when a user subclass of GarbageCollected has non-Lana properties
    // which refer to GCable entities.
    
    for(p=mainlist.head();p;p=mainlist.next(p)) {
        decIteratorReferentsCycleRefCounts(p,true);
        decIteratorReferentsCycleRefCounts(p,false);
        p->decReferentsCycleRefCounts();
    }
        
    // now look for containers with gc_refs of greater than zero, and move them into a different list
    
    for(p=mainlist.head();p;p=q){
        q=mainlist.next(p); // so we can remove during traversal
        dfprintf("Refct : %x %d\n",p,p->gc_refs); 
        if(p->gc_refs>=1){
            dfprintf(" refct moving %x into new list\n",p);
            p->gc_refs=0; // this is the initial move, we need to do this to make sure move() will work!
            move(p);
        }
    }
    
    // only items with a gc_refs of zero will remain.
    
/*    
    dfprintf("Mainlist:\n");
    for(p=mainlist.head();p;p=mainlist.next(p))
        dfprintf("   %lx\n",p);
    dfprintf("Newlist:\n");
    for(p=newlist.head();p;p=newlist.next(p))
        dfprintf("   %lx\n",p);
*/    
    // trace objects which can be accessed from the objects we just moved, and move them too.
    // as we do this, we set gc_refs 1 to indicate it's been done. We would do this in the step
    // above, but we need the whole set's gc_refs to be zero so we can set it to 1 when things are
    // processed. We also make sure we don't move objects which already have a non-zero gc_refs
    // (move() does this).
    // Again, as well as the general-purpose calls which run through the value/key iterators,
    // we have a non-iterator version which is usually empty but can be overridden.
    
    for(p=newlist.head();p;p=q){
        q=newlist.next(p);        
        dfprintf("Entry : %lx  - next %lx\n",p,q);
        dfprintf("moving refs from %lx into new list\n",p);
	traceAndMoveIterator(p,true);
	traceAndMoveIterator(p,false);
	p->traceAndMove(this);
    }
    dfprintf("End of loop.\n");
    
    // now we iterate through the list and set all reference counts of
    // the objects we're about to delete to max.
    
    for(p=mainlist.head();p;p=mainlist.next(p)){
        dfprintf("maxreffing %lx\n",p);
        p->gc_refs=0xffff;
    }
    
    // then we do it again, and tell each of these objects to clear, without dereferencing,
    // all references to the objects we just marked - this is so that we don't delete them twice.
    // Again, as well as the general-purpose calls which run through the value/key iterators,
    // we have a non-iterator version which is usually empty but can be overridden.
    
    for(p=mainlist.head();p;p=mainlist.next(p)){
        clearZombieReferencesIterator(p,true);
        clearZombieReferencesIterator(p,false);
        p->clearZombieReferences();
    }
    
    for(p=mainlist.head();p;p=q){
        q=mainlist.next(p);
        dfprintf("%p is in a cycle  - deleting\n",p);
        delete p;
    }
    
    // and we set the main list to the new list
    
    mainlist.copy(newlist);
    
//    printf("Objects left:\n");
//    for(p=mainlist.head();p;p=mainlist.next(p)){
//        printf("  %x (ref %d)\n",(u32)p,p->refct);
//    }
//    printf("DONE\n");
}

void CycleDetector::decIteratorReferentsCycleRefCounts(GarbageCollected *gc, bool keys) {
    
    Iterator<Value *>* iterator = keys ? gc->createKeyIterator(true) : gc->createValueIterator();
    if(!iterator)return;
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        Value *v = iterator->current();
        if(v->getAllocType() == Complex){
            dfprintf("decrementing cyc %lx\n",v->d.gc);
            v->d.gc->gc_refs--;
        }
    }
    delete iterator;
}

void CycleDetector::traceAndMoveIterator(GarbageCollected *gc, bool keys) {
    
    Iterator<Value *>* iterator = keys ? gc->createKeyIterator(true) : gc->createValueIterator();
    if(!iterator)return;
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        Value *v = iterator->current();
        if(v->getAllocType() == Complex){
            GarbageCollected *g = v->d.gc;
            if(!g->gc_refs) { // if child not done
                move(g);
                traceAndMoveIterator(v->d.gc,keys);
                v->d.gc->traceAndMove(this);
            }
        }
    }
    delete iterator;
}

void CycleDetector::clearZombieReferencesIterator(GarbageCollected *gc, bool keys) {
    Iterator<Value *>* iterator = keys ? gc->createKeyIterator(true) : gc->createValueIterator();
    if(!iterator)return;
    
    for(iterator->first();!iterator->isDone();iterator->next()){
        Value *v = iterator->current();
        if(v->getAllocType() == Complex){
            GarbageCollected *g = v->d.gc;
            if(g->gc_refs == 0xffff) // if child not done
                v->initNone(); // clear without any reference count changes
        }
    }
    delete iterator;
}
