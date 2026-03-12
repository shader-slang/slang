//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -profile cs_6_0
interface IFoo
{
    int doThing();
}

// CHECK: error[E45001]: unresolved external symbol
extern struct Foo : IFoo;
// CHECK: error[E45001]: unresolved external symbol
extern static const int c;

RWStructuredBuffer<int> output;

void main()
{
    Foo f;
    output[0] = c + f.doThing();
}