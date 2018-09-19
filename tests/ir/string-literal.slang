//TEST(compute):SIMPLE:-dump-ir -profile cs_5_0 -entry main

[numthreads(1, 1, 1)]
void main(
    uint tid   : SV_DispatchThreadIndex)
{
	"Hello \t\n\0x083 World";
}

