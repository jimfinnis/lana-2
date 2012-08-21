a=create()
a.v = 1
b=create()
b.v = 2

a.foo = procedure()
    print(this.v) # should print 1
    b.bar()
    print(this.v) # should print 1
end

b.bar = procedure()
    print(this.v)  # should print 2
end

q= procedure()
    a.foo()
end

q()
