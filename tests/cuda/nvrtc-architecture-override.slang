//TEST:SIMPLE(filecheck=SM75): -target ptx -entry computeMain -capability cuda_sm_8_0 -Xnvrtc -arch=compute_75
//TEST:SIMPLE(filecheck=SM75): -target ptx -entry computeMain -capability cuda_sm_8_0 -Xnvrtc --gpu-architecture=compute_75
//TEST:SIMPLE(filecheck=SM75): -target ptx -entry computeMain -capability cuda_sm_8_0 -Xnvrtc -arch -Xnvrtc compute_75
//TEST:SIMPLE(filecheck=SM75): -target ptx -entry computeMain -capability cuda_sm_8_0 -Xnvrtc --gpu-architecture -Xnvrtc compute_75
//TEST:SIMPLE(filecheck=SM80): -target ptx -entry computeMain -capability cuda_sm_7_0 -Xnvrtc -arch=compute_80
//TEST:SIMPLE(filecheck=SM80): -target ptx -entry computeMain -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=SM80): -target ptx -entry computeMain -capability cuda_sm_8_0 -Xnvrtc -DTEST_ARCH=compute_75

// An explicit NVRTC architecture must replace Slang's derived architecture, whether it is lower
// or higher. Use established architectures so this test does not need the latest CUDA toolkit.
// Require empty diagnostics: newer NVRTC accepts duplicate architecture options with a warning,
// while older NVRTC rejects them. Checking only the PTX target would miss the newer behavior.
// The final two cases check that capability-derived selection still works without an override.

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}

// SM75: result code = 0
// SM75-NEXT: standard error = {
// SM75-NEXT: }
// SM75: .target sm_75

// SM80: result code = 0
// SM80-NEXT: standard error = {
// SM80-NEXT: }
// SM80: .target sm_80
