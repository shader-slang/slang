//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// not allowed complex expressions with `some`

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

// wrapped `SomeType`
// CHECK-DAG: error 33083
// CHECK-DAG: error 33083
    some IFoo* bad2;
    some Ptr<IFoo> bad3;

// Complex expression `SomeType`
// CHECK-DAG: error 33088
// CHECK-DAG: error 33088
    some Tuple<some IFoo, int> bad4;
    IFooGeneric<some IFoo> bad5;

    buffer[0] = 0;
}