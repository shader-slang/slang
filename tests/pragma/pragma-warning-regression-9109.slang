//TEST:SIMPLE(filecheck=CHECK): -target spirv -skip-spirv-validation

// Regression test for issue #9109
// This test ensures that #pragma warning directives work correctly across modules
// when different modules have interleaved absolute source locations.
// Previously, pragma warnings in module B could be rejected with warning 15615
// if they had "earlier" absolute locations than pragma warnings in module A.

import "pragma-warning-regression-9109-module-a.slang";
import "pragma-warning-regression-9109-module-b.slang";

// CHECK-NOT: warning 15615

// We expect to see warning 41018 from module B since the pragma only disables it
// for the early return case, not for the entire function
// CHECK: warning 41018

[numthreads(1, 1, 1)]
[shader("compute")]
void main() {
    float a = 1.0f;
    float b;

    bool resultA = moduleAFunction(a, b);
    bool resultB = moduleBFunction(a, b);
}
