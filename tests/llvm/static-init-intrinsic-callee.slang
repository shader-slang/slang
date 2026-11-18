//TEST:SIMPLE(filecheck=CHECK): -target host-cpp -whole-program -O0 -entry main -stage compute

// Companion regression for shader-slang/slang#14578: a
// global initializer that calls a target-intrinsic function must not leave a
// storage-class prefix dangling in the emitted C++ — `emitFuncDecl` emits
// nothing for a target-intrinsic callee, so the forward-declaration pass must
// skip it entirely — and a callee that is public or exported must get a
// declaration whose storage specifier matches its definition (no `static` on
// either). The intrinsic call lowers to its inline text, so nothing named
// after the intrinsic may appear in the output.

//CHECK-NOT: intrinsicValue
//CHECK-NOT: static {{.*}}exportedValue_0
//CHECK: int32_t exportedValue_0();
//CHECK: int32_t s_0 = s_0_init();

__target_intrinsic(cpp, "7")
int intrinsicValue();

export int exportedValue()
{
    return 3;
}

static int s = intrinsicValue() + exportedValue();

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
