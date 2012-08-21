f = procedure()
    ct=0
    for i in range(1,10)
        assertInt(ct,i-1)
        ct=ct+1
    endfor
    assertInt(ct,9)
end

q = gc()
f()
# check it gc's
assertInt(gc(),q)
