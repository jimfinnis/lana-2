a="hello "+"world"
assert("hello world"==a)

spaces = function(n)
    a = "" # test of empty string
    i=0
    while(i<n)
        a = a+"."
        i=i+1
    endwhile
    return a
end

assertStr(".....",spaces(5))
    
assertStr("xo"*4,"xoxoxoxo")
