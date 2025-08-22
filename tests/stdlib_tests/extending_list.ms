class MyList : List {
    fun __iter() = super.__iter()
    fun __String() = "ML:"++this.__value
}

ml = MyList([1,2,3,4])
type(ml)
"\n"
ml.__value
"\n"

for (a: ml)
    a
"\n"

ml.length()
"\n"

ml.empty()
"\n"

~ml.append(0)
~ml.append(1)
ml
"\n"

ml.pop(2)
"\n"
ml.pop()
"\n"
ml
"\n"

~ml.clear()
ml
"\n"