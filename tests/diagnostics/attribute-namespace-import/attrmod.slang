module attrmod;

// A user-defined attribute declared inside a namespace, in its own module. When another module
// imports this one and re-opens the same namespace, the two fragments become sibling scopes of one
// merged namespace; resolving the qualified attribute must search across those sibling scopes.
namespace shared_ns
{
    [__AttributeUsage(_AttributeTargets.Function)]
    public struct ImportedAttribute
    {
        int iParam;
    };

    // A second attribute whose name is also defined in the importing module's fragment of this
    // namespace, so a qualified reference to it is ambiguous.
    [__AttributeUsage(_AttributeTargets.Function)]
    public struct AmbiguousAttribute
    {
        int iParam;
    };
}
