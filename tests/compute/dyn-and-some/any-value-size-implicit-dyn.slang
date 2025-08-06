//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_ERROR): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DERROR

// Cannot have dyn interface unless `dyn`. Checks if implicit `dyn` through `anyValueSize` works
// anyValueSize ==> `dyn`

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error

#ifndef ERROR
[anyValueSize(16)]
#endif
interface DynInterface
{
}

void computeMain()
{
    // CHECK_ERROR: ([[# @LINE+1]]): error 33090
    dyn DynInterface dynInterfaceVar;
}