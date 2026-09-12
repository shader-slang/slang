//TEST:SIMPLE(filecheck=CHECK): tests/spirv/debug-function-scope-line-collide-other.slang -target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly -I tests/spirv

// A `#line` directive can name a path that is also a real input to the same compilation. The remapped
// source then has a compilation unit of its own, so resolving ownership from the reported source binds
// the unit of a file that never compiled the function. Ownership comes from the physical location
// instead, which `#line` does not affect.
//
// The reported source and the parent scope deliberately disagree here, as they do for a function from
// an #include'd header: the source says where the code claims to come from, the scope says which
// compilation unit compiled it.

#line 1 "tests/spirv/debug-function-scope-line-collide-other.slang"
float collidedFn(float x)
{
    return x * 2.0;
}

#line 3000
float collideOtherFn(float x);

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(collidedFn(0.5) + collideOtherFn(0.25));
}

// This file physically holds collidedFn, so its compilation unit is the owning one.
// CHECK-DAG: [[THIS_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-line-collide.slang"
// CHECK-DAG: [[SOURCE_THIS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[THIS_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_THIS:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_THIS]] %uint_{{[0-9]+}}

// The file the `#line` names is a real input, so it has its own DebugSource and compilation unit —
// which is what makes binding from the reported source observably wrong.
// CHECK-DAG: [[OTHER_FILE:%[0-9]+]] = OpString "{{.*}}debug-function-scope-line-collide-other.slang"
// CHECK-DAG: [[SOURCE_OTHER:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[OTHER_FILE]] %{{[0-9]+}}
// CHECK-DAG: [[CU_OTHER:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_OTHER]] %uint_{{[0-9]+}}

// The discriminating assertion: collidedFn reports [[SOURCE_OTHER]] as its source, because that is
// what `#line` asks for, but is scoped to [[CU_THIS]] — the unit that compiled it. Resolving from the
// reported source binds [[CU_OTHER]] instead.
// CHECK-DAG: [[FN_COLLIDED:%[0-9]+]] = OpString "collidedFn"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunction [[FN_COLLIDED]] %{{[0-9]+}} [[SOURCE_OTHER]] %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[CU_THIS]]
