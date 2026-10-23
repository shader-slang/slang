// Regression test for https://github.com/shader-slang/slang/issues/13888.
//
// Two same-named declarations found at one lookup level hide a same-named declaration in an outer
// scope, also when the use site cannot access either of them. Here `NA::w` and `NB::w` are
// `internal` to another module, so the reference to `w` is an access error and does not resolve
// to the global `w`. A single inaccessible declaration hides the global `w` in the same way.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import "using-namespace-inaccessible-lookup-13888-lib";

static const int w = 3;

namespace M
{
    using namespace NA;
    using namespace NB;

    int get()
    {
        return w;
//CHECK:       ^ declaration not accessible
//CHECK:       ^ 'w' is not accessible from the current context.
    }
}
