//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_ERROR): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DERROR

// cannot have dyn interface unless `dyn`. Checks if implicit `dyn` through `anyValueSize` works
// anyValueSize ==> `dyn`

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error

// CHECK_ERROR: error

#ifndef ERROR
[anyValueSize(16)]
#endif
interface DynInterface
{
}

void computeMain()
{
    dyn DynInterface dynInterfaceVar;
}