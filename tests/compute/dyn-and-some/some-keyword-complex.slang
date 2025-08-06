//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// Not allowed complex expressions with `some`

interface IFoo
{
}

__generic<T>
interface IFooGeneric
{
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // Cannot have a `some some` type
    // CHECK-DAG: ([[# @LINE+1]]): error 33083:
    some some IFoo bad1;

    // Ptr of `some` is disallowed
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    some IFoo* bad2;
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    some Ptr<IFoo> bad3;

    // Complex expression around a `some` is disallowed
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    some Tuple<some IFoo, int> bad4;

    // Generic around a `some` is disallowed
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    IFooGeneric<some IFoo> bad5;

    // Array around a `some` is disallowed
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    some IFoo bad6[2];

    // Check if implicit types also abide to the
    // non-complex type rules
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    Tuple<IFoo, int> bad7;

    // This expression does not make any sense, so
    // error.
    // CHECK-DAG: ([[# @LINE+1]]): error 33085:
    some Array<IFoo, 3> bad8;

    buffer[0] = 0;
}