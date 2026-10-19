// Library module for the namespace-import-reopen regression tests
// (https://github.com/shader-slang/slang/issues/11442 and
// https://github.com/shader-slang/slang/issues/13714).

module reopen_lib;

namespace Foo
{
public struct ImportedType
{
    public int value;
}

public static const int importedValue = 7;
}
