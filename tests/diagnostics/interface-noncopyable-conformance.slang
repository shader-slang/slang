//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface IRayQuery
{
}

__generic<let Flags : RAY_FLAG>
extension RayQuery<Flags> : IRayQuery
//CHECK:                    ^^^^^^^^^ non-copyable type cannot conform to interface
//CHECK:                    ^^^^^^^^^ non-copyable type 'RayQuery<Flags>' cannot conform to interface 'IRayQuery' because interfaces are copyable by default.
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
