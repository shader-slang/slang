//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

//CHECK-DAG: error 33090
//CHECK-DAG-NOT: error 33090
interface NonDynInterface
{
}
dyn interface DynInterface
{
}
static dyn NonDynInterface nonDynInterfaceVar;
static dyn DynInterface dynInterfaceVar;

RWStructuredBuffer<int> buffer;
void computeMain()
{
    buffer[0] = 0;
}