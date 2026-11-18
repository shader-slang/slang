//TEST:SIMPLE: -target host-callable -emit-cpu-via-llvm -whole-program -entry main -stage compute

// With -emit-cpu-via-llvm, HostHostCallable flows through the same LLVM CPU
// emitter as llvm-host-ir, so its compute kernels must carry the same
// three-pointer CPU ABI; without the legalization fix the emitted module was
// verifier-invalid. This pins that the path compiles cleanly.
[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
