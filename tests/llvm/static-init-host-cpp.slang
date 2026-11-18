//TEST:SIMPLE(filecheck=CHECK): -target host-cpp -whole-program -O0 -entry main -stage compute

// Companion regression for shader-slang/slang#14578: host-style C++ source
// targets take the same link-time preservation as the LLVM host path, but
// through a different emission mechanism — the C++ emitter emits the
// preserved global at namespace scope with an initializer function, rather
// than deferring it into a module constructor. The unreferenced side-effecting
// static, its initializer function, and a forward declaration of the function
// the initializer calls (required before the initializer function, since the
// C++ emitter emits all globals before all function definitions) must all
// survive into the emitted source. Pinned at -O0 like the LLVM variants.

//CHECK: static int32_t computeInitialValue_0();
//CHECK: int32_t s_0_init()
//CHECK: {{.*}}int32_t s_0 = s_0_init();

static int s = computeInitialValue();

int computeInitialValue()
{
    int sum = 0;
    for (int i = 0; i < 7; ++i)
        sum += i;
    return sum;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
