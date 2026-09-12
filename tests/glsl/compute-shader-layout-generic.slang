//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry csMain<W> -allow-glsl -emit-spirv-directly

// Regression test for https://github.com/shader-slang/slang/issues/13037:
// a standalone GLSL `layout(local_size_...) in;` was silently ignored for a
// *specialized generic* compute entry point, emitting LocalSize 1 1 1 instead of
// the requested dimensions. The local_size lift scanned the raw parent decl (the
// GenericDecl wrapper of the inner entry function) rather than the module scope
// that actually holds the layout declaration, so it found no layout sibling.

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
