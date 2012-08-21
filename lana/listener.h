#ifndef __LISTENER_H
#define __LISTENER_H

/**
 * @file
 * Code for an Observer/Observable implementation used to trap
 * internal lana events.
 */

namespace lana
{

/// these are the various lana events which can be listened for

enum event {
    COMMAND_PARSED,
    EVENT_COUNT
};

/// data passed to an event notification. The actual type depends
/// on the event.

union EventData {
    int i;
    Value *v;
    const char *s;
    instruction *op;
    
    EventData(int q){
        i=q;
    }
    EventData(Value *q){
        v=q;
    }
    EventData(const char *q){
        s=q;
    }
    EventData(instruction *q){
        op=q;
    }
};

/// this class is used to allow Lana to notify the user of events.
/// The user can create their own subclass of this ADT,
/// and pass it to the setListener() method with an event code.
/// The notification method will be called whenever the event occurs.
/// Several events can be set to the same object, and several listeners
/// can listen to the same event. Listeners cannot be unset.

class Listener {
    friend class EventMgr;
    class Listener *next;
public:
    virtual void notify(event e, EventData d) = 0;
};

/// this class handles sending messages to event listeners

class EventMgr {
private:
    Listener *listeners[EVENT_COUNT];
public:
    EventMgr(){
        for(int i=0;i<EVENT_COUNT;i++){
            listeners[i]=NULL;
        }
    }
            
    /// add a listener to the list for a given event
    
    void add(event e,Listener *p){
        p->next = listeners[e];
        listeners[e] = p;
    }
    
    /// call all the listeners for an event
    
    void notify(event e, EventData d){
        for(Listener *p=listeners[e];p;p=p->next)
            p->notify(e,d);
    }
};


}

#endif /* __LISTENER_H */
