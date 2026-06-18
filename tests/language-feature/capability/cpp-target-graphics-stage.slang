// Regression test for shader-slang/slang#11659.
//
// A graphics-stage entry point (vertex/fragment/...) compiled to a CPU
// host-C++ kernel target (cpp/hpp/c/object-code/host-callable/shared-library/
// shader-llvm) used to SIGSEGV the compiler: those targets lower entry-point
// varying parameters with the CPU varying-parameter legalizer, which only
// supports compute system values, and it dereferenced a null parameter while
// legalizing the entry point's result (e.g. an `SV_Position`). The front-end
// now rejects the unsupported stage/target combination with the existing
// capability diagnostic (E36107); and even when that check is bypassed with
// `-ignore-capabilities`, the legalizer diagnoses the unsupported varying
// instead of crashing.

// Default: a clean capability error, not a crash and not silent no-output.
//TEST:SIMPLE(filecheck=VS_CPP): -target cpp -entry vertexMain
//TEST:SIMPLE(filecheck=VS_HPP): -target hpp -entry vertexMain
//TEST:SIMPLE(filecheck=VS_C): -target c -entry vertexMain
//TEST:SIMPLE(filecheck=VS_OBJ): -target object-code -entry vertexMain
//TEST:SIMPLE(filecheck=FS_CPP): -target cpp -entry fragmentMain
// With the capability check suppressed, the legalizer's null-guard must turn
// the unsupported result-position system value into a diagnostic, not a crash.
//TEST:SIMPLE(filecheck=IGNORE): -target cpp -entry vertexMain -ignore-capabilities

// VS_CPP: error[E36107]
// VS_CPP: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'cpp' compilation target.

// VS_HPP: error[E36107]
// VS_HPP: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'cpp' compilation target.

// VS_C: error[E36107]
// VS_C: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'c' compilation target.

// VS_OBJ: error[E36107]
// VS_OBJ: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'cpp' compilation target.

// FS_CPP: error[E36107]
// FS_CPP: entrypoint 'fragmentMain' uses features that are not available in 'fragment' stage for 'cpp' compilation target.

// IGNORE-NOT: error[E36107]
// IGNORE: internal error[E99999]
// IGNORE: this target doesn't support this system-defined varying parameter

[shader("vertex")]
float4 vertexMain() : SV_Position
{
    return float4(0, 0, 0, 1);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1, 0, 0, 1);
}
