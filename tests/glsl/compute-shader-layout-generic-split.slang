//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl -emit-spirv-via-glsl

// Companion to compute-shader-layout-generic.slang for https://github.com/shader-slang/slang/issues/13037.
// Splitting the work-group size across several `layout(...) in;` declarations exercises the
// per-axis merge branch of the local_size lift for a specialized generic entry point — a path
// only reachable once the lift scans the correct (module) scope rather than the GenericDecl wrapper.

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
