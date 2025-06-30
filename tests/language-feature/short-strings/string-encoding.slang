
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

uint decodeNextUnicodePointFromUTF8<ByteReader : IMutatingFunc<uint>>(inout ByteReader utf8Reader)
{
	uint codePoint = 0;
	uint leading = utf8Reader();
	uint mask = 0x80;
	uint count = 0;
	[MaxIters(4)]
	while ((leading & mask) != 0)
	{
		count++;
		mask >>= 1;
	}
	codePoint = (leading & (mask - 1));
	for (uint i = 1; i <= count - 1; i++)
	{
		codePoint <<= 6;
		codePoint += (utf8Reader() & 0x3F);
	}
	return codePoint;
}

__generic<int N>
extension ShortString<N>
{
	struct Reader : IMutatingFunc<uint>
	{
		Array<uint, N> str;
		uint it = 0;

		__init(const in ShortString<N> str, uint it = 0)
		{
			this.str = str.getArray();
			this.it = it;
		}

		[mutating]
		uint operator()()
		{
			return (it < N) ? str[it++] : 0;
		}
	}
	
	Reader begin()
	{
		return Reader(this);
	}
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	Array<uint, 3> c = {'文', '字', '列'};
	let str = "文字列";
	var it = str.begin();
	// CHECK: 0
	output[0] = (decodeNextUnicodePointFromUTF8(it) - c[0]);
	// CHECK: 0
	output[1] = (decodeNextUnicodePointFromUTF8(it) - c[1]);
	// CHECK: 0
	output[2] = (decodeNextUnicodePointFromUTF8(it) - c[2]);
}