//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/language-feature/hlsl-default-this-param-mode.slang -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -

// Methods written in HLSL use `inout` for `this` by default. Each explicit receiver-mode spelling
// overrides that fallback, and trailing `const` has the same receiver-mode meaning as
// `[nonmutating]`. Extension methods use the HLSL fallback. Effective receiver-mode computation
// continues to apply the declaration-kind defaults for property accessors: getters and `ref`
// accessors are read-only, while setters are writable.

static int referencedValue;

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

    func modernTrailingConst() const -> int
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

    int invokeLambda()
    {
        let read = () => value;
        return read();
    }

    property int item
    {
        get
        {
            return value;
        }
        set
        {
            value = newValue;
        }
    }

    property int referencedItem
    {
        ref { return referencedValue; }
    }
};

// A read-only noncopyable receiver cannot use HLSL's ordinary by-value representation. Both
// explicit read-only receiver spellings therefore preserve the contract with `BorrowIn`.
[__NonCopyableType]
struct NonCopyableReadOnlyReceiver
{
    int value;

    [nonmutating]
    int explicitNonmutating()
    {
        return value;
    }

    int trailingConst() const
    {
        return value;
    }
};

extension Counter
{
    int extensionDefaultMutating()
    {
        value++;
        return value;
    }

    int extensionTrailingConst() const
    {
        return value;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Counter counter = {0};
    int result = counter.defaultMutating();
    counter.explicitMutating();
    result += counter.explicitNonmutating();
    result += counter.trailingConst();
    result += counter.modernTrailingConst();
    result += counter.explicitBorrow();
    counter.explicitReference();
    result += counter.invokeLambda();
    counter.item = result;
    result += counter.item;
    counter.referencedItem = result;
    result += counter.extensionDefaultMutating();
    result += counter.extensionTrailingConst();

    NonCopyableReadOnlyReceiver nonCopyableReceiver = {result};
    result += nonCopyableReceiver.explicitNonmutating();
    result += nonCopyableReceiver.trailingConst();
}

// CHECK-LABEL: [nameHint("Counter.defaultMutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.explicitMutating")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.explicitNonmutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.trailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.modernTrailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.explicitBorrow")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInParam(%Counter{{.*}}))
// CHECK-LABEL: [nameHint("Counter.explicitReference")]
// CHECK-NEXT: func {{.*}} : Func(Void, RefParam(%Counter{{.*}}))
// A compiler-synthesized lambda call operator uses `in` rather than the default for methods written
// in HLSL.
// CHECK-LABEL: [nameHint("Counter.invokeLambda._slang_Lambda_invokeLambda_0.()")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter{{.*}}Lambda{{.*}})
// CHECK-LABEL: [nameHint("Counter.invokeLambda")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.item.get")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.item.set")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter), Int)
// CHECK-LABEL: [nameHint("Counter.referencedItem.ref")]
// CHECK-NEXT: func {{.*}} : Func(Ptr(Int), %Counter)
// CHECK-LABEL: [nameHint("Counter.extensionDefaultMutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.extensionTrailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("NonCopyableReadOnlyReceiver.explicitNonmutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInParam(%NonCopyableReadOnlyReceiver{{.*}}))
// CHECK-LABEL: [nameHint("NonCopyableReadOnlyReceiver.trailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInParam(%NonCopyableReadOnlyReceiver{{.*}}))
