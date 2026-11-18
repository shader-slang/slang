// Both CPU-via-LLVM host targets must route a compute kernel through the CPU
// entry-point param legalization, so the kernel carries the three
// context-pointer CPU ABI parameters that the unconditionally emitted
// workgroup wrapper and dispatcher hardcode in their call. Without the
// legalization the emitted LLVM module fails verification, because the
// kernel keeps a zero-parameter signature while the wrapper call passes
// three context pointers. First test coverage for either target;
// follow-up from #14579 / #14582.
//TEST:SIMPLE(filecheck=CHECK):-target host-object-code -whole-program -entry main -stage compute -dump-ir-after introduceExplicitGlobalContext
//TEST:SIMPLE(filecheck=CHECK):-target host-host-callable -emit-cpu-via-llvm -whole-program -entry main -stage compute -dump-ir-after introduceExplicitGlobalContext

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
//CHECK: func %main{{.*}}: Func(Void, Ptr({{.*}}), Ptr(%EntryPointParams, {{.*}}), RawPointerType)
