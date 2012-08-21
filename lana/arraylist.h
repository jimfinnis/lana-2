#ifndef __ARRAYLIST_H
#define __ARRAYLIST_H

/** @file
 * ArrayList, An array list implementation
 */

namespace lana {


/// array list exception - typically get out of range or pop on empty list
class ArrayListException : public Exception {
public:
    ArrayListException(const char *e) : Exception(e) {}
};

/// array list with random access get at  O(1). Insertion at O(n)
/// except at the end of the list where it's O(1). List will occasionally
/// resize, more often on grow than on shrink. Docs need improving :)

class ArrayList {
public:
    /// create a list, with initially enough room for n elements
    ArrayList(int n){
        capacity = n;
        ct = 0;
        data = new Value [n];
    }
    
    /// destroy a list
    ~ArrayList() {
        delete [] data;
    }
    
    /// add an item to the end of the list, return a pointer to
    /// fill in the item. Runs in O(1) time unless the list needs
    /// resizing.
    Value *append(){
        reallocateifrequired(ct+1);
        return data+(ct++);
    }
    
    /// insert an item before position n, returning a pointer to
    /// fill in the item. Runs in O(n) time unless n==-1, in which
    /// case it's O(1)
    Value *insert(int n=-1){
        if(n<0 || n>=ct)
            return append();
        reallocateifrequired(ct+1);
        memmove(data+n+1,data+n,(ct-n)*sizeof(Value));
        ct++;
        return data+n;
    }
    
    /// remove an item from the list, runs in O(1) time, unless there's a resize
    Value *pop(){
        if(!ct) throw ArrayListException("pop on empty list");
        // make DAMN SURE we still have the old item!
        reallocateifrequired(ct-1);
        ct--;
        return data+ct;
    }
    
    /// peek an item from the list, runs in O(1) time
    Value *peek(){
        if(!ct) throw ArrayListException("peek on empty list");
        // make DAMN SURE we still have the old item!
        return data+(ct-1);
    }
    
    
    /// remove an item from somewhere in the list in O(n) time
    bool remove(int n=-1){
        if(n<0||n>=ct)
            return false;
        data[n].clr();
        reallocateifrequired(ct-1);
        ct--;
        if(n>=0 && n!=ct)
            memmove(data+n,data+n+1,(ct-n)*sizeof(Value));
        return true;
    }
    
    /// get a pointer to the nth item of a list in O(1) time. If n==-1
    /// will return the last item.
    Value *get(int n){
        if(n<0||n>=ct)
            throw ArrayListException("get out of range");
        return data+n;
    }
    
    /// return the size of the list
    int count(){
        return ct;
    }
    
    /// return the capacity of the list
    int getCapacity(){
        return capacity;
    }
    
    /// set a value in the list
    void set(int n,Value *v){
        if(n>=ct){
            reallocateifrequired(n+1);
            ct=n+1;
            
            for(int i=ct;i<n;i++)
                data[i].initNone();
        }
        data[n].clr();
        data[n]=*v;
    }
    
    Iterator<Value *> *createKeyIterator();
    
    Iterator<Value *> *createValueIterator();
private:
    
    /// reallocate the list if required by the given new count and copy
    /// all items over. Will NOT change ct.
    void reallocateifrequired(int newct){
        Value *newdata;
//        printf("ct %d, cap %d\n",newct,capacity);
        if(newct>=capacity){
            // need to grow the list
            capacity += (ct>>3) + (ct<9?3:6);
//            printf("GROW\n");
        } else if(capacity>16 && newct<(capacity>>1)) {
            // need to shrink the list. New capacity should still
            // have at least one empty space left at the end, for popped
            // items!
            capacity = capacity>>1;
//            printf("SHRINK\n");
        } else
            return;
        
        // do the resize
        newdata = new Value [capacity];
        memcpy(newdata,data,sizeof(Value)*ct);
        delete [] data;
        data = newdata;

    }
    
    /// the data area
    Value *data;
    /// the number of items currently stored in the list
    int ct;
    /// the capacity of the list
    int capacity;
    
};

class ArrayListIterator : public Iterator<Value *> {
public:
    ArrayListIterator(ArrayList *a){
        idx=-1;
        list = a;
    }
    
    virtual void first(){
        idx = 0;
    }
    virtual void next(){
        idx++;
    }
    virtual bool isDone() const {
        return idx>=list->count();
    }
    
    virtual Value *current() = 0;

protected:
    int idx;
    ArrayList *list;
};

/// array list key iterator - don't create directly. 

class ArrayListKeyIterator : public ArrayListIterator {
public:
    ArrayListKeyIterator(ArrayList *a) : ArrayListIterator(a){}
    
    virtual Value *current() {
        if(idx<0)
            throw Exception("first() not called on iterator");
        if(isDone())
            throw Exception("list iterator done");
        v.setInt(idx);
        return &v;
    }
private:
    Value v;
};

/// array list value iterator - don't create directly. 

class ArrayListValueIterator : public ArrayListIterator {
public:
    ArrayListValueIterator(ArrayList *a) : ArrayListIterator(a){}
    
    virtual Value *current() {
        if(idx<0)
            throw Exception("first() not called on iterator");
        if(isDone())
            throw Exception("list iterator done");
        return list->get(idx);
    }
private:
    Value v;
};


inline Iterator<Value *> *ArrayList::createKeyIterator() {
    return new ArrayListKeyIterator(this);
};

inline Iterator<Value *> *ArrayList::createValueIterator() {
    return new ArrayListValueIterator(this);
};



}


#endif /* __ARRAYLIST_H */
