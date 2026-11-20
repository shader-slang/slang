// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Formats scalar runtime values with a locale-independent fmt-style pattern.
// Supported fields are {}, positional fields, escaped braces, and numeric format specs.
public String format<each T>(String pattern, expand each T args)
{
    return hgbc_asm
    {
    result:
        $$String = OpFormatString $pattern $expand each args;
    };
}

/// Reports whether two strings contain the same bytes.
///
/// For example, `format("{}", 3) == "3"` is true. The comparison is exact: it is case sensitive
/// and performs no Unicode normalization.
public bool operator==(String left, String right)
{
    uint count = hgbc_asm { result: $$uint = OpStringGetByteCount $left; };
    uint otherCount = hgbc_asm { result: $$uint = OpStringGetByteCount $right; };
    if (count != otherCount)
        return false;
    for (uint index = 0; index < count; ++index)
    {
        uint leftByte = hgbc_asm { result: $$uint = OpStringReadByte $left $index; };
        uint rightByte = hgbc_asm { result: $$uint = OpStringReadByte $right $index; };
        if (leftByte != rightByte)
            return false;
    }
    return true;
}

/// Reports whether two strings differ in length or in any byte.
public bool operator!=(String left, String right)
{
    return !(left == right);
}
