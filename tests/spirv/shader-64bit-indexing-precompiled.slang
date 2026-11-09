// An entry point loaded from a precompiled module (`-r`) keeps the `Shader64BitIndexingEXT`
// execution mode that its `[Shader64BitIndexing]` callee requires. The test passes no source file,
// so the entry point and its callee come only from the module.

//TEST:COMPILE: tests/spirv/shader-64bit-indexing-precompiled-module.slang -o tests/spirv/shader-64bit-indexing-precompiled-module.slang-module
//TEST:SIMPLE_EX(filecheck=CHECK): -r tests/spirv/shader-64bit-indexing-precompiled-module.slang-module -target spirv-assembly -emit-spirv-directly -stage compute -entry computeMain

// CHECK: OpCapability Shader64BitIndexingEXT
// CHECK: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]]
// CHECK: OpExecutionMode %[[EP]] Shader64BitIndexingEXT
