//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/language-feature/hlsl-default-this-param-mode.slang -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -

// Methods written in HLSL use `inout` for `this` by default. Each explicit receiver-mode spelling
// below selects its corresponding mode, and trailing `const` requests a nonmutating receiver.
// `replaceWithDefault` also checks lookup of a mutating member in a default argument.

struct Counter
{
    int value;

    int defaultMutating()
    {
        value++;
        return value;
    }

    [mutating]
    void explicitMutating()
    {
        value++;
    }

    [nonmutating]
    int explicitNonmutating()
    {
        return value;
    }

    int trailingConst() const
    {
        return value;
    }

    [constref]
    int explicitBorrow()
    {
        return value;
    }

    [__ref]
    void explicitReference()
    {
        value++;
    }

    void replaceWithDefault(int replacement = defaultMutating())
    {
        value = replacement;
    }
};

// A default HLSL requirement has the same mutating receiver as an explicitly mutating
// implementation.
interface IMutatingReceiver
{
    void mutate();
};

struct MutatingReceiverImplementation : IMutatingReceiver
{
    int value;

    [mutating]
    void mutate()
    {
        value++;
    }
};

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter counter = {0};
    int result = counter.defaultMutating();
    counter.explicitMutating();
    result += counter.explicitNonmutating();
    result += counter.trailingConst();
    result += counter.explicitBorrow();
    counter.explicitReference();
    counter.replaceWithDefault(result);

    MutatingReceiverImplementation implementation = {result};
    implementation.mutate();
}

// CHECK-LABEL: [nameHint("Counter.defaultMutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.explicitMutating")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.explicitNonmutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.trailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.explicitBorrow")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInParam(%Counter{{.*}}))
// CHECK-LABEL: [nameHint("Counter.explicitReference")]
// CHECK-NEXT: func {{.*}} : Func(Void, RefParam(%Counter{{.*}}))
// CHECK-LABEL: [nameHint("Counter.replaceWithDefault")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter), Int)
// CHECK-LABEL: [nameHint("MutatingReceiverImplementation.mutate")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%MutatingReceiverImplementation))
