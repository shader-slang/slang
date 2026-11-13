// The Slang VM emitter names each function by its qualified name, such as `A.run`, and makes a
// repeated name unique, so entry points that share an unqualified name do not collide in a VM
// program (shader-slang/slang#14053).

//TEST:SIMPLE(filecheck=CHECK):-target slangvm -whole-program

// CHECK-NOT: E38016
// CHECK: --START--

namespace A
{
[shader("dispatch")]
int run()
{
    return 1;
}
}

namespace B
{
[shader("dispatch")]
int run()
{
    return 2;
}
}
