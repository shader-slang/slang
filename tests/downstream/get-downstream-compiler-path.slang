//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -get-glslang-path -get-nvrtc-path -get-clang-path -get-spirv-dis-path

// Verify the `-get-<compiler>-path` query option across several downstream compilers in one
// invocation. For each flag the compiler prints "<compiler> path: <result>" and then continues the
// compile (print-and-continue, like `-version`); the lines appear in flag order because each flag is
// handled inline as options are parsed.
//
// This covers more than a single compiler. Across the four flags the query can collectively
// encounter all three result branches -- which one each hits depends on what is installed on the
// runner -- and it includes a hyphenated compiler name:
//   - glslang and nvrtc are shared libraries that, when present, resolve to a real on-disk path --
//     the SLANG_OK branch that emits the path blob.
//   - clang is an executable-based command-line compiler with no recoverable shared-library path,
//     so it reports "not available" (SLANG_E_NOT_AVAILABLE); a compiler that cannot be located at
//     all reports "not found" (SLANG_E_NOT_FOUND).
//   - spirv-dis has a '-' in its name, so it exercises the affix-stripping that recovers <compiler>
//     by removing the fixed "-get-" / "-path" wrapper (a naive lastIndexOf('-') scan would mis-split
//     it). Seeing the "spirv-dis path:" prefix at all is the proof that the stripping is correct.
//
// The value after each "path:" is matched with `{{.+}}` because which of resolved-path /
// "not available" / "not found" occurs depends on what is installed on the runner; the point is that
// every flag is recognized, prints its line in order, and the compile still proceeds -- shown by the
// trailing `void main(` in the emitted HLSL.

//CHECK: glslang path: {{.+}}
//CHECK: nvrtc path: {{.+}}
//CHECK: clang path: {{.+}}
//CHECK: spirv-dis path: {{.+}}
//CHECK: void main(

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = 1;
}
