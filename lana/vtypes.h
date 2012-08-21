/**
 * @file
 * This file contains definitions of the Type subclasses (Type itself is in value.h)
 *
 */

#ifndef __VTYPES_H
#define __VTYPES_H

namespace lana {

/// a type for simple hashables, whose hash is their d value as an integer.
struct SimpleHashableType : public Type {
    virtual u32 getHash(const Value *v) const {
        return v->d.u;
    }
};

/// functions - d.u is a descriptor to a constant holding a pointer to 
/// the function. Ugly, but the only way we can do literals with our bytecode.

struct FunctionType : public SimpleHashableType {
    virtual char *getPtr(const Value *v) const;
    virtual void serialise(class Serialiser *a,FILE *out,Value *v) const;
};

/// a native function
struct NativeFunctionType : public SimpleHashableType {
    virtual void serialise(class Serialiser *a,FILE *out,Value *v) const;
};

/// the string type - string constants are a subclass of this

struct StringType : public Type {
    virtual u32 getHash(const Value *v) const;
    
    virtual char *getStr(const Value *v) const {
        return v->d.s+sizeof(refct_t); // note NO CLONING!
    }
    
    virtual int getInt(const Value *v) const {
        return atoi(v->d.s+sizeof(refct_t));
    }
    virtual float getFloat(const Value *v) const {
        return atof(v->d.s+sizeof(refct_t));
    }
    virtual const char *repr(const Value *v) const {
        startRepr();
        return buf;
    }        
    virtual void serialise(class Serialiser *a,FILE *out,Value *v) const;
    
    virtual void doBinArithOp(Value *out,int op,Value *lhs,Value *rhs){
        if(rhs->type == Types::vtInteger && op==OP_MUL){
            // string * int = repeat
            char *p = lhs->getStr();
            int n = rhs->getInt();
            // string repeats, plus terminator, plus refct.
            char *q = (char *)malloc(strlen(p)*n+1+sizeof(refct_t));
            // init refct and terminate string
            *(refct_t*)q = 1;
            *(q+sizeof(refct_t))=0;
            for(int i=0;i<n;i++)
                strcat(q+sizeof(refct_t),p);
            out->setOther(Types::vtString,q);
        } else if((rhs->type == Types::vtString ||rhs->type == Types::vtStringConst)&& op==OP_ADD) {
            // string + string = concat
            char *p = lhs->getStr();
            char *q = rhs->getStr();
            char *t = (char *)malloc(strlen(p)+strlen(q)+1+sizeof(refct_t));
            // init refct
            *(refct_t*)t = 1;
            char *tt = t+sizeof(refct_t);
            strcpy(tt,p);
            strcat(tt,q);
            out->setOther(Types::vtString,t);
        } else {
            float a = lhs->getFloat();
            float b = rhs->getFloat();
            float r;
            
            switch(op){
            case OP_MUL:r = a*b;break;
            case OP_ADD:r = a+b;break;
            case OP_SUB:r = a-b;break;
            case OP_DIV:r = a/b;break;
            default:
                throw Exception("operation not permitted on strings");
            }
            out->setFloat(r);
        }
    }
    
    /// for "size"
    
    virtual bool makePropRef(Value *v,Value *item,u32 prop);
    
    virtual void doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs);
};

/// string constants

struct StringConstType : public StringType {
    virtual char *getStr(const Value *v) const;
    virtual char *getPtr(const Value *v) const;
    virtual int getInt(const Value *v) const;
    virtual float getFloat(const Value *v) const;
    virtual const char *repr(const Value *v) const {
        startRepr();
        return buf;
    }        
};

/// the integer type definition
struct IntegerType : public SimpleHashableType {
    virtual bool isNumeric(){
        return true;
    }
    virtual char *getStr(const Value *v) const {
        sprintf(buf,"%d",v->d.i);
        return buf;
    }
    virtual int getInt(const Value *v) const {
        return v->d.i;
    }
    virtual float getFloat(const Value *v) const {
        return (float)v->d.i;
    }
    virtual const char *repr(const Value *v) const {
        startRepr();
        sprintf(buf+strlen(buf),"%d",v->d.i);
        return buf;
    }        
    virtual bool negate(Value *in,Value *out){
        out->setInt(-in->d.i);
        return true;
    }
    virtual bool unarynot(Value *in,Value *out){
        out->setInt(!in->d.i);
        return true;
    }
    
