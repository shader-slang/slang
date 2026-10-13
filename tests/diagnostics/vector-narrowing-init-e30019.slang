//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// tools/compile-perf's `diagnostics` workload (lib/workloads.py) pins this
// exact narrowing-assignment shape as its only source of diagnostic
// production, and the manifest declares E30019 an EXPECTED diagnostic so
// bench.py fails the workload if it stops appearing. That check only runs
// against a live compiler in the nightly bench, so it cannot catch a
// coercion-rule or diagnostic-numbering change on its own; this regression
// test anchors the compiler-side half of that contract -- if narrowing a
// float4 into a float3 stops emitting E30019 here, this test fails instead
// of the workload silently reverting to measuring a clean compile.

float bad(float4 v)
{
    float3 t = v;
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'vector<float,3>', got 'vector<float,4>'
    return t.x;
}
