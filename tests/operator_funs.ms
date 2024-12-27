class NumOffset {

    fun NumOffset(x:[Int,Float], off:[Int,Float]) {
        this.x = x
        this.off = off
    }

    fun (+)(a) { return this.x + a + this.off; }
    
    fun (-)(a) {
        return this.x - a + this.off
    }

    fun (-)() {
        return -this.x + this.off
    }

    fun (==)(a:NumOffset) { 
        return this.x == a.x and this.off == a.off
    }

    fun (!=)(a:NumOffset) {
        return not this.(==)(a)
    }

    fun (/)(a) { 
        return this.x / a + this.off
    }

    fun (*)(a) { 
        return this.x * a + this.off
    }

    fun (^)(a) { 
        return (this.x ^ a) + this.off
    }

    fun (%)(a) { 
        return this.x % a + this.off
    }

    fun (>)(a) { 
        return this.x + this.off > a
    }

    fun (<)(a) { 
        return this.x + this.off < a
    }

    fun (>=)(a) { 
        return this.x + this.off >= a
    }

    fun (<=)(a) { 
        return this.x + this.off <= a
    }

    fun (in)(a) {
        return a <= this.x + this.off 
    }

    fun (and)(a) {
        return a and (this.x + this.off) 
    }

    fun (or)(a) {
        return a or (this.x + this.off) 
    }

    fun (xor)(a) {
        return a xor (this.x + this.off) 
    }

    fun (not)() {
        return not (this.x + this.off) 
    }
}

no = NumOffset(10, 1000)
no.(+)(101)
" == "
no + 101
" == "
NumOffset.(+)(101, no)
"\n"
no2 = NumOffset(10, -1000)
no.(==)(no2)
" == "
no == no2
"\n"
no.(==)(no)
" == "
no == no
"\n"
no.(-)(10)
" == "
no - 10
"\n"
no.(-)()
" == "
-no
"\n"
no.(!=)(no2)
" == "
no != no2
"\n"
no.(!=)(no)
" == "
no != no
"\n"

no / 2
"\n"
no * 2
"\n"
no % 3
"\n"
no > 1020
"\n"
no < 1020
"\n"
no >= 1010
"\n"
no <= 1010
"\n"
2000 in no
"\n"
1000 in no
"\n"
no and 0
"\n"
no and 0xFFFFFFFF
"\n"
no or 0
"\n"
no or 0xFF
"\n"
no xor no
"\n"
no xor -1
"\n"
not no
"\n"
no ^ 0
"\n"
no ^ 2
"\n"