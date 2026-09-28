module helper;

namespace Foo
{
    public int moduleHelper(int x) { return x + 1; }
}

// Primary-file `using`: lookup-local to this module (see shader-slang/slang#11443).
using namespace Foo;
