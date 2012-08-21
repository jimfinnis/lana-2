# create an object

a = create()

# set some values in it

a.val1=1
a.val2=2

# set a function in it (i.e a method)

a.setvals = procedure(a,b)
    this.val1 = a
    this.val2 = b
end

# and another one which does a calculation

a.somecalc = function()
    return this.val1+this.val2
end

# assert that the calculation gets the correct value

assertInt(3,a.somecalc())

# make a clone of the object

b = clone(a)
b.setvals(3,5)

# assert that the calculation now gives a different result

assertInt(8,b.somecalc())

# create a new function as a first-class value
# but first copy the old function somewhere safe!

oldfunc = a.somecalc
newfunc = function()
    return this.val1-this.val2
end

# and set this in 'a' - it should also change in 'b'

a.somecalc=newfunc
assertInt(-1,a.somecalc())
assertInt(-2,b.somecalc())

# now change 'b' back to the old calc; 'a' should not change

b.somecalc=oldfunc
assertInt(-1,a.somecalc())
assertInt(8,b.somecalc())

