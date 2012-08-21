#
# a test of simple cloning inheritance
#

a = create()
a.foo = 7
a.zog = 10

b = clone(a)
b.bar = 9
b.zog = a.zog # explicit copy

# b inherits a's 7
assertInt(7,b.foo)

# has its own value here
assertInt(9,b.bar)

# and a copy of a's value
assertInt(10,b.zog)

# if we change a.foo, b.foo will change
a.foo = 20
assertInt(20,b.foo)

# but if we change a.zog, b.zog will not change
a.zog = 1000
assertInt(10,b.zog)
# clone hierarchy

c=clone(b)
assertInt(10,c.zog)
assertInt(1000,a.zog)
assertInt(20,c.foo) #inherited from b
assertInt(9,c.bar)

c.bar=10
assertInt(9,b.bar)
assertInt(10,c.bar)
