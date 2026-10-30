// Standard modules are always built without `-obfuscate`, so code compiled with
// it looks up the `[__unsafeForceInlineEarly]` conformance behind `log` under a
// hashed name that `slang.numerics` does not export. That mismatch is reported
// as an error at the import (shader-slang/slang#14091).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment -experimental-feature -obfuscate

import slang.numerics;
/*CHECK:
^^^^^^ cannot link 'log' from module 'slang/numerics'
^^^^^^ 'slang/numerics' was compiled with a different `-obfuscate` setting
*/

T logOfUserValue<T>(T value)
    where T : IExponentialFunctions
{
    return log(value);
}

[shader("fragment")]
float4 main(float4 color) : SV_TARGET
{
    return logOfUserValue(color);
}
