# test increasingly complex user functions

# a simple test making sure we can distinguish locals from parameters.
# this test failed quite badly at one point

verybasic = function(a)
    cog=3
    bba=4

    return a
end

# another test, of multiple arguments

foo = function(a,b)
    a = a+4
    return a-b*2
end

# making sure functions can call functions and not get arguments
# confused

foo1 = function(a,b,c)
    a = foo(a,b)-c
    b = foo(b,a)+c-a
    return foo(a,b)+foo(b,a)
end

# another layer of testing, this time a function calling a function
# calling a function

foo3 = function(a,b,c)
    b = foo1(a+1,c-1,b+a*2)
    a = foo(b+3,a-2) + foo1(b*2+a,a+2*b-c,a-c)
    c = foo1(a,c*2,b+c*2)
    return foo1(20,c*2,a) + foo1(a,2*c,a) + foo1(10,20,30+b)
end

# the standard factorial test

factorial = function(a)
    if a==0
        return 1
    else
        return factorial(a-1)*a
    endif
end

# we call the above functions, setting globals which are checked
# in userfuncs.cpp since these are basic tests.

t0=verybasic(25)
t1=foo(1,2)
t2=foo1(1,2,3)
bar=foo3(1,4,-7)
resetinstcount()                    # reset the instruction counter
fac10=factorial(10)
#print("count="+str(instcount()))    # print the number of instructions for 10 factorial
#print("factorial="+str(fac10))      # and print the value too.

