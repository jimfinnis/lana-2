oldGC=gc()

a=createTest()

a.test1(2)
assertInt(1600,a.test2())

# test the custom 'i' property - this weirdo returns twice the value
# it was set to!

a.i = 1
assertInt(2,a.i)


# create a new object and link through the custom
# 'link' property

b=createTest()
a.link = b

# should now be two objects after GC (and maybe one or two protos)
assertInt(oldGC+2,gc())

# ensure it's right
if a.link!=b: fail()
if !a.testLink(b): fail()


# here we create a chain of links
# a->b->c->d

c=createTest()
b.link=c
d=createTest()
c.link=d
d.i = 42

# test the chain for correctness
assert(a.link == b)
assert(b.link == c)
assert(c.link == d)
assertInt(84,a.link.link.link.i)

# garbage collect and ensure there were four items left over
assertInt(oldGC+4,gc())

# convert the chain into a cycle
d.link=a

# and clear all the variables
a=0
b=0
c=0
d=0

# gc should now clear everything, detecting the cycle
assertInt(oldGC,gc())
