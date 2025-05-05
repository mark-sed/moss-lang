// A -> B -> C -> D
//        -> E

@converter("A", "B")
fun A2B(txt) = "A2B:"++txt

@converter("B", "C")
fun B2C(txt) = "B2C:"++txt

@converter("B", "E")
fun B2E(txt) = "B2E:"++txt

@converter("C", "D")
fun C2D(txt) = "C2D:"++txt

@formatter
fun B(txt) = txt

Note("hi\n", "A")
try
    E"toE"
catch(e:TypeError)
    D"""caught
"""
B("FromB"++"\n")

//   A2
//  /  \
// B2  C2
//  \  /
//    D

@converter("A2", "B2")
fun A22B2(txt) = "A22B2:"++txt

@converter("A2", "C2")
fun A22C2(txt) = "A22C2:"++txt

@converter("B2", "D")
fun B22D2(txt) = "B22D:"++txt

@converter("C2", "D")
fun C22D2(txt) = "C22D:"++txt

A2"""txt
"""
C2"""txt2
"""

// A3 <-> B3 -> D
//  ^-----------| 

@converter("A3", "B3")
fun A32B3(txt) = "A32B3:"++txt

@converter("B3", "A3")
fun B32A3(txt) = "B32A3:"++txt

@converter("B3", "D")
fun B32D(txt) = "B32D:"++txt

@converter("D", "A3")
fun D2A3(txt) = "D2A3:"++txt

A3"""txt3
"""