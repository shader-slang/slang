//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl -emit-spirv-via-glsl

// Companion to compute-shader-layout-generic.slang for https://github.com/shader-slang/slang/issues/13037.
// When the work-group size is split across several module-scope `layout(...) in;` declarations, all
// of them must be merged (one axis each) for a specialized generic compute entry point, giving a
// synthesized size of 8 x 4 x 2 rather than any single declaration's lone axis.

// CHECK: OpExecutionMode %{{.*}} LocalSize 8 4 2
layout(local_size_x = 8) in;
layout(local_size_y = 4) in;
layout(local_size_z = 2) in;

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
