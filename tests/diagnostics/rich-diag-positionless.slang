// The only diagnostic is the positionless warning E00018 "'-separate-debug-info' is not
// supported for target 'spirv-asm'" — a rich header with no location line, flushed at the
// end of the stream, so the parsed record carries line 0 and no file position.
//TEST:SIMPLE_LINE(filecheck=POSCHECK):-entry main -stage compute -target spirv-asm -separate-debug-info
//TEST:SIMPLE(filecheck=POSHDR):-entry main -stage compute -target spirv-asm -separate-debug-info

[shader("compute")]
[numthreads(1,1,1)]
void main() { }
// POSCHECK: 0
// POSHDR: warning[E00018]: '-separate-debug-info' is not supported for target 'spirv-asm'
