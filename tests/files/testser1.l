#
# Serialisation test file. This file is loaded and serialised,
# Lana is restarted, and the serialised data is loaded. Tests
# are then done to confirm that it's the same data.
#

# this is a binary tree node

treenode = create()
    treenode.add = function(key,value)
        if defined(this.key)
            if this.key == key
                return false    # already there
            elseif key < this.key
                if defined(this.left)
                    return this.left.add(key,value)
                else
                    t = clone(treenode)
                    t.key = key
                    t.value = value
                    this.left = t
                    return true
                endif
            else
                if defined(this.right)
                    return this.right.add(key,value)
                else
                    t = clone(treenode)
                    t.key = key
                    t.value = value
                    this.right = t
                    return true
                endif
            endif
        else
            this.key = key
            this.value = value
            return true
        endif
    end
                    
    treenode.do_traverse = procedure(level)
        if defined(this.key)
            if defined(this.left): this.left.do_traverse(level+1)
            print(" "*level + str(this.key) + "  " + str(this.value))
            if defined(this.right): this.right.do_traverse(level+1)
        endif
    end

    treenode.traverse = procedure()
        this.do_traverse(0)
    end

    treenode.equal = function(b)
    # are these nodes equal?
        if this.key != b.key: return false
        if this.value != b.value: return false
        if defined(this.left)
            if !defined(b.left): return false
            if !this.left.equal(b.left): return false
        endif
        if defined(this.right)
            if !defined(b.right): return false
            if !this.right.equal(b.right): return false
        endif
        return true
    end

$treenode = treenode # make a global
    

make = function()
    count=0
    tree = clone($treenode)
    for x in range(1,511)
        x = (x*329)%1021
        if tree.add(x,x*x): count=count+1
    endfor
    print("created "+str(count)+" nodes.")
    return tree
end


