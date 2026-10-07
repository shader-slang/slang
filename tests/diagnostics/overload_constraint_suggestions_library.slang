// Helper module for overload-constraint-suggestions-hidden.slang.
public interface IBound<X> {}
public struct Holder
{
    internal struct Secret {}
    public static void need<U : IBound<Secret>>(U value) {}
}
