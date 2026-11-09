// Library module for the qualified-namespace-reopen-import regression test
// (https://github.com/shader-slang/slang/issues/13701).

module qualified_reopen_lib;

namespace Outer
{
public struct ImportedType
{
    public int value;
}
}
