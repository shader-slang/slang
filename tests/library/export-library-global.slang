//TEST:SIMPLE(filecheck=CHECK): -o tests/library/export-library-global.slang-module -target spirv -embed-downstream-ir -dump-ir

// CHECK: EmbeddedDownstreamIR(6 : Int,
// CHECK: OpCapability Linkage
// CHECK: OpDecorate %myGlobalVar LinkageAttributes "_SV11myGlobalVar" Export
// CHECK: %myGlobalVar = OpVariable %_ptr_Private_int Private %int_42

export static int myGlobalVar = 42;