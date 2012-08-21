# test simple loops
#testy testy test

# while loop test

foo = function(ct)
    i = 0
    j = 0
    while i<ct
        i=i+1
        j=j+i
    endwhile
    assertInt(ct,i)
    return j
end

assertInt(20100,foo(200))

# repeat-until test, also testing appending to a string!

bar = function(ct)
    i=0
    j="f"
    repeat
        i=i+1
        j=j+"o"
    until i==ct
    assertInt(ct,i)
    return j
end

assertStr("foooo",bar(4))

# while-break test

foo = function(ct)
    i=0
    while i<100
        i = i+1
        if i==ct
            break
        endif
    endwhile
    return -i
end

assertInt(-12,foo(12))

# while-continue test

foo = function(ct)
    out=""
    i=0
    while i<6
        i = i+1
        out = out+"x"
        if i==ct
            continue
        endif
        out = out+"o"
    endwhile
    return out
end

assertStr("xoxoxxoxoxo",foo(3))

# repeat-break test

foo = function(ct)
    i=0
    repeat
        i=i+1
        if i==ct
            break
        endif
    until i==10
    return -i
end

assertInt(-10,foo(12))
assertInt(-5,foo(5))

# repeat-continue test
# which demonstrates the correct semantics. There's a choice here:
# continue can either jump to the test, or jump back to the start
# of the loop ignoring the test. We do the latter.

foo = function(ct)
    # first part of test, without continue!
    i=0
    repeat
        i=i+1
        if i==100
            break
        endif
    until i==ct
    assertInt(i,ct)

    # second part of test, continue causes the end of the loop
    # test to be skipped
    i=0
    repeat
        i=i+1
        if i==ct
            continue
        endif
        if i==100
            break
        endif
    until i==ct # continue will skip this
    return -i
end

assertInt(-100,foo(10))  
