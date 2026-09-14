// Regression test for https://github.com/shader-slang/slang/issues/12731. A callable-data global
// must remain addressable even when the source payload is empty because OpExecuteCallableKHR takes
// a pointer to a real CallableDataKHR variable. Its physical backing must not change the nominal
// empty type: the same type is also reused in a buffer record whose value field stays at offset 0.

//TEST:SIMPLE(filecheck=SPIRV): -entry rgen -stage raygeneration -target spirv-assembly -emit-spirv-directly

struct SharedEmpty
{
}

struct LayoutSensitiveRecord
{
    SharedEmpty marker;
    int value;
}

ConstantBuffer<LayoutSensitiveRecord> record;
RWStructuredBuffer<int> output;

[shader("raygeneration")]
void rgen()
{
    SharedEmpty data;
    CallShader(0, data);
    output[0] = record.value;
}

// The callable ABI uses a role-local scalar backing store. The ordinary record still contains only
// `value`; mutating SharedEmpty itself would leave another member in this physical layout.
// SPIRV-DAG: OpMemberName %LayoutSensitiveRecord 0 "value"
// SPIRV-DAG: OpMemberDecorate %LayoutSensitiveRecord 0 Offset 0
// SPIRV-DAG: %LayoutSensitiveRecord = OpTypeStruct %int
// SPIRV-DAG: %[[CALLABLE_PTR:[A-Za-z0-9_]+]] = OpTypePointer CallableDataKHR %int
// SPIRV-DAG: %[[CALLABLE_STORAGE:[A-Za-z0-9_]+]] = OpVariable %[[CALLABLE_PTR]] CallableDataKHR
// SPIRV: OpExecuteCallableKHR
