// A `float` constant that is a signaling NaN keeps its bits in the LLVM IR.
// `ConstantFP::get` narrows a `double` to `float` with `APFloat::convert`, which sets the quiet
// bit, so 0x7F800001 would be emitted as 0x7FC00001. The constant is built from its bits instead.
// See float-constant-signaling-nan.slang for the bits read back at run time.

//TEST:SIMPLE(filecheck=IR): -target llvm-ir -whole-program -O1
//TEST:SIMPLE(filecheck=NOQ): -target llvm-ir -whole-program -O1

// LLVM prints a `float` constant as the `double` that holds it, so 0x7F800001 is
// 0x7FF0000020000000.

// The NaNs that initialize an array: 0x7F800001, 0xFF800001 and 0x7FBFFFFF are signaling, and
// 0x7FC12345 is a quiet NaN, which keeps its payload as before.
// IR-DAG: store float 0x7FF0000020000000
// IR-DAG: store float 0xFFF0000020000000
// IR-DAG: store float 0x7FF7FFFFE0000000
// IR-DAG: store float 0x7FF82468A0000000
static const float kNaNs[4] = {
    bit_cast<float>(0x7F800001u),
    bit_cast<float>(0xFF800001u),
    bit_cast<float>(0x7FBFFFFFu),
    bit_cast<float>(0x7FC12345u),
};

export float pickNaN(uint i)
{
    return kNaNs[i & 3];
}

// A signaling NaN returned from a function.
// IR-DAG: ret float 0x7FF0000020000000
export float signalingNaN()
{
    return bit_cast<float>(0x7F800001u);
}

// The quieted patterns 0x7FC00001, 0xFFC00001 and 0x7FFFFFFF appear nowhere in the module.
// NOQ-NOT: 0x7FF8000020000000
// NOQ-NOT: 0xFFF8000020000000
// NOQ-NOT: 0x7FFFFFFFE0000000
