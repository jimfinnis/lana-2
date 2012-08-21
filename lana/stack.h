#ifndef __STACK_H
#define __STACK_H

/** @file
 * Stack classes SimpleStack and StackableStack, and their associated
 * exceptions
 */

#include "exception.h"

namespace lana {

/// the root stack exception
class StackException : public Exception {
public:
    StackException(const char *e) : Exception(e) {}
};

/// not enough items on the stack to pop()
class StackUnderflowException : public StackException {
public:
    StackUnderflowException() : StackException("stack underflow") {}
};

/// too many items on the stack to push()
class StackOverflowException : public StackException {
public:
    StackOverflowException() : StackException("stack overflow") {}
};

/// a class to encapsulate a simple stack of N items of type T, 
/// where items are copied into the stack - it is not a stack
/// of pointers.

template <class T,int N> class SimpleStack {
public:
    
    SimpleStack(){
        ct=0;
    }
    
    /// get an item from the top of the stack, discarding n items first.
    T pop(int n=0) {
        if(ct<=n)
            throw StackUnderflowException();
        ct -= n+1;
        return stack[ct];
    }
    
    /// get the nth item from the top of the stack
    T peek(int n=0) {
        if(ct<=n)
            throw StackUnderflowException();
        return stack[ct-(n+1)];
    }
    
    /// get a pointer to the nth item from the top of the stack
    T *peekptr(int n=0) {
        if(ct<=n)
            return NULL;
        return stack+(ct-(n+1));
    }
    
    /// get a pointer to an item from the top of the stack, discarding n items first,
    /// return NULL if there are not enough items instead of throwing an exception.
    T *popptrnoex(int n=0) {
        if(ct<=n)
            return NULL;
        ct -= n+1;
        return stack+ct;
    }
    
    /// get a pointer to an item from the top of the stack, discarding n items first
    T *popptr(int n=0) {
        if(ct<=n)
            throw StackUnderflowException();
        ct -= n+1;
        return stack+ct;
    }
    
    /// push an item onto the stack, returning the new item slot
    /// to be written into.
    T* pushptr() {
        if(ct==N)
            throw StackOverflowException();
        return stack+(ct++);
    }
    
    /// push an item onto the stack, passing the object in - consider
    /// using pushptr(), it might be quicker.
    void push(T o){
        if(ct==N)
            throw StackOverflowException();
        stack[ct++]=o;
    }
    
    /// swap the top two items on the stack
    void swap(){
        if(ct<2)
            throw StackUnderflowException();
        T x;
        x=stack[ct-1];
        stack[ct-1]=stack[ct-2];
        stack[ct-2]=x;
    }
    
    /// is the stack empty?
    bool isempty(){
        return ct==0;
    }
    
    /// empty the stack
    void clear() {
        ct=0;
    }
    
    /// left public for debugging handiness and stuff (such as VirtualMachine::cleanAndFlush())
    T stack[N];
    int ct;
};

/// a class to encapsulate a "stackable stack" - a stack of 16 stacks of
/// N items of type T.

template <class T,int N> class StackableStack {
public:
    StackableStack(){
        current = new SimpleStack<T,N>();
        ct=0;
    }
    
    ~StackableStack(){
        clearall();
        delete current;
    }
    
    /// clear the top stack
    void clearcurr(){
        current->clear();
    }
    
    /// call SimpleStack::pop() on the top stack
    T pop(int n=0) {
        return current->pop(n);
    }
    
    /// call SimpleStack::peek() on the top stack
    T peek(int n=0) {
        return current->peek(n);
    }
    
    /// call SimpleStack::peekptr() on the top stack
    T *peekptr(int n=0) {
        return current->peekptr(n);
    }
    
    /// call SimpleStack::popptr() on the top stack
    T *popptr(int n=0) {
        return current->popptr(n);
    }
    /// call SimpleStack::popptrnoex() on the top stack
    T *popptrnoex(int n=0) {
        return current->popptrnoex(n);
    }
    /// call SimpleStack::push() on the top stack
    void push(T o){
        current->push(o);
    }
    
    /// call SimpleStack::pushptr() on the top stack
    T *pushptr() {
        return current->pushptr();
    }
    
    /// clear all stacks and delete them
    void clearall(){
        while(ct)
            popstack();
        current->clear();
    }
    
    /// create a new stack and push it onto the stack-of-stacks
    void pushstack() {
        if(ct==16)
            throw StackOverflowException();
        stack[ct++]=current;
        current=new SimpleStack<T,N>();
    }
    
    /// delete the topmost stack and return to the stack below
    void popstack() {
        if(ct==0)
            throw StackUnderflowException();
        delete current;
        current = stack[--ct];
    }
    
    /// return the top stack
    SimpleStack<T,N> *getstack(){
        return current;
    }
    
private:
    SimpleStack<T,N> *stack[16],*current;
    int ct;
};     

}


#endif /* __STACK_H */
