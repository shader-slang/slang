//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// validate some uses of `some`.

dyn interface IFooDyn
{
}

interface IFoo
{
}

struct myS : IFoo
{
    int v2;
}

// not allowed `some` members
//
// CHECK-DAG: error 33084
// CHECK-DAG: error 33084
struct someStructMember1
{
    some IFoo v;
};
__generic<T>
struct someStructMember3
{
    some IFoo v;
};

// not allowed `some` global
// CHECK-DAG: error 33085
static some IFoo globalSomeVariable;

RWStructuredBuffer<int> buffer;
void computeMain()
{

// Not allowed a `some` non-interface type
// CHECK-DAG: error 33083
    some myS bad1;

// `dyn` and `some` is disallowed
// CHECK-DAG: error 33089
    dyn some IFooDyn bad6;     

    buffer[0] = 0;
}