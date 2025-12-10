// simple-numeric-test.slang
// Example: Generate tests for multiple numeric types
//
// Generates to: tests/fiddle/numeric-ops/
// Note: Skips 'double' for CPU tests (known limitation with -output-using-type)

#if 0 // FIDDLE TEMPLATE:
%local types = {"int", "uint", "float"}
%for _, typ in ipairs(types) do
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<$typ> outputBuffer;

// Test for type: $typ

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    $typ a = 10;
    $typ b = 5;
    $typ result = a + b;
    outputBuffer[0] = result;
}

// CHECK: 15
%    emit_test_file("numeric-ops/add-" .. typ .. ".slang")
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
