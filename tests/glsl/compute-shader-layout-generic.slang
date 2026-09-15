//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl -emit-spirv-via-glsl

// Regression test for https://github.com/shader-slang/slang/issues/13037:
// a module-scope GLSL `layout(local_size_...) in;` must apply to a specialized
// generic compute entry point, not only to a non-generic one. The first run uses
// the default direct SPIR-V backend; the second uses the glslang backend.

// CHECK: OpExecutionMode %{{.*}} LocalSize 8 4 2
layout(local_size_x = 8, local_size_y = 4, local_size_z = 2) in;

interface IWork
{
    void run();
}

struct W : IWork
{
    void run() {}
}

[shader("compute")]
void csMain<T : IWork>(T t)
{
    t.run();
}