    virtual void doBinArithOp(Value *out,int op,Value *lhs,Value *rhs){
        if(rhs->type == Types::vtFloat){
            float a = (float)lhs->d.i;
            float b = rhs->getFloat();
            float r;
            switch(op){
            case OP_MUL:r = a*b;break;
            case OP_ADD:r = a+b;break;
            case OP_SUB:r = a-b;break;
            case OP_DIV:r = a/b;break;
            default:
                throw Exception("operation not permitted on integers");
            }
            out->setFloat(r);
        } else {
            int a = lhs->d.i;
            int b = rhs->getInt();
            int r;
            switch(op){
            case OP_MOD:r = a%b;break;
            case OP_MUL:r = a*b;break;
            case OP_ADD:r = a+b;break;
            case OP_SUB:r = a-b;break;
            case OP_DIV:r = a/b;break;
            default:
                throw Exception("operation not permitted on integers");
            }
            out->setInt(r);
        }
    }
    virtual void doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs);
        
};

/// the float type definition

struct FloatType : public SimpleHashableType {
    virtual bool isNumeric(){
        return true;
    }
    virtual char *getStr(const Value *v) const {
        sprintf(buf,"%f",v->d.f);
        return buf;
    }
    virtual int getInt(const Value *v) const {
        return (int)v->d.f;
    }
    virtual float getFloat(const Value *v) const {
        return v->d.f;
    }
    virtual const char *repr(const Value *v) const {
        startRepr();
        sprintf(buf+strlen(buf),"%s",v->d.i?"true":"false");
        return buf;
    } 
    virtual bool negate(Value *in,Value *out){
        out->setFloat(-in->d.f);
        return true;
    }
    virtual bool unarynot(Value *in,Value *out){
        out->setInt(!(int)in->d.f);
        return true;
    }
    
    virtual void doBinArithOp(Value *out,int op,Value *lhs,Value *rhs){
        float a = lhs->d.f;
        float b = rhs->getFloat();
        float r;
        switch(op){
        case OP_MUL:r = a*b;break;
        case OP_ADD:r = a+b;break;
        case OP_SUB:r = a-b;break;
        case OP_DIV:r = a/b;break;
        default:
            throw Exception("operation not permitted on floats");
        }
        out->setFloat(r);
    }
    virtual void doBinComparisonOp(Value *out,int op,Value *lhs,Value *rhs);
    
};


/// the type for references to variables
struct RefType : public Type {
    virtual Value *deref(Value *v){
        return (Value *)v->d.s;
    }
    virtual void store(Value *ref,Value *v){
        Value *dst = (Value *)ref->d.s;
        *dst = *v;
    }
};

/// the type for boolean values
struct BooleanType : public SimpleHashableType {
    virtual bool getBool(const Value *v) const {
        return v->d.i?true:false;
    }
    virtual char *getStr(const Value *v) const {
        sprintf(buf,"%s",v->d.i?"true":"false");
        return buf;
    }
    virtual const char *repr(const Value *v) const {
        startRepr();
        sprintf(buf+strlen(buf),"%s",v->d.i?"true":"false");
        return buf;
    } 
    virtual bool unarynot(Value *in,Value *out){
        out->setBool(!in->d.i);
        return true;
    }
};

/// a superclass for garbage collected, iterable objects

struct IterableType : public SimpleHashableType {
    virtual const char *repr(const Value *v) const {
        startRepr();
        sprintf(buf+strlen(buf),"%x/(ct%d)",(u32)v->d.gc,v->d.gc->refct);
        return buf;
    } 
    virtual Iterator<Value *> *createIter(Value *v,
                                          bool keys) {
        return keys ? v->d.gc->createKeyIterator(false) : v->d.gc->createValueIterator();
    }
    
    /// this is used for special properties of iterable objects - "keys" and "values" typically.
    
    virtual bool makePropRef(Value *v,Value *item,u32 prop);
    
    /// used when we're using as, say, a key - not in an assignment
    virtual void serialise(Serialiser *s,FILE *out,Value *v) const;
};

}


#endif /* __VTYPES_H */
