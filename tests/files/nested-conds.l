#
# Tests, using the assert mechanism, of various 
# nested conditions - also dangling else, and elseif.
#

foo = function(a)
    if a < 10
        if a < 0
            return 1  # a<0
        else
            return 2  # 0<=a<10
        endif
    else
        return 3      # a>=10
    endif
end

# test the above function

assertInt(3,foo(10))
assertInt(2,foo(0))
assertInt(1,foo(-1))
assertInt(3,foo(11))

#
# similar to the above, but with the nested condition
# inside the else branch
#

bar = function(a)
    if a < 10
        return 1      # a<10
    else
        if a > 20
            return 2  # a>20
        else
            return 3  # 10<=a<20
        endif
    endif
end

assertInt(1,bar(9))
assertInt(2,bar(21))
assertInt(3,bar(20))
assertInt(3,bar(10))


# Test the elseif

foo = function(a)
    b=0
    if a==1
        b=101
    elseif a==2
        b=201
    elseif a==3
        b=301
    else
        b=401
    endif
    return b
end

assertInt(101,foo(1))
assertInt(201,foo(2))
assertInt(301,foo(3))
assertInt(401,foo(4))
assertInt(401,foo(14))


# now some fun with quick-if

foo = function(a)
    if a<10
        if a==1: return 10
        if a==2: return 20
        if a==3: return 30
        return 40
    else
        if a==10: return -1
        if a==11: return -2
        if a==12: return -3
        return -4
    endif
end

assertInt(10,foo(1))
assertInt(20,foo(2))
assertInt(30,foo(3))
assertInt(40,foo(4))
assertInt(40,foo(5))

assertInt(-1,foo(10))
assertInt(-2,foo(11))
assertInt(-3,foo(12))
assertInt(-4,foo(13))
assertInt(-4,foo(14))
