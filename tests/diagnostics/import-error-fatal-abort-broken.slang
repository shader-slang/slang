// Companion module for import-error-fatal-abort.slang. It contains a genuine
// user error, so importing it fails during compilation.

float4 brokenHelper()
{
    return someUndefinedSymbol;
}
