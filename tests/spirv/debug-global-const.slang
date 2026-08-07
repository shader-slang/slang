//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage fragment -entry main -g2 -O0 -emit-spirv-directly

// Test that debug info (DebugGlobalVariable) is emitted for global const variables with -g2 -O0.
// Previously the IRGlobalConstant was removed by replaceGlobalConstants before the SPIRV emitter
// ran, so no debug info was emitted for such constants.

static const double cf1 = 3.0;
static const float cf2 = 2.5;

float main() : SV_TARGET
{
    return float(cf1) + cf2;
}

// CHECK-DAG: DebugGlobalVariable
// CHECK-DAG: "cf1"
