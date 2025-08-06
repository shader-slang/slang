//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// Ensure `dyn` is only allowed with `dyn interface`

interface NonDynInterface
{
}
dyn interface DynInterface
{
}

// CHECK: ([[# @LINE+2]]): error 33090
// CHECK-NOT: error
static dyn NonDynInterface nonDynInterfaceVar;
static dyn DynInterface dynInterfaceVar;

RWStructuredBuffer<int> buffer;
void computeMain()
{
    dyn DynInterface data;
    buffer[0] = 0;
}