//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// cannot have dyn interface unless `dyn`. Checks if implicit `dyn` through `anyValueSize` works
// anyValueSize ==> `dyn`

// CHECK: OpEntryPoint
// CHECK-NOT: error

[anyValueSize(16)]
interface DynInterface
{
}

void computeMain()
{
    dyn DynInterface dynInterfaceVar;
}