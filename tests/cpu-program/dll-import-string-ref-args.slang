//TEST:EXECUTABLE:

// `[DllImport]` calls go through `NativeCallMarshallingContext::marshalNativeCall`. An `inout` or
// `out` `String` parameter is `const char**` in its native form, which cannot point at a managed
// `String`. The call therefore passes the address of a native temporary and copies the pointer
// that the callee leaves there back into the caller's `String` after the call.

// The native signature of this export is `void pickLonger(const char** text, const char** other)`,
// which is also the native signature of the import below that takes two `inout String`s.
[DllExport]
export void pickLonger(NativeString* text, NativeString* other)
{
    if ((*other).length > (*text).length)
        *text = *other;
}

[DllImport("", "pickLonger")]
void pickLongerImported(inout String text, inout String other);

// An `out String` argument passes an uninitialized native temporary that the callee fills in.
[DllExport]
export void copyText(NativeString* destination, NativeString* source)
{
    *destination = *source;
}

[DllImport("", "copyText")]
void copyTextImported(out String destination, inout String source);

void printText(NativeString text)
{
    printf("%s\n", text);
}

export __extern_cpp int main()
{
    String text = "short";
    String other = "tiny";
    pickLongerImported(text, other);
    printText(text);
    other = "much longer";
    pickLongerImported(text, other);
    printText(text);

    String copy = "";
    copyTextImported(copy, other);
    printText(copy);
    return 0;
}
