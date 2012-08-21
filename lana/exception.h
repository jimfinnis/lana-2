/**
 * @file
 * 
 * This file contains the core exception classes used by Lana,
 * all of which are derived from lana::Exception, which itself
 * is derived from std::exception in the STL.
 */

#ifndef __LANAEXCEPTION_H
#define __LANAEXCEPTION_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <exception>

namespace lana {

/// the root Lana exception

class Exception : public std::exception {
public:
    /// construct the exception. Note that
    /// space is allocated here, which is deleted
    /// by the destructor.
    Exception(const char *e){
        if(e)
            strncpy(error,e,1024);
        else
            strcpy(error,"???");
    }
    
    /// a variadic fluent modifier to set a better string with sprintf
    Exception& set(const char *s,...){
        va_list args;
        va_start(args,s);
        
        vsnprintf(error,1024,s,args);
        va_end(args);
        return *this;
    }
    
    /// default ctor
    Exception(){
        strcpy(error,"???");
    }
    
    /// construct the exception. Note that
    /// space is allocated here, which is deleted
    /// by the destructor. This version considers e to be
    /// a format string, and arg a string argument.
    Exception(const char *e,const char *arg){
        snprintf(error,1024,e,arg);
    }
    
    /// return the error string
    virtual const char *what () const throw (){
        return error;
    }
    
    /// a copy of the error string
    char error[1024];
};

/// errors in recreation
class RecreateException : public Exception {
public:
    RecreateException(const char *e) : Exception(e){}
} ;

/// this exception is thrown when the parser fails
class ParseException : public Exception {
public:
    ParseException(const char *e) : Exception(e){}
} ;

/// this exception is thrown when Growable fails, usually
/// when an out-of-range offset is requested
class GrowableException : public Exception {
public:
    GrowableException(const char *e) : Exception(e){}
} ;

/// this exception is thrown when a generic runtime error occurs

class RuntimeException : public Exception {
public:
    /// runtime exceptions are generated by VirtualMachine,
    /// which will often know the Lana filename and source line
    /// currently being executed. These are passed into the
    /// constructor.
    RuntimeException(const char *e,const char *fn,int l);
    
    /// the current Lana filename or NULL
    char fileName[1024];
    /// the current Lana line or -1
    int line;


};

}


#endif /* __LANAEXCEPTION_H */