//TEST:SIMPLE: -target host-callable -whole-program -entry computeMain -stage compute

// Without -emit-cpu-via-llvm, HostHostCallable compiles through the
// downstream C++ path, whose host-callable functions keep their natural
// signatures for direct host calls. This pins that the CPU-ABI legalization
// added for the via-LLVM path does not change that behavior. The entry point
// is named computeMain so the C++ path's 'main' rename warning stays out of
// the picture.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {}
