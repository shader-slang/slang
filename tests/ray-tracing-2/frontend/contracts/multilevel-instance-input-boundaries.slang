//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target metal -no-codegen

import slang.raytracing;

struct PortableTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MultiLevelTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

struct PortableContext : rt::IHitContext
{
    typealias TraceContext = PortableTraceContext;
    typealias Payload = void;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

struct MultiLevelContext : rt::IHitContext
{
    typealias TraceContext = MultiLevelTraceContext;
    typealias Payload = void;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

// The path accessors are specific to multilevel contexts on all three hit inputs.
[require(structural_raytracing_closesthit)]
void checkPortableClosestHit(rt::ClosestHitInput<PortableContext> input)
{
    let limit = input.maxInstanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'maxInstanceCount' is not a member of 'rt.ClosestHitInput<PortableContext>'.
*/
    let count = input.instanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^ 'instanceCount' is not a member of 'rt.ClosestHitInput<PortableContext>'.
*/
    let index = input.getInstanceIndex(0);
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'getInstanceIndex' is not a member of 'rt.ClosestHitInput<PortableContext>'.
*/
    let id = input.getInstanceID(0);
/*CHECK:
                   ^^^^^^^^^^^^^ member not found
                   ^^^^^^^^^^^^^ 'getInstanceID' is not a member of 'rt.ClosestHitInput<PortableContext>'.
*/
}

[require(structural_raytracing_anyhit)]
void checkPortableAnyHit(rt::AnyHitInput<PortableContext> input)
{
    let limit = input.maxInstanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'maxInstanceCount' is not a member of 'rt.AnyHitInput<PortableContext>'.
*/
    let count = input.instanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^ 'instanceCount' is not a member of 'rt.AnyHitInput<PortableContext>'.
*/
    let index = input.getInstanceIndex(0);
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'getInstanceIndex' is not a member of 'rt.AnyHitInput<PortableContext>'.
*/
    let id = input.getInstanceID(0);
/*CHECK:
                   ^^^^^^^^^^^^^ member not found
                   ^^^^^^^^^^^^^ 'getInstanceID' is not a member of 'rt.AnyHitInput<PortableContext>'.
*/
}

[require(structural_raytracing_intersection)]
void checkPortableIntersection(rt::IntersectionInput<PortableContext> input)
{
    let limit = input.maxInstanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'maxInstanceCount' is not a member of 'rt.IntersectionInput<PortableContext>'.
*/
    let count = input.instanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^ 'instanceCount' is not a member of 'rt.IntersectionInput<PortableContext>'.
*/
    let index = input.getInstanceIndex(0);
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'getInstanceIndex' is not a member of 'rt.IntersectionInput<PortableContext>'.
*/
    let id = input.getInstanceID(0);
/*CHECK:
                   ^^^^^^^^^^^^^ member not found
                   ^^^^^^^^^^^^^ 'getInstanceID' is not a member of 'rt.IntersectionInput<PortableContext>'.
*/
}

// No instance level is silently selected for the existing scalar properties.
[require(structural_raytracing_closesthit)]
void checkMultiLevelScalars(rt::ClosestHitInput<MultiLevelContext> input)
{
    let index = input.instanceIndex;
/*CHECK:
                      ^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^ 'instanceIndex' is not a member of 'rt.ClosestHitInput<MultiLevelContext>'.
*/
    let id = input.instanceID;
/*CHECK:
                   ^^^^^^^^^^ member not found
                   ^^^^^^^^^^ 'instanceID' is not a member of 'rt.ClosestHitInput<MultiLevelContext>'.
*/
}

// A miss has no hit path, even when its trace context uses multilevel traversal.
[require(structural_raytracing_miss)]
void checkMiss(rt::MissInput<MultiLevelContext> input)
{
    let limit = input.maxInstanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'maxInstanceCount' is not a member of 'rt.MissInput<MultiLevelContext>'.
*/
    let count = input.instanceCount;
/*CHECK:
                      ^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^ 'instanceCount' is not a member of 'rt.MissInput<MultiLevelContext>'.
*/
    let index = input.getInstanceIndex(0);
/*CHECK:
                      ^^^^^^^^^^^^^^^^ member not found
                      ^^^^^^^^^^^^^^^^ 'getInstanceIndex' is not a member of 'rt.MissInput<MultiLevelContext>'.
*/
    let id = input.getInstanceID(0);
/*CHECK:
                   ^^^^^^^^^^^^^ member not found
                   ^^^^^^^^^^^^^ 'getInstanceID' is not a member of 'rt.MissInput<MultiLevelContext>'.
*/
}

// The extension-selection marker is an implementation detail, not another application protocol.
struct FakeMultiLevel : rt::IMultiLevelAccelerationStructure {}
/*CHECK:
                            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ declaration not accessible
                            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 'IMultiLevelAccelerationStructure' is not accessible from the current context.
*/
