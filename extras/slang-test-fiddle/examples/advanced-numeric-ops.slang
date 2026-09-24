// advanced-numeric-ops.slang
// Example: Generate tests for multiple types and operations
//
// Generates to: <output-dir>/numeric-permutations/
// Note: Skips 'double' for CPU tests (known limitation with -output-using-type)

#if 0 // FIDDLE TEMPLATE:
%local types = {
%    {name="int", example=10},
%    {name="float", example=10.0}
%}
%
%local ops = {
%    {op="+", name="add", result=15},
%    {op="-", name="sub", result=5},
%    {op="*", name="mul", result=50}
%}
%
%for _, typ in ipairs(types) do
%    for _, op in ipairs(ops) do
%        -- Extract table fields to locals for $splicing
%        local typeName = typ.name
%        local typeExample = typ.example
%        local opSymbol = op.op
%        local opResult = op.result
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<$typeName> outputBuffer;

// Test: $typeName $opSymbol operator

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    $typeName a = $typeExample;
    $typeName b = 5;
    $typeName result = a $opSymbol b;
    outputBuffer[0] = result;
}

// CHECK: $opResult
%        emit_test_file("numeric-permutations/" .. typ.name .. "-" .. op.name .. ".slang")
%    end
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
