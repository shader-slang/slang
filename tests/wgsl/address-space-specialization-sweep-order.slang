//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry cmain -stage compute

// Address-space specialization removes a function once all of its calls target clones. Here `G`'s
// original body still calls the original `F`, so `F` loses its last use only when `G` is removed.
// The exports are kept alive, so a later dead-code pass would not remove an unused duplicate of `F`;
// the specialization pass itself must keep removing until nothing more is left without uses.

groupshared int gs;

export void F(inout int x)
{
    x += 1;
}

export void G(inout int y)
{
    F(y);
}

[numthreads(1, 1, 1)]
void cmain()
{
    F(gs);
    G(gs);
}

// CHECK-COUNT-1: fn F_{{[0-9]+}}(
// CHECK-NOT: fn F_{{[0-9]+}}(
