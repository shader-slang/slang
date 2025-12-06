//TEST:SIMPLE(filecheck=CHECK): -target spirv -skip-spirv-validation

// This test ensures that #pragma warning directives work correctly across modules.
// Previously, there was a bug where pragma warnings in one module could be rejected
// if they had "earlier" absolute source locations than pragma warnings in a previously
// processed module, causing warning 15615 "Cannot insert #pragma warning here".

import "pragma-warning-module-a.slang";
import "pragma-warning-module-b.slang";

// CHECK-NOT: warning 15615
// We should still see the warning 41018 from module B since its pragma is suppressed by our fix
// CHECK: warning 41018

[numthreads(1, 1, 1)]
[shader("compute")]
void main() {
    float a = 1.0f;
    float b;

    bool resultA = moduleAFunction(a, b);
    bool resultB = moduleBFunction(a, b);
}