module structural_rt_higher_order_legacy_helper;

struct LegacyPayload
{
    uint value;
}

RaytracingAccelerationStructure legacyScene;

public void legacyTraceLeaf()
{
    RayDesc ray = {};
    LegacyPayload payload = {};
    TraceRay(legacyScene, 0, 0xff, 0, 0, 0, ray, payload);
}

public void invokeFunction(functype()->void function)
{
    // The callee remains an IR parameter until higher-order specialization clones this helper for
    // the concrete function value supplied at its call site.
    function();
}
