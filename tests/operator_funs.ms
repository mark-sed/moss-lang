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

    fun ([])(i) {
        return ((this.x + this.off)++"")[i]
    }

    fun (())(a:Int, b) {
        return this.x + a + b
    }

    fun (())() {
        return nil
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
no[0]++no[1]++"\n"

class NlText {

    fun NlText(txt) {
        this.txt = txt ++ "\n"
    }

    fun ([])(i) {
        return this.txt[i]
    }
}

t = NlText("Hi all!")
t[3]++t[0]++t[7]

try {
    no == 5
} catch (e:TypeError) {
    "caught\n"
}

try {
    t + 5
} catch(e:TypeError) {
    "caught\n"
}

no(3, 5)
"\n"
no()
"\n"
try
    no(1)
catch(e)
    "caught\n"

try
    no(true, 4)
catch(e)
    "caught\n"

class It {
    fun It(end) {
        this.end = end
    }

    fun __iter() {
        this.i = 0
        return this
    }

    fun __next() {
        if (this.i >= this.end)
            raise StopIteration()
        v = this.i
        this.i += 1
        return v
    }

    fun (())() {
        return "called " ++ this.end ++ "\n"
    }

    fun (())(pref) {
        return pref ++ " called " ++ this.end ++ "\n"
    }
}

it = It(3)
it()
it(">")