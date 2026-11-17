// A companion to gh-14386-half-nan-quiet.slang for the byte-code VM: the interpreter runs
// from serialized constants, and a `half` constant is materialized there through
// `FloatToHalf` (slang-emit-vm.cpp), so the narrowed NaN must be the quiet encoding on this
// path too. The INTERPRET lane prints through printf rather than storing to a buffer, which
// is why it lives apart from the buffer lanes of the main test. Pre-fix, the folded
// signaling NaN 0x7C01 / 0xFC01 fails the `{{7|f}}e00` check. The sign of the folded
// `0.0 / 0.0` is not specified, so the checks accept either sign.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half h = half(1.0);
    //CHECK: half-one 0x3c00
    printf("half-one 0x%x\n", (uint)bit_cast<uint16_t>(h));
    half n = half(0.0 / 0.0);
    //CHECK: half-nan 0x{{7|f}}e00
    printf("half-nan 0x%x\n", (uint)bit_cast<uint16_t>(n));
    //CHECK: float-nan 0x{{7|f}}fc00000
    printf("float-nan 0x%x\n", bit_cast<uint>(0.0f / 0.0f));
}
