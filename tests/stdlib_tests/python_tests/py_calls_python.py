def foo(a, b, /, c, d=1, *ff, e=9, f=2):
    return f"Hi from foo\n{a} {b} {c} {d} {e} {f} {ff}\n{a + b}"

def bar(a=1, b=3):
    return (a - b, b-a)

def add(a, b):
    return a + b


def greet(name, greeting="Hello"):
    return f"{greeting}, {name}!\n"


def pos_only(a, b, /):
    return a * b


def kw_only(*, x, y=10):
    return x + y


def mixed(a, b=2, *args, c, d=4, **kwargs):
    return {
        "a": a,
        "b": b,
        "args": args,
        "c": c,
        "d": d,
        "kwargs": kwargs,
    }


def collect(*args):
    return args


def collect_kwargs(**kwargs):
    return kwargs


def everything(a, b=2, /, c=3, *args, d, e=5, **kwargs):
    return {
        "a": a,
        "b": b,
        "c": c,
        "args": args,
        "d": d,
        "e": e,
        "kwargs": kwargs,
    }