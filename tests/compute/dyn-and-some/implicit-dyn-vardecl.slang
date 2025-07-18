//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -
//TEST:SIMPLE(filecheck=CHECK_DYN_IMPLICIT): -target spirv -stage compute -entry computeMain -lang slang -std 2025

// CHECK: OpEntryPoint
// CHECK-NOT: error 
interface NotDynInterface
{
}

void computeMain()
{
    // -std 2026 would make this a `some NotDynInterface`, no error.
    //
    // -std 2025 would make this a `dyn NotDynInterface`. We do not 
    // error in this case to stay backwards compatible.
    NotDynInterface dynInterfaceVar;
}