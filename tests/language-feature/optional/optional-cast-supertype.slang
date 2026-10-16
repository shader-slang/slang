//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

// An Optional initialized from a `derived as Base` upcast must hold the base value,
// not the address of the base sub-object, so `.value` reads fields of a struct.
// The property must hold on both the interpreter (INTERPRET) and code generation
// (SPIRV); reaching OpEntryPoint proves SPIR-V emission completed.

struct A
{
    int x;
}

struct B : A
{
    int y;
}

int upcastToOptional(int seed)
{
    B b;
    b.x = seed;
    b.y = seed + 1;
    Optional<A> o = b as A;
    return o.hasValue ? o.value.x : -1;
}

// SPIR-V generation of the same upcast must complete; OpEntryPoint appears only
// once code emission finishes.
// SPV: OpEntryPoint
RWStructuredBuffer<int> outBuf;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = upcastToOptional(5);
}

void main()
{
    // Repro: `b as A` on a value whose base is a stored sub-object.
    B b;
    b.x = 5;
    b.y = 6;
    Optional<A> o = b as A;
    // CHECK: 5
    if (o.hasValue)
        printf("%d\n", o.value.x);

    // Control: a same-type `as` is an identity cast, not a base-subobject
    // extraction, so its payload is already a plain value.
    A a;
    a.x = 7;
    Optional<A> o2 = a as A;
    // CHECK: 7
    if (o2.hasValue)
        printf("%d\n", o2.value.x);

    // Control: the plain base-subobject upcast (no Optional) must still read the
    // right field, isolating the upcast lowering from Optional packaging.
    B b3;
    b3.x = 9;
    b3.y = 10;
    A a3 = b3;
    // CHECK: 9
    printf("%d\n", a3.x);
}
