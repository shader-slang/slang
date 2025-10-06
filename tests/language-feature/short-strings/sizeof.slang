//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj

// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;



void test<int N>(const in ShortString<N> str, inout uint i)
{
	uint expectedSize = 0;
	__target_switch
	{
		case cpp:
		case cuda:
		{
			expectedSize = sizeof(uintptr_t);
			break;
		}
		case hlsl:
		case wgsl:
		{
			expectedSize = ((N + 3) / 4) * 4;
			break;
		}
		default:
		{
			expectedSize = N;
			break;
		}
	}
	output[i++] = sizeof(str) - expectedSize;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	uint i = 0;
	test("abc", i);
	test("xywz", i);
	test("&_$='#", i);
	test("", i);
}