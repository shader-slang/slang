//TEST(compute):SIMPLE:-dump-ir -stage compute -entry main

[numthreads(1, 1, 1)]
void main(
    uint tid   : SV_DispatchThreadIndex)
{
	"Hello \t\n\0x083 World";
}

