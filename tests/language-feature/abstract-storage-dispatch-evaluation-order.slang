//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A dispatch expression evaluates its dispatch size before its thread-group size. Both size
// expressions in this test have a side effect, so the checks require exactly one call for each
// expression in that source order.

static int evaluationTrace;

void appendTrace(int digit)
{
    evaluationTrace = evaluationTrace * 10 + digit;
}

// The test uses a property getter as the dispatch-size expression. The getter must run before the
// expression that computes the thread-group size.
struct DispatchSizeStorage
{
    uint3 storedValue;

    property uint3 value
    {
        [mutating]
        get
        {
            appendTrace(1);
            return storedValue;
        }
    }
}

uint3 evaluateThreadGroupSize()
{
    appendTrace(2);
    return uint3(1, 1, 1);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void dispatchedKernel()
{}

void formDispatch(inout DispatchSizeStorage storage)
{
    let dispatch = __dispatch_kernel(dispatchedKernel, storage.value, evaluateThreadGroupSize());
}

// CHECK-LABEL: [nameHint("DispatchSizeStorage.value.get")]
// CHECK: func [[DISPATCH_SIZE_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("evaluateThreadGroupSize")]
// CHECK: func [[EVALUATE_THREAD_GROUP_SIZE:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("formDispatch")]
// CHECK-NOT: call [[DISPATCH_SIZE_GET]]
// CHECK-NOT: call [[EVALUATE_THREAD_GROUP_SIZE]]
// CHECK: let [[DISPATCH_SIZE:%[A-Za-z0-9_]+]] : {{.*}} = call [[DISPATCH_SIZE_GET]]
// CHECK-NOT: call [[DISPATCH_SIZE_GET]]
// CHECK-NOT: call [[EVALUATE_THREAD_GROUP_SIZE]]
// CHECK: let [[THREAD_GROUP_SIZE:%[A-Za-z0-9_]+]] : {{.*}} = call [[EVALUATE_THREAD_GROUP_SIZE]]
// CHECK-NOT: call [[DISPATCH_SIZE_GET]]
// CHECK-NOT: call [[EVALUATE_THREAD_GROUP_SIZE]]
// CHECK: {{.*}}DispatchKernel({{.*}}, [[THREAD_GROUP_SIZE]], [[DISPATCH_SIZE]])
// CHECK-NOT: call [[DISPATCH_SIZE_GET]]
// CHECK-NOT: call [[EVALUATE_THREAD_GROUP_SIZE]]
// CHECK: return_val
