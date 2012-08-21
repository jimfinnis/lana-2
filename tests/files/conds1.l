#
# Test simple conditions
#

foo = function(a,b)
    if a+b < 10
        return 12
    else
        return 3
    endif
end

bar = function(a,b)
    if a>b
        a=a+10
        b=b+10
    endif
    return a+b
end

# conds.cpp runs this, and checks the return values are correct
# (since this test runs before the lana asserter is tested)


t1=foo(1,4)
t2=foo(10,12)
t3=bar(3,4)
t4=bar(4,3)
