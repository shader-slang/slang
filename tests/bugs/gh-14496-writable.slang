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

//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g0 -experimental-feature

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

void throughWriteonlyRef(__ref_writeonly V v)
{
    v[0].y = 4;
}

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

    var wonly = V();
    throughWriteonlyRef(wonly);
}

// No write in this file may round-trip through the `set` accessor at any
// point in the output — a reroute would store into sideTable with index 3.
//IR-NOT: call %Vx5Foperatorx5Bx5Dx5Fset(
// The local write must lower to a direct store through the bound `ref`
// accessor, and the pointer-member write must derive the ReadWrite/Device
// field pointer (the fully-qualified 4-operand form, access 0) and bind the
// `ref` through it, and the `__ref_writeonly` base must also bind the `ref`
// (a reroute would load write-only memory through the getter) — three ref
// calls in total...
//IR: call %Vx5Foperatorx5Bx5Dx5Fref(
//IR: call %Vx5Foperatorx5Bx5Dx5Fref(
//IR: Ptr(%V, 0 : UInt64,
//IR: call %Vx5Foperatorx5Bx5Dx5Fref(
// ...and still no `set` call after the last positive match.
//IR-NOT: call %Vx5Foperatorx5Bx5Dx5Fset(
