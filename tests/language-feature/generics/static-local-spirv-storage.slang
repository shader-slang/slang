// Regression test for shader-slang/slang#11651.
//
// A function-local `static` variable reached through a non-inlined helper that
// runs multiple times in one dispatch (here via a variadic-pack `expand`) must
// keep its backing storage module-scope `Private` so its value persists across
// calls. It must NOT be demoted to per-call `Function` storage. `writeSingle`
// is forced out-of-line with `[noinline]` so the storage class is observable in
// the emitted SPIR-V (the inlined form folds the variable into the entry point
// and would mask the bug).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

struct S<T>
{
    T v;
    __init(T x) { v = x; }
}

S<T> makeS<T>(T x)
{
    return S<T>(x);
}

void accept<each T>(expand each T value) {}

[noinline]
void writeSingle<T : __BuiltinFloatingPointType>(T value)
{
    static int counter = 0;
    outputBuffer[counter++] = __realCast<float>(value);
}

void write<each T : __BuiltinFloatingPointType>(expand S<each T> value)
{
    accept(expand writeSingle((each value).v));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    write(makeS(1.0), makeS(2.0), makeS(3.0), makeS(4.0));
    write();
}

// The function-`static` `counter` must remain module-scope `Private`.
// CHECK: %counter = OpVariable %{{.*}} Private
