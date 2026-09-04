// Resolve the effective language for the whole translation unit before parsing any source file.
// The GLSL directive is deliberately in the second file, while this first file requires GLSL
// parsing.

//TEST:SIMPLE_EX(filecheck=CHECK): tests/glsl/effective-language-from-later-source.slang tests/glsl/effective-language-from-later-source-helper.slang -entry main -stage compute -target spirv -emit-spirv-directly -Wno-120

layout(local_size_x = 1) in;

void main()
{}

// CHECK: OpEntryPoint GLCompute
