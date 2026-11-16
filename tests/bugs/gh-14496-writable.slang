// Companion to gh-14496.slang: the guard must NOT fire on a genuinely
// writable base. A component write on a mutable local still binds the
// `[mutating] ref` accessor and stores directly, and the pointer-member
// write (`Ptr<Holder>`, default Access.ReadWrite + AddressSpace.Device)
// exercises the fully-derived ReadWrite/Device field pointer that
// `extractField` builds — if either the derivation or the guard mishandled
// the writable shapes, the writes would reroute through the `set` accessor
// (into sideTable) and the ref calls below would disappear from the IR.
//
// This is pinned at the IR level rather than as an executable test: no
// execution backend can currently run a call to this `ref` accessor — the
// CPU and CUDA pipelines cannot lower a `ref` accessor whose body returns
// an l-value (a pre-existing gap), the Vulkan pipeline crashes on the same
// shape, and the SPIR-V emitter hits a pre-existing field-address assertion
// (reproduced on unmodified master) for the pointer-member variant.

//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g0

RWStructuredBuffer<uint4> sideTable;

struct V
{
    uint4 data[4];

    __subscript(int i) -> uint4
    {
        get { return data[i]; }
        [nonmutating] set { sideTable[i] = newValue; }
        [mutating] ref { return data[i]; }
    }
};

struct Holder { V v; }

[numthreads(1, 1, 1)]
void computeMain()
{
    var local = V();
    local[3].z = 7;
    sideTable[0] = local.data[3];

    Holder h2 = Holder();
    Ptr<Holder> hp = Ptr<Holder>(&h2); // default Access.ReadWrite, AddressSpace.Device
    hp->v[3].z = 7;
    sideTable[1] = h2.v.data[3];
}

// The local write must lower to a direct store through the bound `ref`
// accessor, and the pointer-member write must derive the ReadWrite/Device
// field pointer (the fully-qualified 4-operand form, access 0) and bind the
// `ref` through it — two ref calls in total...
//IR: call %Vx5Foperatorx5Bx5Dx5Fref(
//IR: Ptr(%V, 0 : UInt64,
//IR: call %Vx5Foperatorx5Bx5Dx5Fref(
// ...and neither write may round-trip through the `set` accessor.
//IR-NOT: call %Vx5Foperatorx5Bx5Dx5Fset(
