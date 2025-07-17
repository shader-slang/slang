[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void main()
{
    SetMeshOutputCounts(1, 1);
}