//TEST:SIMPLE(filecheck=CHECK): -target spirv
// CHECK: OpEntryPoint

__generic <Scalar : __BuiltinFloatingPointType, int Mode>
struct Spectrum
{
	static const int Samples = Mode & 0xFF;
	typealias VecT = vector<Scalar, Samples>;
	VecT data;
	
	static const bool IsRGB = (Mode & 0x100) != 0;

	__generic <Float : __BuiltinFloatingPointType>
	__init(vector<Float, Samples> v)
	{
		this.data = VecT(v);
	}

	static This MakeFromRGB<Float : __BuiltinFloatingPointType>(vector<Float, 3> rgb)
	{
		if(IsRGB)
		{
			return (Spectrum<Scalar, 0x103>(rgb) as This).value;
		}
		else
		{
			return {};
		}
	}
}

static const int DefaultMode = 0x103;
typealias Spec = Spectrum<float, DefaultMode>;

[shader("vertex")]
Spec main(float3 vertex_color : COLOR0) : COLOR0
{
	return Spec::MakeFromRGB(vertex_color);
}
