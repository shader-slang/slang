//TEST:SIMPLE(filecheck=CHECK_SOME_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2025
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -std 2026

// cannot have interface unless `dyn`. Checks if implicit `dyn` assigned to varDecl
//CHECK_SOME_IMPLICIT: error
//CHECK_DYN_IMPLICIT-NOT: error 
dyn interface MaybeDynInterface
{
}

void computeMain()
{
    MaybeDynInterface dynInterfaceVar;
}