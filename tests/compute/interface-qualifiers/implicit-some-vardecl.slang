//TEST:SIMPLE(filecheck=CHECK_SOME_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2025
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -std 2026

// CHECK_DYN_IMPLICIT: OpEntryPoint

// cannot use `some` struct members, only fails if implicit `some`
// CHECK_SOME_IMPLICIT: error
// CHECK_DYN_IMPLICIT-NOT: error 
dyn interface MyInterface
{
}
struct MyStruct
{
    MyInterface interface;
}

void computeMain()
{
}