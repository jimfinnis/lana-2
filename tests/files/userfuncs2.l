# test of nested user funcs WITHOUT closures

# we write a function foo
foo = function(a,b)
    # which creates a function v - actually since there is no closure
    # this is just a local function created at compile tile
    v = function(a,b,c)
        q = a+b
        return a+100*b+c-q
    end
    # which we then use to return a value
    return v(a+b,10,b-a)+a
end

# make sure the value is correct
assertInt(996,foo(1,6))


# test of longish functions
q = function()
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    i=1
    return i
end
assertInt(1,q())

