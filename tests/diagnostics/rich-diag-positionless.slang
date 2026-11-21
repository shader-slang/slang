// Lanes for records with no source position. The first two pin the positionless warning
// E00018 ("'-separate-debug-info' is not supported for target 'spirv-asm'"), whose rich
// header has no location line at all and is flushed at end of stream. The CLILOC lane pins
// the bare-path location shape: the E00017 unknown-option error locates on the command
// line, rendered as ` --> command line` with no line:col, so the parsed record carries line
// 0 with the whole text as its path. The CKPT lane pins a codeless standalone note
// leading a stream (report-checkpoint-none, code -1) — also line 0. The PERF lane pins a
// coded note whose message spans continuation lines (the performance-benchmark report)
// — also line 0, with the continuation lines buffered into the record's text.
//TEST:SIMPLE_LINE(filecheck=POSCHECK):-entry main -stage compute -target spirv-asm -separate-debug-info
//TEST:SIMPLE(filecheck=POSHDR):-entry main -stage compute -target spirv-asm -separate-debug-info
//TEST:SIMPLE_LINE(filecheck=CLILOC):-entry main -stage compute -target spirv-asm -unknown-test-option
//TEST:SIMPLE_LINE(filecheck=CKPT):-entry main -stage compute -target spirv-asm -report-checkpoint-intermediates
//TEST:SIMPLE_LINE(filecheck=PERF):-entry main -stage compute -target spirv-asm -report-perf-benchmark

[shader("compute")]
[numthreads(1,1,1)]
void main() { }
// POSCHECK: 0
// CKPT: 0
// CLILOC: 0
// PERF: 0
// POSHDR: warning[E00018]: '-separate-debug-info' is not supported for target 'spirv-asm'
