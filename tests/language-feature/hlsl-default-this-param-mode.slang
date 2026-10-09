//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/language-feature/hlsl-default-this-param-mode.slang -target spirv-asm -stage compute -entry computeMain -dump-ir -O0 -o -

// In the HLSL dialect, non-static methods declared on value types receive `this` as
// `inout` by default. Every explicit receiver-mode spelling overrides that default, and C++-style
// trailing `const` has the same effective mode as `[nonmutating]`. Methods declared in an extension
// of a value type use the same rules.

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
    result += counter.explicitBorrow();
    counter.explicitReference();
    result += counter.invokeLambda();
    counter.item = result;
    result += counter.extensionDefaultMutating();
    result += counter.extensionTrailingConst();
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
// A compiler-synthesized lambda call operator uses `in` rather than inheriting the HLSL default
// for source member functions, so an immutable closure value can invoke it. The surrounding source
// member function still uses HLSL's writable default.
// CHECK-LABEL: [nameHint("Counter.invokeLambda._slang_Lambda_invokeLambda_0.()")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter{{.*}}Lambda{{.*}})
// CHECK-LABEL: [nameHint("Counter.invokeLambda")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.item.get")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
// CHECK-LABEL: [nameHint("Counter.item.set")]
// CHECK-NEXT: func {{.*}} : Func(Void, BorrowInOutParam(%Counter), Int)
// CHECK-LABEL: [nameHint("Counter.extensionDefaultMutating")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%Counter))
// CHECK-LABEL: [nameHint("Counter.extensionTrailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, %Counter)
