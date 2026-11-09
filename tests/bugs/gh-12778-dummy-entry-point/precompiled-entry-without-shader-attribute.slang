// An entry point that comes only from a precompiled module's entry-point list links as a dummy
// `EntryPoint` with no `FuncDecl`, so `EntryPoint::getModule()` is null. Linking must still give
// it an entry-point decoration. `vmain` has no `[shader]` attribute, so loading the module does
// not rediscover it as an entry point with a declaration, and only the dummy reaches the linker.

//TEST:COMPILE: tests/bugs/gh-12778-dummy-entry-point/vertex-module.slang -o tests/bugs/gh-12778-dummy-entry-point/vertex-module.slang-module -entry vmain -stage vertex
//TEST:SIMPLE_EX(filecheck=CHECK): -r tests/bugs/gh-12778-dummy-entry-point/vertex-module.slang-module -target spirv -stage vertex -entry vmain -o tests/bugs/gh-12778-dummy-entry-point/vertex-entry.spv

// Writing to a file selects the per-entry-point output path, which links the dummy entry point.
// CHECK: result code = 0
