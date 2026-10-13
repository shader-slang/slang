//TEST:COMPILE: -lang hlsl tests/serialization/effective-this-param-mangling-hlsl-module.slang -o tests/serialization/effective-this-param-mangling-hlsl-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/serialization/effective-this-param-mangling-hlsl-module.slang-module -no-codegen -dump-ir

// Loading the binary module must preserve both effective receiver modes and the provenance that
// distinguishes HLSL's implicit mutable default from an explicitly read-only receiver. Mangling
// consumes that provenance: the implicit default uses the established `m` suffix, while trailing
// `const` retains the suffix-free read-only spelling.

import "effective-this-param-mangling-hlsl-module";

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    HLSLReceiver receiver = {1};
    outputBuffer[0] = receiver.implicitMutable(2) + receiver.trailingConst(3);
}

// CHECK: [import("{{.*}}HLSLReceiver15implicitMutablep1pi_iim")]
// CHECK-NEXT: [nameHint("HLSLReceiver.implicitMutable")]
// CHECK-NEXT: func {{.*}} : Func(Int, BorrowInOutParam(%HLSLReceiver), Int)
// CHECK: [import("{{.*}}HLSLReceiver13trailingConstp1pi_ii")]
// CHECK-NEXT: [nameHint("HLSLReceiver.trailingConst")]
// CHECK-NEXT: func {{.*}} : Func(Int, %HLSLReceiver, Int)
