//TEST:INTERPRET(filecheck=ICHECK):

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

void main()
{
	float3 color = {1, 2, 3};
	let spec = Spec::MakeFromRGB(color);
	// ICHECK: (1, 2, 3)
	printf("(%.0f, %.0f, %.0f)", spec.data[0], spec.data[1], spec.data[2]);
}
