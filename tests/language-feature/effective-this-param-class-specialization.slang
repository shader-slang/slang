//TEST:SIMPLE: -no-codegen -Wno-38109
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir -Wno-38109

// Specializing an interface requirement's `This` type to a class changes only its effective
// parameter type. Its declaration-derived mode remains part of the requirement ABI. An ordinary
// class method uses In, so the non-In requirements below exercise synthesized thunks that preserve
// the requirement mode while forwarding to the class implementation.

interface IMutatingRequirement
{
    [mutating]
    void mutate();
}

class MutatingClass : IMutatingRequirement
{
    void mutate() {}
}

interface IConstRefRequirement
{
    [constref]
    int read();
}

class ConstRefClass : IConstRefRequirement
{
    int read()
    {
        return 0;
    }
}

interface IRefRequirement
{
    [__ref]
    void update();
}

class RefClass : IRefRequirement
{
    void update() {}
}

interface IFieldBackedProperty
{
    property value : int
    {
        get;
        set;
    }
}

// The synthesized accessors that adapt a field to this property requirement likewise preserve the
// receiver modes chosen for the requirement without treating source-level receiver-mode specifiers
// as valid on class members.
class FieldBackedPropertyClass : IFieldBackedProperty
{
    int value;
}

interface INonmutatingFieldBackedProperty
{
    property value : int
    {
        get;
        [nonmutating] set;
    }
}

// The effective `this` parameter's `In` mode promises that the setter does not replace the class
// reference. It can still mutate a field of the object named by that reference, so field-backed
// synthesis must give the wrapper body a writable `this` expression without changing the ABI.
class NonmutatingFieldBackedPropertyClass : INonmutatingFieldBackedProperty
{
    int value;
}

// A generic requirement chooses its parameter modes from the unspecialized declaration. When
// witness synthesis applies a non-copyable specialization, the synthesized setter must preserve
// that declaration-time In mode instead of re-defaulting the specialized value type to BorrowIn.
[__NonCopyableType]
struct NonCopyablePropertyValue
{
    int value;
}

interface IGenericFieldBackedProperty<T>
{
    property value : T
    {
        get;
        set;
    }
}

class GenericFieldBackedPropertyClass
    : IGenericFieldBackedProperty<NonCopyablePropertyValue>
{
    NonCopyablePropertyValue value;
}

// Each thunk keeps the receiver mode from the unspecialized interface requirement even though its
// specialized receiver value type is a class.
// CHECK-LABEL: [nameHint("MutatingClass.$__syn_mutate")]
// CHECK: func {{.*}} : Func(Void, BorrowInOutParam(%MutatingClass))
// CHECK-LABEL: [nameHint("ConstRefClass.$__syn_read")]
// CHECK: func {{.*}} : Func(Int, BorrowInParam(%ConstRefClass,
// CHECK-LABEL: [nameHint("RefClass.$__syn_update")]
// CHECK: func {{.*}} : Func(Void, RefParam(%RefClass,
// CHECK: func {{.*}} : Func(Void, BorrowInOutParam(%FieldBackedPropertyClass), Int)
// CHECK: func {{.*}} : Func(Void, %NonmutatingFieldBackedPropertyClass, Int)

// The synthesized setter's value parameter remains In even though its substituted value type is
// non-copyable. A mode re-adjustment bug would wrap the final parameter in BorrowInParam.
// CHECK-LABEL: [nameHint("GenericFieldBackedPropertyClass")]
// CHECK: func {{.*}} : Func(Void, BorrowInOutParam(%GenericFieldBackedPropertyClass),
// CHECK-SAME: %NonCopyablePropertyValue)
