// Companion module for import-error-fatal-abort.slang. It contains a genuine
// user error, so importing it fails during compilation.

float4 brokenHelper()
{
    return someUndefinedSymbol;
}

// The undefined identifier above must stay on line 6: the companion test pins
// that diagnostic with a caret annotation matched by (line, column), and this
// file's line number is the one it matches against.
