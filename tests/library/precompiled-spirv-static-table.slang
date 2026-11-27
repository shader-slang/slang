// precompiled-spirv-static-table.slang

// Links the precompiled table-library module (embedded SPIR-V, no entry
// points) into a compute entry point. The linked SPIR-V must contain the
// table composite and the self-contained copy the library's read uses: a
// variable stored into before its dynamic-index access chain. A shared
// Private global would have no entry point to initialize it in, so it must
// not appear.

//TEST:COMPILE: tests/library/module-library-static-table.slang -o tests/library/module-library-static-table.slang-module -target spirv -embed-downstream-ir -incomplete-library
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O0 -skip-spirv-validation

import "module-library-static-table";

RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    gOut[id.x] = tableLookup(float(id.x));
}

// The table's composite and the read's self-contained variable, stored into
// before the access chain that reads it.
// CHECK: OpConstantComposite %_arr_float_int_8 %float_0 %float_0_125 %float_0_25 %float_0_375 %float_0_5 %float_0_625 %float_0_75 %float_0_875
// CHECK: %[[TABLE:[0-9]+]] = OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK: OpStore %[[TABLE]]
// CHECK: OpAccessChain %_ptr_Function_float %[[TABLE]]
// A shared Private global has no entry point to initialize it in a library.
// CHECK-NOT: OpVariable %_ptr_Private__arr_float_int_8 Private
