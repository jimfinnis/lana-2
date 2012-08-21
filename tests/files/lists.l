oldGC = gc()

a = list()
a[1]=100
b= clone(a)
assertInt(100,b[1])
b[1]=200
assertInt(200,b[1])
assertInt(100,a[1])

a[0]=b
a.append(1)
a.append(2)
assertInt(1,a[2])
assertInt(2,a[3])

q = procedure()
    for i in values(a)
        print(i)
    endfor
end

q()

#
# Best do some GC checks
#

print(a)
print(b)
b=0

assertInt(oldGC+2,gc())
a=0
assertInt(oldGC,gc())

#
#Push and pop
#

a=list()
a.push(10)
a.push(30)
assertInt(30,a.pop())
assertInt(10,a.pop())

#both ends
a.push(10)
a.shift(1)
a.push(30)
a.shift(2)
assertInt(30,a.pop())
assertInt(10,a.pop())
assertInt(2,a.unshift())
assertInt(1,a.unshift())

# just a quick check that the ends are right

a.push(1)
a.push(2)
a.push(3)
assertInt(1,a[0])
assertInt(2,a[1])
assertInt(3,a[2])
assertInt(3,a.peek())
