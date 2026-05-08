//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface IRayQuery
{
}

__generic<let Flags : RAY_FLAG>
extension RayQuery<Flags> : IRayQuery
//CHECK:                    ^^^^^^^^^ non-copyable interface conformance is not fully supported
//CHECK:                    ^^^^^^^^^ conforming non-copyable type 'RayQuery<Flags>' to interface 'IRayQuery' is not properly supported and may lead to compiler crashes.
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
