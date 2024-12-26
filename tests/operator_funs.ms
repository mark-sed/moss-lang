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
}

no = NumOffset(10, 1000)
no.(+)(101)
"\n"
no2 = NumOffset(10, -1000)
no.(==)(no2)
"\n"
no.(==)(no)
"\n"
no.(-)(10)
"\n"
no.(-)()
"\n"
no.(!=)(no2)
"\n"
no.(!=)(no)
"\n"