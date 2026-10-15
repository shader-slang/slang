//TEST:SIMPLE: -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// An accessor can be installed directly as an interface witness only when its complete ParamInfo
// matches the requirement. These implementations deliberately use In where their requirements use
// BorrowIn. Witness synthesis must bridge that ABI difference instead of publishing the source
// accessor directly.

interface IBorrowingProperty
{
    property int item
    {
        get;
        set(__constref int replacement);
    }
}

struct BorrowingPropertyImpl : IBorrowingProperty
{
    int storedValue;

    property int item
    {
        get { return storedValue; }
        set(int replacement) { storedValue = replacement; }
    }
}

interface IBorrowingSubscript
{
    __subscript(__constref int index) -> int
    {
        get;
    }
}

struct BorrowingSubscriptImpl : IBorrowingSubscript
{
    __subscript(int index) -> int
    {
        get { return index; }
    }
}

// The synthesized property setter preserves the requirement's BorrowIn value parameter and loads
// it before forwarding to the implementation's In parameter.
// CHECK-LABEL: [nameHint("BorrowingPropertyImpl")]
// CHECK: func {{.*}} : Func(Void, BorrowInOutParam(%BorrowingPropertyImpl), BorrowInParam(Int,
// CHECK: param [[PROPERTY_VALUE:%[A-Za-z0-9_]+]] : BorrowInParam(Int,
// CHECK: let [[LOADED_PROPERTY_VALUE:%[A-Za-z0-9_]+]] : Int = load([[PROPERTY_VALUE]])
// CHECK: call {{.*}}({{.*}}, [[LOADED_PROPERTY_VALUE]])

// The synthesized subscript getter likewise preserves the requirement's BorrowIn index and loads
// it before forwarding to the implementation's In index.
// CHECK-LABEL: [nameHint("BorrowingSubscriptImpl")]
// CHECK: func {{.*}} : Func(Int, %BorrowingSubscriptImpl, BorrowInParam(Int,
// CHECK: param [[SUBSCRIPT_INDEX:%[A-Za-z0-9_]+]] : BorrowInParam(Int,
// CHECK: let [[LOADED_SUBSCRIPT_INDEX:%[A-Za-z0-9_]+]] : Int = load([[SUBSCRIPT_INDEX]])
// CHECK: call {{.*}}({{.*}}, [[LOADED_SUBSCRIPT_INDEX]])
