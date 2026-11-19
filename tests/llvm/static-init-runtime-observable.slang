//TEST:EXECUTABLE:

// Runtime regression for shader-slang/slang#14578: the shape tests in this
// directory pin the emitted module text; this test proves the preserved
// initializer's effect is observable when the compiled program runs. The
// unreferenced `s` calls `bump()` twice during static initialization — before
// the fix the link dropped `s` and its initializer wholesale, so `bump()`
// never ran and the state stayed 0. `t` is the referenced-static interaction:
// `main` references it, so it is cloned through the entry-point clone graph
// and the new allowlist only adds a keepAlive decoration to that existing
// clone; its initializer stays at namespace scope and runs at load time like
// `s`'s. `u` covers a callee the forward-declaration pass must skip: a
// target-intrinsic call lowers to its inline text, so no declaration is owed,
// and the emitted program must still compile and run. The harness builds its
// own command line for executable tests (fixed `-target exe` and include
// paths; directive arguments are not passed through), so no options appear
// here; the .expected file pins the output.

static int g_state = 0;

int bump()
{
    g_state = g_state + 3;
    return 6;
}

int eleven()
{
    return 11;
}

__target_intrinsic(cpp, "7")
int intrinsicValue();

static int s = bump() + bump();

static int t = eleven();

static int u = intrinsicValue() + eleven();

export __extern_cpp int main()
{
    printf("unreferenced-static-init=%d\n", g_state);
    printf("referenced-static=%d\n", t);
    return 0;
}
