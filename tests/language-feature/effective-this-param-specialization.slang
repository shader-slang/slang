//TEST:SIMPLE: -no-codegen -Wno-38109
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir -Wno-38109

// The generic method records both T and its declaration-derived In mode. Specialization changes
// the effective parameter type, but it must not change that mode: the unspecialized callee and all
// specialized call sites share one ABI.

interface IReadable
{
    int read();
}

struct CopyableValue : IReadable
{
    int value;

    int read()
    {
        return value;
    }
}

[__NonCopyableType]
struct NonCopyableValue : IReadable
{
    int value;

    int read()
    {
        return value;
    }
}

extension<T : IReadable> T
{
    int readTwice()
    {
        return this.read() * 2;
    }
}

// The same declaration-only rule applies to ordinary parameters. Specializing `T` to a
// non-copyable type must not re-default this declaration's `In` parameter to `BorrowIn`.
void consumeByValue<T>(T value)
{}

void exerciseSpecializations()
{
    CopyableValue copyable = {2};
    NonCopyableValue nonCopyable;
    nonCopyable.value = 3;

    int a = copyable.readTwice();
    int b = nonCopyable.readTwice();
    consumeByValue(nonCopyable);
}

// The generic callees encode their declaration-time modes before any concrete specialization.
// CHECK-LABEL: [nameHint("readTwice")]
// CHECK: func {{.*}} : Func(Int, %{{.*}})
// CHECK-LABEL: [nameHint("consumeByValue")]
// CHECK: func {{.*}} : Func(Void, %{{.*}})

// The non-copyable call sites pass values to those `In` parameters. A substituted-type
// re-defaulting bug would instead introduce a borrow wrapper and pass storage.
// CHECK-LABEL: [nameHint("exerciseSpecializations")]
// CHECK: let [[RECEIVER_VALUE:%.*]] : %NonCopyableValue = load({{.*}})
// CHECK: call specialize({{.*}}%NonCopyableValue{{.*}})([[RECEIVER_VALUE]])
// CHECK: let [[PARAM_VALUE:%.*]] : %NonCopyableValue = load({{.*}})
// CHECK: call specialize({{.*}}%NonCopyableValue{{.*}})([[PARAM_VALUE]])
