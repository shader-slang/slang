//TEST:SIMPLE(filecheck=CHECK):-target spirv
[require(hlsl)]
struct MyType
{}

//CHECK: error[E36107]
[numthreads(1,1,1)]
void f1()
{
    MyType t;  // compile to spirv should result error here.
}

ConstantBuffer<MyType> t2;
//CHECK: error[E36107]
[numthreads(1,1,1)]
void f2()  // compile to spirv should result error here.
{
    ConstantBuffer<MyType> t3 = t2; 
}
