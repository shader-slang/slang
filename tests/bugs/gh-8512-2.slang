//TEST:INTERPRET(filecheck=ICHECK): -disasm
// ICHECK: main

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

int main()
{
	float3 color = {1, 2, 3};
	return int(Spec::MakeFromRGB(color).data[0]);
}
