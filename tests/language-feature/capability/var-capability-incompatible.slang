//TEST:SIMPLE(filecheck=CHECK1):-target spirv
//TEST:SIMPLE(filecheck=CHECK2):-target spirv

[require(hlsl)]
struct MyType
{}

//CHECK1: error 36107: entrypoint 'f1' uses features that are not available in 'compute' stage for 'spirv' target.
[numthreads(1,1,1)]
void f1()
{
    MyType t;  // compile to spirv should result error here.
}

[numthreads(1,1,1)]
void f2(uniform ConstantBuffer<MyType> t)  // compile to spirv should result error here.
{
     
}

//CHECK2: error 36107: entrypoint 'f2' uses features that are not available in 'compute' stage for 'spirv' target.
ConstantBuffer<MyType> t2; // error here too.