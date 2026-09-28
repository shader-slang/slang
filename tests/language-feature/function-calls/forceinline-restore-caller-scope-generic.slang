//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -g3 -O0

// Both the entry and the restoration of an early-inlined generic must use the
// DebugFunction already specialized by the generic clone environment.
RWStructuredBuffer<int> outputBuffer;

[__unsafeForceInlineEarly]
T leaf<T : IArithmetic>(T value)
{
    return value + T(1);
}

[__unsafeForceInlineEarly]
T outer<T : IArithmetic>(T value)
{
    T nested = leaf<T>(value);
    T result = nested + T(2);
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int callerResult = outer<int>(int(tid.x));
    float otherResult = outer<float>(float(tid.x));
    outputBuffer[tid.x] = callerResult + int(otherResult);
}

// Capture each specialized scope from its local-variable record. Entry, nested call,
// and restoration must all reference that same identity and the same inline site.
// CHECK-DAG: %[[MAINNAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[MAIN:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MAINNAME]]
// CHECK-DAG: %nested = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %[[INT_SCOPE:[0-9]+]] %uint_0
// CHECK-DAG: %nested_0 = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %[[FLOAT_SCOPE:[0-9]+]] %uint_0
// CHECK-DAG: %[[INT_SITE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %[[MAIN]]{{$}}
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %[[INT_SCOPE]] %[[INT_SITE]]{{$}}
// CHECK-DAG: %[[FLOAT_SITE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %[[MAIN]]{{$}}
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %[[FLOAT_SCOPE]] %[[FLOAT_SITE]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[INT_SCOPE]] %[[INT_SITE]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[INT_SCOPE]] %[[INT_SITE]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MAIN]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[FLOAT_SCOPE]] %[[FLOAT_SITE]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[FLOAT_SCOPE]] %[[FLOAT_SITE]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MAIN]]{{$}}
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugNoScope
