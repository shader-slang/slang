//TEST:EXECUTABLE: -target exe -whole-program -O0

// Runtime regression for shader-slang/slang#14578: the shape tests in this
// directory pin the emitted module text (the constructor exists, is
// registered in llvm.global_ctors); this test proves the preserved
// initializer's effect is actually observable when the compiled program
// runs. The unreferenced `s` calls `bump()` twice during static
// initialization — before the fix the link dropped `s` and its initializer
// wholesale, so `bump()` never ran and the state stayed 0. The compiled
// program must print the state after those calls (the expected output file
// pins it). `t` is the referenced-static interaction: its initializer
// already ran through the entry-point clone path, and the already-cloned
// copy now also gains a keepAlive decoration. The C++ emission path requires
// forward declarations for functions called from initializers
// (static-init-host-cpp.slang pins that text shape).

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

static int s = bump() + bump();

static int t = eleven();

export __extern_cpp int main()
{
    printf("unreferenced-static-init=%d\n", g_state);
    printf("referenced-static=%d\n", t);
    return 0;
}
