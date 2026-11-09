// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Formats scalar runtime values with a locale-independent fmt-style pattern.
// Supported fields are {}, positional fields, escaped braces, and numeric format specs.
String format<each T>(String pattern, expand each T args)
{
    return hgbc_asm
    {
    result:
        $$String = OpFormatString $pattern $expand each args;
    };
}
