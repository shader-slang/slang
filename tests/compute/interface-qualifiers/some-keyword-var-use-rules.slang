//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

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

__generic<T>
interface IFooGeneric
{
}

//CHECK-DAG: error 33084
//CHECK-DAG: error 33084
//CHECK-DAG: error 33084
//CHECK-DAG-NOT: error 33084
struct someStructMember1
{
    some IFoo v;
};
struct someStructMember2
{
    some IFooGeneric<float> v;
};
__generic<T>
struct someStructMember3
{
    some IFoo v;
};

//CHECK-DAG: error 33085
static some IFoo globalSomeVariable;

RWStructuredBuffer<int> buffer;
void computeMain()
{

// Not interface type
//CHECK-DAG: error 33083
    some myS bad1;
    
// wrapped `SomeType`
//CHECK-DAG: error 33083
//CHECK-DAG: error 33083
    some IFoo* bad2;
    some Ptr<IFoo> bad3;

// Complex expression `SomeType`
// CHECK-DAG: error 33088
// CHECK-DAG: error 33088
    some Tuple<some IFoo, int> bad4;
    IFooGeneric<some IFoo> bad5;

// `dyn` and `some` is disallowed
//CHECK-DAG: error 33089
    dyn some IFooDyn bad6;

    // good uses of `some`
    some IFoo good1;
    some IFooGeneric<float> good2;
     

    buffer[0] = 0;
}