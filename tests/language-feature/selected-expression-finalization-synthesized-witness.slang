//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// Conformance checking synthesizes an adapter when the receiver mode or an ordinary parameter mode
// of a concrete method differs from its interface requirement. While semantic checking builds the
// adapter body, it must retain each selected expression until exactly one surrounding call,
// conversion, or assignment consumes that expression. The checks below cover the resulting adapter
// bodies and witness-table entries for a generic throwing method, a property, and a subscript.

typealias Callback = functype() -> int;

int defaultCallback()
{
    return 0;
}

enum WitnessError
{
    failure,
}

// The interface requires a mutating receiver, while the concrete generic method is non-mutating.
// The adapter must specialize the concrete method, preserve both the error and success cases, and
// rebuild the successful result from the returned function value.
interface IThrowingFunctionFactory
{
    [mutating]
    Callback makeCallback<T>() throws WitnessError;
}

struct ThrowingFunctionFactory : IThrowingFunctionFactory
{
    Callback makeCallback<T>() throws WitnessError
    {
        return defaultCallback;
    }
}

// Both interface accessors require a mutating receiver, while the concrete getter is non-mutating.
// The interface setter also borrows its value parameter while the concrete setter takes the value
// directly. Conformance checking therefore synthesizes separate getter and setter adapters.
interface IFunctionProperty
{
    property Callback callback
    {
        [mutating] get;
        [mutating] set(__constref Callback value);
    }
}

struct FunctionProperty : IFunctionProperty
{
    Callback storedCallback;

    property Callback callback
    {
        [nonmutating] get { return storedCallback; }
        [mutating] set(Callback value) { storedCallback = value; }
    }
}

// The subscript accessors need the same receiver and value conversions as the property accessors.
// Their adapters must also load and forward the borrowed index parameter.
interface IFunctionSubscript
{
    __subscript(__constref int index) -> Callback
    {
        [mutating] get;
        [mutating] set(__constref Callback value);
    }
}

struct FunctionSubscript : IFunctionSubscript
{
    Callback storedCallback;

    __subscript(int index) -> Callback
    {
        [nonmutating] get { return storedCallback; }
        [mutating] set(Callback value) { storedCallback = value; }
    }
}

// The throwing adapter receives a mutable reference, loads the concrete receiver, and calls the
// concrete generic method specialized with the interface method's type argument. It then rebuilds
// either the error result or the successful result containing the returned function value.
// CHECK-LABEL: [export("{{.*}}syn_umakeCallback{{.*}}")]
// CHECK: generic [[THROW_ADAPTER:%[A-Za-z0-9_]+]] :
// CHECK: param [[TYPE_ARGUMENT:%[A-Za-z0-9_]+]] : type_t
// CHECK: [nameHint("ThrowingFunctionFactory.$__syn_makeCallback")]
// CHECK: func {{%[A-Za-z0-9_]+}} : Func({{.*}},
// CHECK-SAME: BorrowInOutParam(%ThrowingFunctionFactory))
// CHECK: param [[THROW_THIS:%[A-Za-z0-9_]+]] :
// CHECK-SAME: BorrowInOutParam(%ThrowingFunctionFactory)
// CHECK: let [[THROW_VALUE:%[A-Za-z0-9_]+]] : %ThrowingFunctionFactory = load([[THROW_THIS]])
// CHECK: let [[THROWING_RESULT:%[A-Za-z0-9_]+]] : {{.*}} = call specialize({{.*}},
// CHECK-SAME: [[TYPE_ARGUMENT]])
// CHECK-SAME: ([[THROW_VALUE]])
// CHECK: let [[IS_ERROR:%[A-Za-z0-9_]+]] : Bool = isResultError([[THROWING_RESULT]])
// CHECK: ifElse([[IS_ERROR]], [[ERROR_BLOCK:%[A-Za-z0-9_]+]],
// CHECK-SAME: [[VALUE_BLOCK:%[A-Za-z0-9_]+]], [[VALUE_BLOCK]])
// CHECK: block [[ERROR_BLOCK]]:
// CHECK: let [[ERROR:%[A-Za-z0-9_]+]] : {{.*}} = getResultError([[THROWING_RESULT]])
// CHECK: let [[ERROR_RESULT:%[A-Za-z0-9_]+]] : {{.*}} = makeResultError([[ERROR]])
// CHECK: return_val([[ERROR_RESULT]])
// CHECK: block [[VALUE_BLOCK]]:
// CHECK: let [[CALLBACK:%[A-Za-z0-9_]+]] : Func(Int) = getResultValue([[THROWING_RESULT]])
// CHECK: let [[CALLBACK_RESULT:%[A-Za-z0-9_]+]] : {{.*}} = makeResultValue([[CALLBACK]])
// CHECK: return_val([[CALLBACK_RESULT]])
// CHECK: witness_table_entry({{.*}},[[THROW_ADAPTER]])

