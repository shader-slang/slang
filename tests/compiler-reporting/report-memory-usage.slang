//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main -report-memory-usage

// Pins the `-report-memory-usage` output SHAPE, not its values, which are machine- and
// build-specific. The shape is what tools/compile-perf/bench.py parses (`parse_mem`), and a drift
// in it — a tab becoming a space, the "kb" suffix changing, a counter losing its capitalized "Kb"
// — degrades memory collection to silently reporting no data, with every run still green. That is
// the same failure mode bench.py guards against for the api-driver's producer by reading its
// printf; this test is the equivalent guard for the slangc producer, which nothing else covers.
//
// The counters checked are one per component family rather than all of them, so that adding a
// counter does not require touching this test, while removing or renaming one of the load-bearing
// ones does.

// CHECK-DAG: [MEM] astArenaReservedKb{{[[:space:]]+}}{{[0-9]+}}kb
// CHECK-DAG: [MEM] irArenaReservedKb{{[[:space:]]+}}{{[0-9]+}}kb
// CHECK-DAG: [MEM] sourceContentKb{{[[:space:]]+}}{{[0-9]+}}kb
// CHECK-DAG: [MEM] builtinModuleIrKb{{[[:space:]]+}}{{[0-9]+}}kb
// CHECK-DAG: [MEM] endOfCompileRssKb{{[[:space:]]+}}{{[0-9]+}}kb

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
}
