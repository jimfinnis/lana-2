a=dict()
print("foo")
assertInt(0,size(a))
print("bar")

a["hello"]="blart"
a[3]="boo"

assertStr("blart",a["hello"])
assertStr("boo",a[3])

a[7]=3
assertInt(3,a[7])
a[7]=a[3]
assertStr("boo",a[7])

assertInt(3,size(a))
assert(del(a["hello"]))
assert(!del(a[1021]))
assertInt(2,size(a))

b=dict()

f = procedure()
    i=0
    while i<10
        a[i]=i*i        
        b[str(i+100)]=a[i]
        i=i+1
    endwhile

    i=values(a)    
    i.first()
    ct=0
    while !i.isDone()
#        print(str(ct*ct)+"="+str(i.current()))
        assertInt(ct*ct,i.current())
        i.next()
        ct=ct+1
    endwhile
    assertInt(10,ct)
    
    #for each over values implicit
    ct=0
    for i in a
        assertInt(ct*ct,i)
        ct=ct+1
    endfor
    assertInt(10,ct)

    #for each over values explicit
    ct=0
    for i in values(a)
        assertInt(ct*ct,i)
        ct=ct+1
    endfor
    assertInt(10,ct)
    
    #for each over keys
    ct=0
    for i in keys(a)
        assertInt(i*i,a[i])
        ct=ct+1
    endfor
    assertInt(10,ct)
        
end

f()

# quick GC check
oldGC=gc()
a=0
b=0
assertInt(oldGC-2,gc())


# GC of cycles involving dicts through both keys and values

oldGC=gc()
a=dict()
a[2]="hello"
b=create()
a["object"]=b
b.dict = a  # created a cycle!
a=0
b=0
assertInt(oldGC,gc())

a=dict()
a[2]="hello"
b=create()
a[b]=8 # reference through key
assertInt(8,a[b])
b.dict = a  # created a cycle!
a=0
b=0
assertInt(oldGC,gc())

# more iterator tests

# checking empty dict
a=dict()
i=values(a)
i.first()
assert(i.isDone())

# test of string const / string keys

a["helloworld"]=3
assertInt(3,a["hello"+"world"])
a["hello"+"world"]=3
assertInt(3,a["helloworld"])

a=0
i=0 # remember to do this!
assertInt(oldGC,gc())


# now to test cloning

a=dict()
a["hello"]=100
a["fish"]="moon"
b=clone(a)

assertInt(100,b["hello"])
assertInt(100,a["hello"])
assertStr("moon",a["fish"])
assertStr("moon",b["fish"])

#subsequent adds to a won't be reflected in b

a["cat"]="help"
assertStr("help",a["cat"])
assert(!defined(b["cat"]))

#can change b without changing a

b["fish"]="carthorse"
assertStr("moon",a["fish"])
assertStr("carthorse",b["fish"])

# and garbage will be collected despite the inheritance
# and more than one crosslink

a["foon"]=b
b["foon"]=a
b[1]=a
a=0
b=0
assertInt(oldGC,gc())


