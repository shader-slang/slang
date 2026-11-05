//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/language-feature/hlsl-default-this-param-mode.slang -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -

// Methods written in HLSL use `in` for `this` by default. Each explicit receiver-mode spelling
// below selects its corresponding mode, and trailing `const` requests a nonmutating receiver.
// `replaceWithDefault` also checks member lookup in a default argument.

struct Counter
{
    int value;

    int defaultNonmutating()
    {
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

    [mutating]
    void replaceWithDefault(int replacement = defaultNonmutating())
    {
        value = replacement;
    }
};

// A default HLSL requirement has the same nonmutating receiver as an explicitly nonmutating
// implementation.
interface INonmutatingReceiver
{
    int read();
};

struct NonmutatingReceiverImplementation : INonmutatingReceiver
{
    int value;

    [nonmutating]
    int read()
    {
        return value;
    }
};

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter counter = {0};
    int result = counter.defaultNonmutating();
    counter.explicitMutating();
    result += counter.explicitNonmutating();
    result += counter.trailingConst();
    result += counter.explicitBorrow();
    counter.explicitReference();
    counter.replaceWithDefault(result);

    NonmutatingReceiverImplementation implementation = {result};
    result += implementation.read();
}

// CHECK-LABEL: [nameHint("Counter.defaultNonmutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
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
// CHECK-LABEL: [nameHint("NonmutatingReceiverImplementation.read")]
// CHECK-NEXT: func {{.*}} : Func(Int, %NonmutatingReceiverImplementation)
