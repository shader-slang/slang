//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -no-codegen

// Regression test for #10837.
//
// When a `#line` directive remaps line numbers, the diagnostic renderer
// used to show the wrong source line (using the remapped line number
// to index into the physical file). Verify the correct source line is
// displayed — it should show the line containing `undeclaredIdentifier`,
// not the padding struct field at physical line 3.

struct Padding {
    uint a;
    uint b;
};

#line 1
void test()
{
    int x = undeclaredIdentifier;
}

// The displayed source line must be the actual error line, not a
// padding struct field from the physical file.
// CHECK: undeclaredIdentifier
// CHECK: undefined identifier
