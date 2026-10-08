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

// A wrapper with a writable receiver can safely forward to an implementation with a nonmutating
// receiver. The following property and subscript declarations exercise that adaptation.
interface IMutatingPropertyGetter
{
    property int item
    {
        [mutating] get;
    }
}

struct NonmutatingPropertyGetterImpl : IMutatingPropertyGetter
{
    property int item
    {
        [nonmutating] get { return 0; }
    }
}

interface IMutatingSubscriptGetter
{
    __subscript(int index) -> int
    {
        [mutating] get;
    }
}

struct NonmutatingSubscriptGetterImpl : IMutatingSubscriptGetter
{
    __subscript(int index) -> int
    {
        [nonmutating] get { return index; }
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

// The implementation getter retains its nonmutating, by-value receiver ABI. The synthesized
// witness wrapper keeps the requirement's writable receiver ABI, loads that receiver, and forwards
// the resulting value to the implementation getter.
// CHECK-LABEL: [nameHint("NonmutatingPropertyGetterImpl")]
// CHECK: [nameHint("NonmutatingPropertyGetterImpl.item.get")]
// CHECK-NEXT: func [[PROPERTY_GETTER:%[A-Za-z0-9_]+]] : Func(Int, %NonmutatingPropertyGetterImpl)
// CHECK: func {{.*}} : Func(Int, BorrowInOutParam(%NonmutatingPropertyGetterImpl))
// CHECK: param [[PROPERTY_THIS:%[A-Za-z0-9_]+]] : BorrowInOutParam(%NonmutatingPropertyGetterImpl)
// CHECK: let [[LOADED_PROPERTY_THIS:%[A-Za-z0-9_]+]] : %NonmutatingPropertyGetterImpl =
// CHECK-SAME: load([[PROPERTY_THIS]])
// CHECK: call [[PROPERTY_GETTER]]([[LOADED_PROPERTY_THIS]])

// The subscript getter performs the same adaptation while forwarding its ordinary index parameter
// unchanged.
// CHECK-LABEL: [nameHint("NonmutatingSubscriptGetterImpl")]
// CHECK: [nameHint("NonmutatingSubscriptGetterImpl.operator[].get")]
// CHECK-NEXT: func [[SUBSCRIPT_GETTER:%[A-Za-z0-9_]+]] :
// CHECK-SAME: Func(Int, %NonmutatingSubscriptGetterImpl, Int)
// CHECK: func {{.*}} : Func(Int, BorrowInOutParam(%NonmutatingSubscriptGetterImpl), Int)
// CHECK: param [[SUBSCRIPT_THIS:%[A-Za-z0-9_]+]] :
// CHECK-SAME: BorrowInOutParam(%NonmutatingSubscriptGetterImpl)
// CHECK: param [[ORDINARY_SUBSCRIPT_INDEX:%[A-Za-z0-9_]+]] : Int
// CHECK: let [[LOADED_SUBSCRIPT_THIS:%[A-Za-z0-9_]+]] : %NonmutatingSubscriptGetterImpl =
// CHECK-SAME: load([[SUBSCRIPT_THIS]])
// CHECK: call [[SUBSCRIPT_GETTER]]([[LOADED_SUBSCRIPT_THIS]], [[ORDINARY_SUBSCRIPT_INDEX]])
