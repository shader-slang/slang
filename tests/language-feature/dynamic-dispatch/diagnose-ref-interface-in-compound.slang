// Tests disabled: diagnostic 52010 not yet implemented (see #9906).
// Enable when __ref/__constref dynamic dispatch diagnostic is added.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cuda

// Generic compound types (Tuple, Optional) are checked recursively for
// interface members, just like structs and arrays. If any type argument
// of the compound type is or contains a dynamic-dispatch interface, the
// whole parameter is rejected under __ref/__constref because AnyValue
// packing is incompatible with reference semantics.

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2; }
}

// Tuple with an interface element: the interface slot is AnyValue-packed.
typealias TupleWithInterface = Tuple<IFoo, float>;

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type '{{.*}}' cannot be used in dynamic dispatch context.
float processTupleRef(__ref TupleWithInterface t)
{
    return t._0.getValue();
}

// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type '{{.*}}' cannot be used in dynamic dispatch context.
float processTupleConstref(__constref TupleWithInterface t)
{
    return t._0.getValue() + t._1;
}

// Optional wrapping an interface: the contained value is AnyValue-packed.
// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type '{{.*}}' cannot be used in dynamic dispatch context.
float processOptionalRef(__ref Optional<IFoo> opt)
{
    if (opt.hasValue)
        return opt.value.getValue();
    return 0.0;
}

// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type '{{.*}}' cannot be used in dynamic dispatch context.
float processOptionalConstref(__constref Optional<IFoo> opt)
{
    if (opt.hasValue)
        return opt.value.getValue();
    return 0.0;
}

// Nested compounds: the check traverses Tuple -> Optional -> IFoo.
typealias NestedCompound = Tuple<Optional<IFoo>, int>;

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type '{{.*}}' cannot be used in dynamic dispatch context.
float processNestedRef(__ref NestedCompound nc)
{
    if (nc._0.hasValue)
        return nc._0.value.getValue();
    return float(nc._1);
}

// A named struct containing a Tuple with interface members is also
// caught, since the struct-field check recurses into the Tuple.
struct TupleContainer
{
    Tuple<IFoo, IFoo> pair;
}

// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type 'TupleContainer' cannot be used in dynamic dispatch context.
float processTupleContainer(__constref TupleContainer c)
{
    return c.pair._0.getValue() + c.pair._1.getValue();
}

IFoo factory(uint id)
{
    if (id == 0) return ImplA(1.0);
    return ImplB(2.0);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo obj = factory(dispatchThreadID.x);

    TupleWithInterface tup = Tuple<IFoo, float>(obj, 5.0);
    outputBuffer[0] = processTupleRef(tup);
    outputBuffer[1] = processTupleConstref(tup);

    Optional<IFoo> opt = Optional<IFoo>(obj);
    outputBuffer[2] = processOptionalRef(opt);
    outputBuffer[3] = processOptionalConstref(opt);

    NestedCompound nested = Tuple<Optional<IFoo>, int>(opt, 10);
    outputBuffer[4] = processNestedRef(nested);

    TupleContainer tc;
    tc.pair = Tuple<IFoo, IFoo>(obj, obj);
    outputBuffer[5] = processTupleContainer(tc);
}
