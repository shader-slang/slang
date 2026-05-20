//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("");
    //CHECK: printf("")
}
