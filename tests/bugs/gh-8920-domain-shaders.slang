//TEST:SIMPLE(filecheck=CHECK): -target spirv -g

// This is a minimal repro for issue 8920

struct PatchOut {
    int nothing;
}

struct VOut {
    int dummy;
};

[[shader("vertex")]]
VOut main(
    const OutputPatch<PatchOut, 1> bp
) {
    VOut v;
    v.dummy = 1;

    return v;

    // CHECK: OpFunctionEnd
}
