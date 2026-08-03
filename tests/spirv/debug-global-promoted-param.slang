//TEST:SIMPLE(filecheck=CHECK):-target spirv-assembly -emit-spirv-directly -g2 -entry main -stage anyhit

// A whole entry-point parameter promoted to an interface global is still reported as
// a debug global. Only the per-field leaves of a scalarized `struct` varying are
// suppressed, so ray-tracing payload/attribute parameters keep their records.
// See debug-global-no-loc.slang for the suppressed case.

// CHECK-DAG: OpExtInst {{.*}} Debug{{Global}}Variable {{.*}} %payload
// CHECK-DAG: OpExtInst {{.*}} Debug{{Global}}Variable {{.*}} %attr

struct Payload { float value; };

[shader("anyhit")]
void main(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.value += attr.barycentrics.x;
}
