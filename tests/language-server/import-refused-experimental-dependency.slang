//TEST:LANG_SERVER(filecheck=CHECK):

// The precompiled `slang.tensor.differentiable` module imports `slang.tensor`, which imports the
// experimental `slang.numerics`. That import is refused while experimental features are
// disabled, as they are by default. Loading either tensor module therefore fails with a
// diagnostic, and the language server keeps answering requests for this file instead of
// terminating.

import slang.tensor.differentiable;

struct Point
{
    int x;
};

void use()
{
//HOVER:19,5
    Point p;
}

// CHECK: struct Point
