//TEST_IGNORE_FILE:

// A module that is not `public module`, so its declarations default to internal visibility.
// Each differentiable method below is generic and nested in two or more generic levels; for the
// interface default methods, the second level is the implicit `This` generic that wraps a default
// implementation. The compiler synthesizes derivative members for these methods in extensions that
// are lifted out of all of those generic levels, and those members see the outer generic
// parameters with the method's visibility.
module "nested-generic-method-visibility-lib";

public interface IScale<T : __BuiltinFloatingPointType>
{
    [Differentiable]
    public T square<Index : __BuiltinIntegerType>(T x, Index idx) { return x * x; }

    [Differentiable]
    public static T squareStatic<Index : __BuiltinIntegerType>(T x, Index idx) { return x * x; }
}

public struct Scaler : IScale<float>
{
}

public struct Outer<T : __BuiltinFloatingPointType>
{
    public struct Inner<B>
    {
        [Differentiable]
        public T cube<Index : __BuiltinIntegerType>(T x, Index idx) { return x * x * x; }

        [Differentiable]
        public static T cubeStatic<Index : __BuiltinIntegerType>(T x, Index idx)
        {
            return x * x * x;
        }

        [Differentiable]
        internal static T cubeInternal<Index : __BuiltinIntegerType>(T x, Index idx)
        {
            return x * x * x;
        }

        // A private method gets an internal synthesized extension with private members, so its
        // derivatives take a different visibility path from the public methods above.
        [Differentiable]
        private static T cubePrivate<Index : __BuiltinIntegerType>(T x, Index idx)
        {
            return x * x * x;
        }

        // Under Slang 2026, an unmodified member inherits the visibility of `Inner`, so this
        // method is public and its derivatives take the same path as `cubeStatic`. Under older
        // language versions it is internal, and it only needs to compile.
        [Differentiable]
        static T cubeInherited<Index : __BuiltinIntegerType>(T x, Index idx)
        {
            return x * x * x;
        }

        [Differentiable]
        public static T cubeViaPrivate<Index : __BuiltinIntegerType>(T x, Index idx)
        {
            return cubePrivate(x, idx);
        }
    }
}

public struct Wrapper<T : IDifferentiable>
{
    public struct Inner<B>
    {
        [Differentiable]
        public static T identity<Index : __BuiltinIntegerType>(T x, Index idx) { return x; }

        // The user-written derivative doubles the incoming gradient so that a caller can tell it
        // apart from a synthesized one.
        [BackwardDerivative(identityWithCustomBwdImpl)]
        public static T identityWithCustomBwd<Index : __BuiltinIntegerType>(T x, Index idx)
        {
            return x;
        }

        public static void identityWithCustomBwdImpl<Index : __BuiltinIntegerType>(
            inout DifferentialPair<T> x,
            Index idx,
            T.Differential dOut)
        {
            x = diffPair(x.p, T.dadd(dOut, dOut));
        }
    }
}

public struct L1<A : __BuiltinFloatingPointType>
{
    public struct L2<B : __BuiltinFloatingPointType>
    {
        public struct L3<C : __BuiltinFloatingPointType>
        {
            [Differentiable]
            public static A squareA<Index : __BuiltinIntegerType>(A x, Index idx) { return x * x; }

            [Differentiable]
            public static B squareB<Index : __BuiltinIntegerType>(B x, Index idx) { return x * x; }

            [Differentiable]
            public static C squareC<Index : __BuiltinIntegerType>(C x, Index idx) { return x * x; }
        }
    }
}
