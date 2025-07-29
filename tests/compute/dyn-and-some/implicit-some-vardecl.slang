//TEST:SIMPLE(filecheck=CHECK_SOME_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2025

// CHECK_DYN_IMPLICIT: OpEntryPoint
// CHECK_DYN_IMPLICIT-NOT: error

dyn interface MyInterface
{
}
struct MyStruct
{
    // Cannot use `some` struct members, fails if implicit `some`
    // CHECK_SOME_IMPLICIT: ([[# @LINE+1]]): error 33085
    MyInterface interface;
}

void computeMain()
{
}