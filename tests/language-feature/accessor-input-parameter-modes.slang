//TEST:SIMPLE: -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A setter's value parameter and a subscript's index parameters are input-only, but they can use
// either the ordinary `In` mode or the immutable-borrow `BorrowIn` mode. Subscript parameters also
// retain the ordinary callable support for default arguments.

struct Storage
{
    int value;

    property int item
    {
        get
        {
            return value;
        }
        set(__constref int replacement)
        {
            value = replacement;
        }
    }

    __subscript(__constref int offset = 0) -> int
    {
        get
        {
            return value + offset;
        }
    }
}

// These parameters have no explicit mode spelling. Their non-copyable declared type adjusts the
// effective mode from `In` to `BorrowIn`, which is an allowed input-only mode for both accessors.
[__NonCopyableType]
struct NonCopyableInput
{
    int value;
}

struct NonCopyableStorage
{
    property NonCopyableInput item
    {
        set(NonCopyableInput replacement) {}
    }

    __subscript(NonCopyableInput index) -> int
    {
        get { return index.value; }
    }
}

int exerciseAccessorInputModes()
{
    Storage storage = {41};
    storage.item = 42;
    return storage[];
}

void exerciseNonCopyableSetter(
    inout NonCopyableStorage storage,
    NonCopyableInput replacement)
{
    storage.item = replacement;
}

// CHECK-LABEL: [nameHint("Storage.item.set")]
// CHECK: func {{.*}} : Func(Void, BorrowInOutParam(%Storage), BorrowInParam(Int

// The explicit const-reference setter argument is materialized and passed by address.
// CHECK-LABEL: [nameHint("exerciseAccessorInputModes")]
// CHECK: let [[SET_VALUE:%[0-9]+]] : Ptr(Int) = var
// CHECK-NEXT: store([[SET_VALUE]], 42 : Int)
// CHECK: call {{.*}}({{.*}}, [[SET_VALUE]])
// CHECK: let [[DEFAULT_INDEX:%[0-9]+]] : Ptr(Int) = var
// CHECK-NEXT: store([[DEFAULT_INDEX]], 0 : Int)
// CHECK: call {{.*}}({{.*}}, [[DEFAULT_INDEX]])

// The type-adjusted BorrowIn setter parameter is forwarded without loading and copying it.
// CHECK-LABEL: [nameHint("exerciseNonCopyableSetter")]
// CHECK: param [[REPLACEMENT:%[A-Za-z0-9_]+]] : BorrowInParam(%NonCopyableInput
// CHECK: call {{.*}}({{.*}}, [[REPLACEMENT]])
