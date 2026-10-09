//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A setter whose value type is non-copyable receives its new value through an implicit
// `BorrowIn` parameter. Both semantic checking and lowering must therefore use the right-hand
// side's `ref` accessor to obtain an address, even when a getter is also available.

[__NonCopyableType]
struct NonCopyableValue
{
    int value;
}

static NonCopyableValue externalValue = {0};

struct NonMutatingRefSource
{
    property NonCopyableValue item
    {
        [mutating]
        get
        {
            NonCopyableValue result;
            result.value = 0;
            return result;
        }

        [nonmutating]
        ref { return externalValue; }
    }
}

struct NonCopyableValueSink
{
    property NonCopyableValue item
    {
        set(NonCopyableValue newValue) {}
    }
}

void passRefAddressToSetter(
    NonMutatingRefSource source,
    inout NonCopyableValueSink destination)
{
    destination.item = source.item;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    NonMutatingRefSource source;
    NonCopyableValueSink destination;
    passRefAddressToSetter(source, destination);
}

// CHECK-LABEL: [nameHint("NonMutatingRefSource.item.ref")]
// CHECK-NEXT: func [[REF_ACCESSOR:%[A-Za-z0-9_]+]] :
// CHECK-SAME: Func(Ptr(%NonCopyableValue), %NonMutatingRefSource)
// CHECK-LABEL: [nameHint("NonCopyableValueSink.item.set")]
// CHECK: func [[SETTER:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("passRefAddressToSetter")]
// CHECK-NOT: call
// CHECK: let [[VALUE_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%NonCopyableValue) = call [[REF_ACCESSOR]]
// CHECK-NEXT: call [[SETTER]]({{.*}}, [[VALUE_ADDRESS]])
// CHECK-NEXT: return_val
