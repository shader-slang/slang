module structural_open_section_plugin_z;

import slang.raytracing;
import structural_open_section_context;

struct ZLinkedClosestHit : rt::IClosestHitShader
{
    typealias Context = LinkedHitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 5;
    }
}

struct ZLinkedHitGroup : IHitTag
{
    typealias Context = LinkedHitContext;
    typealias ClosestHit = ZLinkedClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ZLinkedMiss : IMissTag
{
    typealias Context = LinkedMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 6;
    }
}

struct ZLinkedCallable : ICallableTag
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input)
    {
        input.data.value += 7;
    }
}
