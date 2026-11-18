//TEST:SIMPLE(filecheck=CHECK):-target llvm-host-ir -emit-cpu-via-llvm -whole-program

// A zero-parameter compute kernel legalizes to the CPU ABI: exactly three
// context pointers (threadInput, entryPointParams, globalParams), matching
// the arguments the emitted workgroup wrapper and dispatcher hardcode
// (emitComputeEntryPointWorkGroup / emitComputeEntryPointDispatcher in
// slang-llvm-builder.cpp).
[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
//CHECK-NOT: define void @_S{{[^()]*}}()
//CHECK: define void @_S{{[^()]*}}(ptr {{[^,%]*}}%{{[^,)]*}}, ptr {{[^,%]*}}%{{[^,)]*}}, ptr {{[^,%]*}}%{{[^,)]*}})
//CHECK: define void @main_Group(ptr
//CHECK: define void @main(ptr
