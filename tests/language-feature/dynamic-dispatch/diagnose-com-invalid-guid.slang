// COM interfaces require a valid 128-bit GUID in the standard
// 8-4-4-4-12 hexadecimal format for binary interface identification.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

[COM("not-a-valid-guid")]
/*CHECK:
 ^^^ E31123
 ^^^ 'not-a-valid-guid' is not a valid GUID
*/
interface ICOMBadGuid
{
    float getValue();
}

// Correct 8-4-4-4-12 structure but with non-hex characters ('g', 'z').
[COM("g0000000-0z00-0000-0000-000000000001")]
/*CHECK:
 ^^^ E31123
 ^^^ 'g0000000-0z00-0000-0000-000000000001' is not a valid GUID
*/
interface ICOMBadHex
{
    float getValue();
}

// All valid hex characters but too few digits (28 instead of 32).
// This exercises the length validation rather than the character validation.
[COM("0000000-0000-0000-0000-000000000001")]
/*CHECK:
 ^^^ E31123
 ^^^ '0000000-0000-0000-0000-000000000001' is not a valid GUID
*/
interface ICOMTooShort
{
    float getValue();
}
