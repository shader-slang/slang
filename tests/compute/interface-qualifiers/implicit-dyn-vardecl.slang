//TEST:SIMPLE(filecheck=CHECK_SOME_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2025
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -std 2026

// CHECK_DYN_IMPLICIT: OpEntryPoint

// cannot have interface varDecl unless `dyn interface`. Checks if implicit `dyn` is assigned
// CHECK_SOME_IMPLICIT: error
// CHECK_DYN_IMPLICIT-NOT: error 
interface MaybeDynInterface
{
}

void computeMain()
{
    dyn MaybeDynInterface dynInterfaceVar;
}