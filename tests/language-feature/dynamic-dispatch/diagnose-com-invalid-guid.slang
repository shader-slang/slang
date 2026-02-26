// COM interfaces require a valid 128-bit GUID in the standard
// 8-4-4-4-12 hexadecimal format for binary interface identification.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: ([[# @LINE+1]]): error 31123
[COM("not-a-valid-guid")]
interface ICOMBadGuid
{
    float getValue();
}
