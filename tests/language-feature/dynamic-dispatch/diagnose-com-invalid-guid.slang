// COM interfaces require a valid 128-bit GUID in the standard
// 8-4-4-4-12 hexadecimal format for binary interface identification.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: ([[# @LINE+1]]): error 31123
[COM("not-a-valid-guid")]
interface ICOMBadGuid
{
    float getValue();
}

// Correct 8-4-4-4-12 structure but with non-hex characters ('g', 'z').
// CHECK: ([[# @LINE+1]]): error 31123
[COM("g0000000-0z00-0000-0000-000000000001")]
interface ICOMBadHex
{
    float getValue();
}

// All valid hex characters but too few digits (28 instead of 32).
// This exercises the length validation rather than the character validation.
// CHECK: ([[# @LINE+1]]): error 31123
[COM("0000000-0000-0000-0000-000000000001")]
interface ICOMTooShort
{
    float getValue();
}
