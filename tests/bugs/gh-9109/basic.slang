// Test: pragma warning with import statements
// Ensures pragma warnings work correctly across imported modules.
// This is a regression test for issue 9109 where pragma warnings
// were incorrectly rejected when used with import statements.
//TEST:SIMPLE(filecheck=CHECK):

import "basic-module-a.slang";
import "basic-module-b.slang";

void main()
{
    // These functions use pragma warning(disable: 41018) and pragma warning(default: 41018)
    // If the pragma warnings weren't working correctly, we'd get warning 15615 errors
    // during compilation. A successful compile means the pragmas were accepted.
    float out1 = 0.0f;
    foo(5.0f, out1);

    float out2 = 0.0f;
    bar(10.0f, out2);
}

// CHECK-NOT: warning 15615
