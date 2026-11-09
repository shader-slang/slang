// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

public int typeIndex<T, each P>()
{
    return __index_of_type_in_pack<T, P>;
}

public int forwardedTypeIndex<T, each P>()
{
    return typeIndex<T, P>();
}

public struct TypeIndexHolder<let N : int>
{
    public static int get() { return N; }
}

public int typeIndexAsGenericArgument<T, each P>()
{
    return TypeIndexHolder<__index_of_type_in_pack<T, P>>.get();
}

public typealias TypeIndexedArray<T, each P> = int[__index_of_type_in_pack<T, P> + 1];

public struct TypeIndexWrapper<T> {}
public struct TypeIndexValueWrapper<let N : int> {}
public struct TypeIndexOuter<T>
{
    public struct Inner {}
}

public int expandedTypeIndex<T, each P>()
{
    return __index_of_type_in_pack<TypeIndexWrapper<T>, expand TypeIndexWrapper<each P>>;
}

public int trimmedTypeIndex<T, each P>()
{
    return __index_of_type_in_pack<T, __trimFirst(P)>;
}

public int valueDependentTypeIndex<let N : int>()
{
    return __index_of_type_in_pack<TypeIndexValueWrapper<4>,
        expand each Tuple<TypeIndexValueWrapper<N>, TypeIndexValueWrapper<4>>>;
}

public int partiallyKnownTypeIndex<T>()
{
    return __index_of_type_in_pack<float, expand each Tuple<T, float>>;
}

public interface ITypeIndexAssociated { associatedtype Value; }
public struct TypeIndexAssociated : ITypeIndexAssociated { public typealias Value = float; }

public int associatedTypeIndex<T : ITypeIndexAssociated, each P>()
{
    return __index_of_type_in_pack<T.Value, P>;
}
