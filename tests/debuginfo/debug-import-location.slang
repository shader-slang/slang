// Regression test for 0b7e9344f / #9520 and b0d4ffe41 / #10907:
// When the shader imports another module the DebugCompilationUnit and
// DebugSource were associated with the imported file instead of the main file,
// causing the debugger to report the wrong filename for breakpoints and
// source locations.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

import debug_import_helper;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    uint a = scale_val(input[0], 3u);   // line 14 — location must name this file
    output[0] = a;                       // line 15
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break debug-import-location.slang 14
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// If the bug regresses, the DebugCompilationUnit points at the imported
// module's file, so the break at "debug-import-location.slang:14" would
// never fire and 'location' would report the wrong filename.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}debug-import-location.slang:14
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [21]
