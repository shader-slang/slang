//TEST_IGNORE_FILE:
module "overload-import-overrides-local-many-args-helper";

// Exports an exact match for the float arguments, so it wins on conversion cost over the
// importing module's int-parameter overload.
public float accumulate(
    float a0, float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9,
    float a10, float a11, float a12, float a13, float a14, float a15, float a16, float a17, float a18, float a19,
    float a20, float a21, float a22, float a23, float a24, float a25, float a26, float a27, float a28, float a29,
    float a30, float a31, float a32, float a33, float a34, float a35, float a36, float a37, float a38, float a39,
    float a40, float a41, float a42, float a43, float a44, float a45, float a46, float a47, float a48, float a49,
    float a50, float a51, float a52, float a53, float a54, float a55, float a56, float a57, float a58, float a59,
    float a60, float a61, float a62, float a63, float a64, float a65, float a66, float a67, float a68, float a69,
    float a70, float a71, float a72, float a73, float a74, float a75, float a76, float a77, float a78, float a79,
    float a80, float a81, float a82, float a83, float a84, float a85, float a86, float a87, float a88, float a89,
    float a90, float a91, float a92, float a93, float a94, float a95, float a96, float a97, float a98, float a99)
{
    return a0 + a99;
}
