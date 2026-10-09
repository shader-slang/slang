//TEST:SIMPLE(filecheck=CHECK): -target spirv -g3 -O0
//TEST:SIMPLE(filecheck=NOSCOPE): -target spirv -g3 -O0
RWStructuredBuffer<int> outputBuffer;

// Function with a single basic block
[ForceInline]
int basicBlockFunc(int value1, int value2)
{
    // Simple operation that should be inlined
    return value1 * 2 + value2;
}

// Function with multiple basic blocks
[ForceInline]
int multipleBlockFunc(int value1, int value2)
{
    int result = value1 * 2;

    // Add a condition to create multiple basic blocks
    if (value1 > value2)
    {
        result += value2 * 3;
    }
    else
    {
        result -= value2;
    }

    return result;
}

// Test case a: Multiple calls to basic block function
int testMultipleBasicBlockCalls()
{
    int result = basicBlockFunc(10, 5);
    result += basicBlockFunc(20, 15);
    result += basicBlockFunc(30, 25);
    return result;
}

// Test case b: Multiple calls to multiple block function
int testMultipleBlockCalls()
{
    int result = multipleBlockFunc(10, 5);
    result += multipleBlockFunc(20, 25);
    result += multipleBlockFunc(30, 25);
    return result;
}

// Test case c: One call to basic block, other instructions, another call to basic block
int testBasicBlockWithInstructions()
{
    int result = basicBlockFunc(10, 5);

    // Some other instructions
    result *= 2;
    result += 10;

    result += basicBlockFunc(20, 15);
    return result;
}

// Test case d: One call to multiple block func, other instructions, another call to multiple block func
int testMultipleBlockWithInstructions()
{
    int result = multipleBlockFunc(10, 5);

    // Some other instructions
    result *= 2;
    result += 10;

    result += multipleBlockFunc(20, 25);
    return result;
}

// Test case e: One call to basic block, other instructions, call to multiple block
int testBasicToMultipleBlock()
{
    int result = basicBlockFunc(10, 5);

    // Some other instructions
    result *= 2;
    result += 10;

    result += multipleBlockFunc(20, 25);
    return result;
}

// Additional test case: Mixed calls with condition
int testMixedCallsWithCondition()
{
    int result = basicBlockFunc(15, 7);
    if (result > 30)
    {
        result += multipleBlockFunc(8, 3);
    }
    else
    {
        result += multipleBlockFunc(12, 9);
    }
    return result;
}
[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Call all test functions directly and store results in different indices

    // Test case a: Multiple calls to basic block function
    outputBuffer[0] = testMultipleBasicBlockCalls();

    // Test case b: Multiple calls to multiple block function
    outputBuffer[1] = testMultipleBlockCalls();

    // Test case c: One call to basic block, other instructions, another call to basic block
    outputBuffer[2] = testBasicBlockWithInstructions();

    // Test case d: One call to multiple block func, other instructions, another call to multiple block func
    outputBuffer[3] = testMultipleBlockWithInstructions();

    // Test case e: One call to basic block, other instructions, call to multiple block
    outputBuffer[4] = testBasicToMultipleBlock();

    // Additional test case: Mixed calls with condition
    outputBuffer[5] = testMixedCallsWithCondition();

    // Set any remaining indices to 0
    if (dispatchThreadID.x >= 6)
    {
        outputBuffer[dispatchThreadID.x] = 0;
    }
}


// NOSCOPE-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope

// Every inlined call must restore its own caller before updating that caller's
// result. Matching the inline scope first excludes block-entry scope markers.
// CHECK-DAG: %[[BASIC_NAME:[0-9]+]] = OpString "testMultipleBasicBlockCalls"
// CHECK-DAG: %[[BASIC:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[BASIC_NAME]]
// CHECK-DAG: %[[MULTI_NAME:[0-9]+]] = OpString "testMultipleBlockCalls"
// CHECK-DAG: %[[MULTI:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MULTI_NAME]]
// CHECK-DAG: %[[BASIC_INST_NAME:[0-9]+]] = OpString "testBasicBlockWithInstructions"
// CHECK-DAG: %[[BASIC_INST:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[BASIC_INST_NAME]]
// CHECK-DAG: %[[MULTI_INST_NAME:[0-9]+]] = OpString "testMultipleBlockWithInstructions"
// CHECK-DAG: %[[MULTI_INST:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MULTI_INST_NAME]]
// CHECK-DAG: %[[MIXED_NAME:[0-9]+]] = OpString "testBasicToMultipleBlock"
// CHECK-DAG: %[[MIXED:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MIXED_NAME]]
// CHECK-DAG: %[[COND_NAME:[0-9]+]] = OpString "testMixedCallsWithCondition"
// CHECK-DAG: %[[COND:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[COND_NAME]]
// CHECK-DAG: %[[COND_BRANCH:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock %{{[0-9]+}} %uint_95 %uint_9 %[[COND]]
// CHECK-DAG: %[[COND_CALL:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_95 %[[COND_BRANCH]]{{$}}

// CHECK-LABEL: %testMultipleBasicBlockCalls = OpFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[BASIC]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[BASIC_RESULT:result[0-9_]*]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[BASIC]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[BASIC_RESULT]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[BASIC]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[BASIC_RESULT]] %{{[a-zA-Z0-9_]+}}

// CHECK-LABEL: %testMultipleBlockCalls = OpFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MULTI_RESULT:result[0-9_]*]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MULTI_RESULT]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MULTI_RESULT]] %{{[a-zA-Z0-9_]+}}

// CHECK-LABEL: %testBasicBlockWithInstructions = OpFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[BASIC_INST]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[BASIC_INST_RESULT:result[0-9_]*]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[BASIC_INST]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[BASIC_INST_RESULT]] %{{[a-zA-Z0-9_]+}}

// CHECK-LABEL: %testMultipleBlockWithInstructions = OpFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI_INST]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MULTI_INST_RESULT:result[0-9_]*]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI_INST]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MULTI_INST_RESULT]] %{{[a-zA-Z0-9_]+}}

// CHECK-LABEL: %testBasicToMultipleBlock = OpFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MIXED]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MIXED_RESULT:result[0-9_]*]] %{{[a-zA-Z0-9_]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MIXED]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[MIXED_RESULT]] %{{[a-zA-Z0-9_]+}}

// CHECK-LABEL: %testMixedCallsWithCondition = OpFunction
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[COND]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[COND_RESULT:result[0-9_]*]] %{{[a-zA-Z0-9_]+}}
// The conditional call returns to the branch, where the addition still executes.
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %[[COND_CALL]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[COND_BRANCH]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %[[COND_RESULT]] %int_62
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[COND]]{{$}}
// CHECK-NOT: DebugScope %{{[0-9]+}} %{{[0-9]+}}{{$}}
// CHECK: OpReturnValue %int_62
