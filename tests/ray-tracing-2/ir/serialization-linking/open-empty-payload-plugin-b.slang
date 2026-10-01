module structural_open_empty_payload_plugin_b;

import slang.raytracing;
import structural_open_empty_payload_context;

struct AmbiguousMissContextB : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = OtherEmptyPayload;
    typealias Record = void;
}

struct AmbiguousMissB : IAmbiguousMissTag
{
    typealias Context = AmbiguousMissContextB;
    void invoke(rt::MissInput<Context> input) {}
}
