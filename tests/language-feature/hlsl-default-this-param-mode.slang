//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/language-feature/hlsl-default-this-param-mode.slang -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -
//TEST:SIMPLE_EX(filecheck=OPTIN): -lang hlsl tests/language-feature/hlsl-default-this-param-mode.slang -fhlsl-methods-mutating-by-default -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -

// Methods written in HLSL use `in` for `this` by default. Each explicit receiver-mode spelling
// below selects its corresponding mode, and trailing `const` requests a nonmutating receiver.
// `replaceWithDefault` also checks member lookup in a default argument.
// With the HLSL opt-in, unannotated methods use `inout`; explicit specifiers keep their modes.

struct Counter
{
    int value;

    int unannotated()
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
    void replaceWithDefault(int replacement = unannotated())
    {
        value = replacement;
    }
};

// A nonmutating implementation can satisfy this requirement under either HLSL default.
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
    int result = counter.unannotated();
    counter.explicitMutating();
    result += counter.explicitNonmutating();
    result += counter.trailingConst();
    result += counter.explicitBorrow();
    counter.explicitReference();
    counter.replaceWithDefault(result);

    NonmutatingReceiverImplementation implementation = {result};
    result += implementation.read();

    // A synthesized lambda must still satisfy the nonmutating core `IFunc` requirement.
    let identity = (int value) => value;
    result = identity(result);
}

// CHECK-LABEL: [nameHint("Counter.unannotated")]
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

// OPTIN-LABEL: [nameHint("Counter.unannotated")]
// OPTIN-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// OPTIN-LABEL: [nameHint("Counter.explicitMutating")]
// OPTIN-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter))
// OPTIN-LABEL: [nameHint("Counter.explicitNonmutating")]
// OPTIN-NEXT: func {{.*}} : Func(Int, %Counter)
// OPTIN-LABEL: [nameHint("Counter.trailingConst")]
// OPTIN-NEXT: func {{.*}} : Func(Int, %Counter)
// OPTIN-LABEL: [nameHint("Counter.explicitBorrow")]
// OPTIN-NEXT: func {{.*}} : Func(Int, BorrowInParam(%Counter{{.*}}))
// OPTIN-LABEL: [nameHint("Counter.explicitReference")]
// OPTIN-NEXT: func {{.*}} : Func(Void, RefParam(%Counter{{.*}}))
// OPTIN-LABEL: [nameHint("Counter.replaceWithDefault")]
// OPTIN-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter), Int)
// OPTIN-LABEL: [nameHint("NonmutatingReceiverImplementation.read")]
// OPTIN-NEXT: func {{.*}} : Func(Int, %NonmutatingReceiverImplementation)
