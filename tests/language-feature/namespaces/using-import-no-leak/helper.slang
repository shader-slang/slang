module helper;

namespace Foo
{
    public int moduleHelper(int x) { return x + 1; }
}

// A `using namespace` in a module's PRIMARY source file must affect lookup only
// within this module; it must not become part of the module's imported surface.
// See shader-slang/slang#11443.
using namespace Foo;
