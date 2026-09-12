module structural_open_section_plugin_a;

import slang.raytracing;
import structural_open_section_context;

struct ALinkedClosestHit : rt::IClosestHitShader
{
    typealias Context = LinkedHitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 1;
    }
}

struct ALinkedHitGroup : IHitTag
{
    typealias Context = LinkedHitContext;
    typealias ClosestHit = ALinkedClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct BDerivedClosestHit : rt::IClosestHitShader
{
    typealias Context = LinkedHitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 2;
    }
}

// Normal subtype semantics make this entry a member of OpenHitGroups<IHitTag> even though its
// declared conformance names the derived tag.
struct BDerivedHitGroup : IDerivedHitTag
{
    typealias Context = LinkedHitContext;
    typealias ClosestHit = BDerivedClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

public struct CGenericClosestHit<T> : rt::IClosestHitShader
{
    typealias Context = LinkedHitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 11;
    }
}

// A generic declaration represents an unbounded family and is not an automatic open-section
// candidate. The focused test explicitly composes CGenericHitGroup<uint> as a TypeConformance
// component, which gives the producer one exact finite specialization to catalog.
public struct CGenericHitGroup<T> : IHitTag
{
    typealias Context = LinkedHitContext;
    typealias ClosestHit = CGenericClosestHit<T>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ALinkedMiss : IMissTag
{
    typealias Context = LinkedMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 3;
    }
}

struct ALinkedCallable : ICallableTag
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input)
    {
        input.data.value += 4;
    }
}
