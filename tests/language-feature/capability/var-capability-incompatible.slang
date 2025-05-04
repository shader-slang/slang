//TEST:SIMPLE(filecheck=CHECK):-target spirv
[require(hlsl)]
struct MyType
{}

//CHECK: error 36107: entrypoint 'f1' uses features that are not available in 'compute' stage for 'spirv' target.
[numthreads(1,1,1)]
void f1()
{
    MyType t;  // compile to spirv should result error here.
}

ConstantBuffer<MyType> t2;
//CHECK: error 36107: entrypoint 'f2' uses features that are not available in 'compute' stage for 'spirv' target.
[numthreads(1,1,1)]
void f2()  // compile to spirv should result error here.
{
    ConstantBuffer<MyType> t3 = t2; 
}