// The synthesized property getter loads the concrete receiver before forwarding to the
// non-mutating getter. The synthesized setter instead preserves the receiver reference, loads the
// interface's borrowed value parameter, and forwards both values to the concrete setter.
// CHECK-LABEL: [nameHint("FunctionProperty.callback.get")]
// CHECK: func [[PROPERTY_GET:%[A-Za-z0-9_]+]] :
// CHECK: [export("{{.*}}syn_uproperty_ucallback{{.*}}")]
// CHECK: func [[PROPERTY_GET_ADAPTER:%[A-Za-z0-9_]+]] : Func(Func(Int),
// CHECK-SAME: BorrowInOutParam(%FunctionProperty))
// CHECK: param [[PROPERTY_GET_THIS:%[A-Za-z0-9_]+]] : BorrowInOutParam(%FunctionProperty)
// CHECK: let [[PROPERTY_VALUE:%[A-Za-z0-9_]+]] : %FunctionProperty = load([[PROPERTY_GET_THIS]])
// CHECK: let [[PROPERTY_RESULT:%[A-Za-z0-9_]+]] : Func(Int) =
// CHECK-SAME: call [[PROPERTY_GET]]([[PROPERTY_VALUE]])
// CHECK: return_val([[PROPERTY_RESULT]])
// CHECK-LABEL: [nameHint("FunctionProperty.callback.set")]
// CHECK: func [[PROPERTY_SET:%[A-Za-z0-9_]+]] :
// CHECK: [export("{{.*}}syn_uproperty_ucallback{{.*}}")]
// CHECK: func [[PROPERTY_SET_ADAPTER:%[A-Za-z0-9_]+]] : Func(Void,
// CHECK-SAME: BorrowInOutParam(%FunctionProperty), BorrowInParam(Func(Int){{.*}}))
// CHECK: param [[PROPERTY_SET_THIS:%[A-Za-z0-9_]+]] : BorrowInOutParam(%FunctionProperty),
// CHECK: param [[PROPERTY_SET_ARG:%[A-Za-z0-9_]+]] : BorrowInParam(Func(Int){{.*}})
// CHECK: let [[PROPERTY_ARG:%[A-Za-z0-9_]+]] : Func(Int) = load([[PROPERTY_SET_ARG]])
// CHECK: call [[PROPERTY_SET]]([[PROPERTY_SET_THIS]], [[PROPERTY_ARG]])
// CHECK: witness_table_entry({{.*}},[[PROPERTY_GET_ADAPTER]])
// CHECK: witness_table_entry({{.*}},[[PROPERTY_SET_ADAPTER]])

// The synthesized subscript accessors apply the same receiver and value conversions. They also
// load and forward the borrowed index parameter before calling each concrete accessor.
// CHECK-LABEL: [nameHint("FunctionSubscript.operator[].get")]
// CHECK: func [[SUBSCRIPT_GET:%[A-Za-z0-9_]+]] :
// CHECK: [export("{{.*}}syn_uoperator{{.*}}")]
// CHECK: func [[SUBSCRIPT_GET_ADAPTER:%[A-Za-z0-9_]+]] : Func(Func(Int),
// CHECK-SAME: BorrowInOutParam(%FunctionSubscript), BorrowInParam(Int{{.*}}))
// CHECK: param [[SUBSCRIPT_GET_THIS:%[A-Za-z0-9_]+]] : BorrowInOutParam(%FunctionSubscript),
// CHECK: param [[SUBSCRIPT_GET_INDEX:%[A-Za-z0-9_]+]] : BorrowInParam(Int{{.*}})
// CHECK: let [[SUBSCRIPT_VALUE:%[A-Za-z0-9_]+]] : %FunctionSubscript =
// CHECK-SAME: load([[SUBSCRIPT_GET_THIS]])
// CHECK: let [[SUBSCRIPT_INDEX:%[A-Za-z0-9_]+]] : Int = load([[SUBSCRIPT_GET_INDEX]])
// CHECK: let [[SUBSCRIPT_RESULT:%[A-Za-z0-9_]+]] : Func(Int) =
// CHECK-SAME: call [[SUBSCRIPT_GET]]([[SUBSCRIPT_VALUE]], [[SUBSCRIPT_INDEX]])
// CHECK: return_val([[SUBSCRIPT_RESULT]])
// CHECK-LABEL: [nameHint("FunctionSubscript.operator[].set")]
// CHECK: func [[SUBSCRIPT_SET:%[A-Za-z0-9_]+]] :
// CHECK: [export("{{.*}}syn_uoperator{{.*}}")]
// CHECK: func [[SUBSCRIPT_SET_ADAPTER:%[A-Za-z0-9_]+]] : Func(Void,
// CHECK-SAME: BorrowInOutParam(%FunctionSubscript), BorrowInParam(Int,
// CHECK-SAME: BorrowInParam(Func(Int)
// CHECK: param [[SUBSCRIPT_SET_THIS:%[A-Za-z0-9_]+]] : BorrowInOutParam(%FunctionSubscript),
// CHECK: param [[SUBSCRIPT_SET_INDEX:%[A-Za-z0-9_]+]] : BorrowInParam(Int{{.*}}),
// CHECK: param [[SUBSCRIPT_SET_VALUE:%[A-Za-z0-9_]+]] : BorrowInParam(Func(Int){{.*}})
// CHECK: let [[SET_INDEX:%[A-Za-z0-9_]+]] : Int = load([[SUBSCRIPT_SET_INDEX]])
// CHECK: let [[SET_VALUE:%[A-Za-z0-9_]+]] : Func(Int) = load([[SUBSCRIPT_SET_VALUE]])
// CHECK: call [[SUBSCRIPT_SET]]([[SUBSCRIPT_SET_THIS]], [[SET_INDEX]], [[SET_VALUE]])
// CHECK: witness_table_entry({{.*}},[[SUBSCRIPT_GET_ADAPTER]])
// CHECK: witness_table_entry({{.*}},[[SUBSCRIPT_SET_ADAPTER]])
