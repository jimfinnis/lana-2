#ifndef __SER_H
#define __SER_H

namespace lana {

/// The serialiser, which writes the contents of a Lana system out to
/// a stream. Session variables will not be output unless there are
/// links to the values contained in them from globals.

class Serialiser {
public:
    /// create a serialiser from a session
    Serialiser(class Session *s);
    ~Serialiser();
    
    /// output all the globals to a file.
    void write(FILE *out);
    
    /// return a string of spaces for the current indent level
    const char *indents();
    
    
    class Session *session;
    class API *api;
    class Language *lana;
    class Constants *consts;
    
    /// used to store data referred to by different names. The values
    /// are offsets into the string store.
    IntKeyedHash<u32> hash;
    /// used to store names for the hash.
    Growable *stringStore;
    
    /// this is called by the serialiser to serialise each global. It's also called
    /// by the serialisation methods of things like objects and dictionaries to serialise
    /// their subvalues. It
    /// - checks if the value v should be hashed. If so, it checks to see if it's already
    /// in the hash and uses the name stored there if it is.
    /// - If it's not in the hash, it serialises the value with the appropriate serialisation
    /// method and adds it to the hash.
    /// - If it's not a hashable value it just serialises it.
    
    void serialiseValue(FILE *out, Value *v, const char *name);
    
    /// this is uses to associate a value with a name. It should only be called on complex
    /// objects which we want to define once only - typically a compound like an object or
    /// dictionary. The key is the d.u field of a value.
    void setHash(u32 key,const char *nam);
    
    /// this gets a value in the hash, or NULL if not there
    const char *getHash(u32 key);
    
    /// general-purpose "preserialiser" - call this with a hashable value
    /// and the serialiser will generate an assignment if necessary, and
    /// return the name under which it is hashed. <b>Only call this for values
    /// for which Type::serHash would be true</b>.
    /// NOTE - the returned value MUST BE FREED AFTER USE.
    
    const char *preSerialise(FILE *out, u32 target,Type *tp);
};

}

#endif /* __SER_H */
