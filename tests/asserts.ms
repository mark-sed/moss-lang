try { assert(false); } catch (e:AssertionError) e

/**
 * some comment to offset line number
 */

try {
    
    assert(false, "assert"); 
} catch (e:AssertionError)
    e

fun foo(x) {
    if (not x) {
        try { assert(x, "x is false"); } catch (e:AssertionError) e
    }
}

~foo(false)
