# a simple forward jump terminating a loop


foo = function(a,b)
    total = 0
    while true
        total = total + 1
        if a>b
            goto loopend
        endif
        a=a+1
    endwhile
loopend:
    return total    
end

assertInt(8,foo(4,10))

#
# a complex set of forward and backward jumps
#

foo = function()
    out = ""
    goto a
g: # comment here is allowed
    out=out+"d"
    goto h
c:
    out=out+"h"
    goto d
h:
    out=out+"s"
    return out
b:
    out = out+"s"
    goto c
a:
    out = out+"f"
    out = out+"i"
    goto b
d:
    out=out+"-h"
    goto e
f:
    out=out+"a"
    goto g
e:
    out=out+"e"
    goto f
end

assertStr("fish-heads",foo())
