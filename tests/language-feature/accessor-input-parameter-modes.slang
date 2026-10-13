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

void exerciseChainedNonCopyableSetterAssignment(
    inout NonCopyableStorage storage,
    NonCopyableInput first,
    NonCopyableInput second)
{
    // The inner assignment returns the original destination, not the non-copyable value supplied to
    // its setter. The outer assignment must therefore select the same setter without loading or
    // copying either right-hand side.
    (storage.item = first) = second;
}

struct GetterOnlyStorage
{
    int value;

    property int item
    {
        get { return value; }
    }
}

struct CopyableConstRefDestination
{
    property int item
    {
        set(__constref int replacement) {}
    }
}

void exerciseCopyableConstRefSetter()
{
    CopyableConstRefDestination destination = {};
    GetterOnlyStorage source = {43};

    // A copyable explicit `__constref` setter parameter can borrow a temporary containing the
    // getter result. It does not require `GetterOnlyStorage.item` to expose a `ref` accessor.
    destination.item = source.item;
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

// The type-adjusted `BorrowIn` setter parameter is forwarded without loading and copying it.
// CHECK-LABEL: [nameHint("exerciseNonCopyableSetter")]
// CHECK: param [[REPLACEMENT:%[A-Za-z0-9_]+]] : BorrowInParam(%NonCopyableInput
// CHECK: call {{.*}}({{.*}}, [[REPLACEMENT]])

// A chained assignment keeps the direct setter argument mode while it recovers the inner
// assignment's destination for the second call.
// CHECK-LABEL: [nameHint("exerciseChainedNonCopyableSetterAssignment")]
// CHECK: param [[FIRST:%[A-Za-z0-9_]+]] : BorrowInParam(%NonCopyableInput
// CHECK: param [[SECOND:%[A-Za-z0-9_]+]] : BorrowInParam(%NonCopyableInput
// CHECK: call {{.*}}({{.*}}, [[FIRST]])
// CHECK-NEXT: call {{.*}}({{.*}}, [[SECOND]])

// The getter-only `source.item` value is materialized before the copyable explicit-constref setter
// is called.
// CHECK-LABEL: [nameHint("GetterOnlyStorage.item.get")]
// CHECK: func [[SOURCE_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("CopyableConstRefDestination.item.set")]
// CHECK: func [[DESTINATION_SET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("exerciseCopyableConstRefSetter")]
// CHECK: let [[SOURCE_VALUE:%[A-Za-z0-9_]+]] : Int = call [[SOURCE_GET]]
// CHECK: let [[BORROWED_VALUE:%[A-Za-z0-9_]+]] : Ptr(Int) = var
// CHECK: store([[BORROWED_VALUE]], [[SOURCE_VALUE]])
// CHECK: call [[DESTINATION_SET]]({{.*}}, [[BORROWED_VALUE]])
