//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage amplification -g2 -emit-spirv-directly

// Test that the global variable created for DispatchMesh payload has a meaningful name
// in the SPIR-V debuginfo output.

struct TaskPayload {
    float dummy;
};

[numthreads(1, 1, 1)]
void main() {
    TaskPayload payload;
    payload.dummy = 1.0;
    DispatchMesh(1, 1, 1, payload);
}

// Verify the global payload variable has a distinctive name (not "unnamed")
// CHECK: OpEntryPoint TaskEXT %main "main"
// CHECK: OpString "__EmitMeshTasks_Payload"
// CHECK: TaskPayloadWorkgroupEXT
// CHECK: DebugGlobalVariable
