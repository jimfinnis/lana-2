#
# Simple objects and properties
#

oldGC=gc()
obj = create()
obj.foo = 3
obj.bar = "hello "
obj.foo = obj.bar+str(obj.foo)
assertStr("hello hello 3",obj.bar+str(obj.foo))

zog=create()
zog.a = "world"
assertStr("hello world",obj.bar+zog.a)
zog.b = obj
assertStr("hello hello 3",zog.b.bar+str(zog.b.foo))
obj.a = zog
assertStr("hello ",obj.a.b.a.b.a.b.a.b.a.b.a.b.a.b.a.b.bar)
obj.a.b.a.b.a.b.a.b.a.b.a.b.a.b.a.b.bar="pencil"
assertStr("pencil",obj.a.b.a.b.a.b.a.b.a.b.a.b.a.b.a.b.bar)

# delete references, leaving cycles
zog=0
obj=0

# there should be cycles until we gc.
assertInt(oldGC+2,gccount()) # two extra items
assertInt(oldGC,gc())

# bunch of objects all linked to each other
a=create()
b=create()
c=create()
d=create()
a.x = b
b.x = c
c.x = d
d.x = a
# make sure using Valgrind that this is cleared too
b.foo = "fish"+"head" 

e=create()
e.foo="bloogle"+"blark"
a.y=e

oldGC=gc()

# clear all except for e, which should survive, not being in the cycle;
a=0
b=0
c=0
d=0
assertInt(oldGC-4,gc())
e=0
assertInt(oldGC-5,gc())

# object iterators. Testing this is a little messy.
# Also serves as a test of symbol quoting

ids = dict()
ids[0] = `foo
ids[1] = `bar
ids[2] = `baz
ids[3] = `wibble
ids[4] = `impartial
ids[5] = `submarine
ids[6] = `trouser

f = procedure()
    a=create()
    for i in range(0,size(ids))
        a[ids[i]] = str(i)+"oof"
    endfor
    
    # check value count
    ct=0
    for i in a
        ct=ct+1
    endfor
    assertInt(ct,size(ids))

    # now iterate keys
    ct=0
    for i in keys(a)
        # lookup which element in the dict has the required key
        found=-1
        for j in keys(ids)
            if ids[j]==i
                found=j
                break
            endif
        endfor
        if found==-1: assertFail("bad reverse key lookup")
        # and check that the value matches that index
        assertStr(str(found)+"oof",a[i])
        ct=ct+1
    endfor
    assertInt(ct,size(ids))
end

f()


# create an object and some props, and test deletion and size

a = create()
a.foo=1
a.bar=2
del(a.foo)
assert(!defined(a.foo))
