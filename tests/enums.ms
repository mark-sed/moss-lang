enum Colors {
    Blue
    Red
    Green
    Purple
}

Colors
"\n"
vardump(Colors)

Colors.Red
"\n"
Colors.Purple
"\n"
try {
    Colors.Black
} catch (e) {
    "caught\n"
}

Colors.Blue == Colors.Green
"\n"
Colors.Purple == Colors.Purple
"\n"

enum Colors2 {
    Blue
    Red
    Green
    Purple
}

Colors.Blue == Colors2.Blue
"\n"

e1 = Colors.Purple
e2 = Colors.Blue
e3 = Colors2.Blue

e1 == e2
"\n"
e1 == e1
"\n"
e1 == e3
"\n"
e2 == e3
"\n"

try {
    e1 > e2
} catch (e:TypeError) {
    "caught\n"
}

// Enum needs type before it
try {
    Purple
} catch (e:NameError) {
    "caught\n"
}

class MC {
    enum State { NEW, OLD, EXP }

    fun MC(st) {
        this.state = st
    }

    fun get_state() {
        return this.state
    }
}

mc = MC(MC.State.OLD)
mc.get_state()
"\n"
if (mc.state == MC.State.OLD) { "old\n"; }
 