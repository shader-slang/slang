// Library module for the namespace-import-nested regression test
// (https://github.com/shader-slang/slang/issues/11442).
//
// Defines a type inside a namespace that the importing module reopens.

module nested_namespace_lib;

namespace Foo
{
    public struct ImportedType
    {
        public int value;
    }
}
