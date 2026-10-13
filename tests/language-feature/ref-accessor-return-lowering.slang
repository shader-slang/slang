//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A `ref` accessor returns an address even though its source-level result type names the value in
// that storage. A noncopyable value normally uses an implicit output parameter when returned by
// value, but a `ref` accessor must instead return a single pointer. These cases cover both
// property and subscript accessors. The subscript case also ensures that collecting its index
// parameter does not add an output parameter for either the subscript or its accessor.

[__NonCopyableType]
struct NonCopyableValue
{
    int value;
}

struct RefAccessorStorage
{
    NonCopyableValue nonCopyablePropertyBacking;
    NonCopyableValue nonCopyableSubscriptBacking;

    property NonCopyableValue nonCopyableProperty
    {
        [mutating]
        ref { return nonCopyablePropertyBacking; }
    }

    __subscript(int index) -> NonCopyableValue
    {
        [mutating]
        ref { return nonCopyableSubscriptBacking; }
    }
}

// CHECK-LABEL: [nameHint("RefAccessorStorage.nonCopyableProperty.ref")]
// CHECK-NEXT: func {{%[A-Za-z0-9_]+}} :
// CHECK-SAME: Func(Ptr(%NonCopyableValue), BorrowInOutParam(%RefAccessorStorage))
// CHECK: let [[NONCOPYABLE_PROPERTY_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%NonCopyableValue) =
// CHECK-SAME: get_field_addr
// CHECK-NOT: load
// CHECK: return_val([[NONCOPYABLE_PROPERTY_ADDRESS]])

// CHECK-LABEL: [nameHint("RefAccessorStorage.operator[].ref")]
// CHECK-NEXT: func {{%[A-Za-z0-9_]+}} :
// CHECK-SAME: Func(Ptr(%NonCopyableValue), BorrowInOutParam(%RefAccessorStorage), Int)
// CHECK: let [[NONCOPYABLE_SUBSCRIPT_ADDRESS:%[A-Za-z0-9_]+]] : Ptr(%NonCopyableValue) =
// CHECK-SAME: get_field_addr
// CHECK-NOT: load
// CHECK: return_val([[NONCOPYABLE_SUBSCRIPT_ADDRESS]])
