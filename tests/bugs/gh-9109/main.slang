// Regression test for GitHub issue #9109
// The bug was that pragma warnings were rejected based on the length
// of comments in other imported modules.
//TEST:SIMPLE(filecheck=CHECK):

import "module-a.slang";
import "module-b.slang";

void main()
{
    float b = 0.0f;
    foo(5.0f, b);
}

// CHECK-NOT: warning 15615
// This verifies that pragma warning directives are accepted in all modules
// regardless of the length of code or comments in other modules.
