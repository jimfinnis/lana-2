#
# test of nested loops
#
#

foo = function(a,b)
    out=""
    i=0
    while i<a
        out=out+"x"
        j=0
        while j<b
            out=out+"o"
            j=j+1
        endwhile
        i=i+1
    endwhile
    return out
end

assertStr("",foo(0,2))
assertStr("xx",foo(2,0))
assertStr("xooooxoooo",foo(2,4))

#
# nested rep-un
#

foo = function(a,b)
    out=""
    i=0
    repeat
        out=out+"x"
        j=0
        repeat
            out=out+"o"
            j=j+1
        until j>=b
        i=i+1
    until i>=a
    return out
end

assertStr("xoo",foo(0,2))
assertStr("xoxo",foo(2,0))
assertStr("xooooxoooo",foo(2,4))

# while inside repeat-until

foo = function(a,b)
    out=""
    i=0
    repeat
        out=out+"x"
        j=0
        while j<b
            out=out+"o"
            j=j+1
        endwhile
        i=i+1
    until i>=a
    return out
end

assertStr("xoo",foo(0,2))
assertStr("xx",foo(2,0))
assertStr("xooooxoooo",foo(2,4))

# as above, but using break to leave inner loop

foo = function(a,b)
    out=""
    i=0
    repeat
        out=out+"x"
        j=0
        while true
            out=out+"o"
            if j==b: break # also test of quick-if
            j=j+1
        endwhile
        i=i+1
    until i>=a
    return out
end

assertStr("xooo",foo(0,2))
assertStr("xoxo",foo(2,0))
assertStr("xoooooxooooo",foo(2,4))

