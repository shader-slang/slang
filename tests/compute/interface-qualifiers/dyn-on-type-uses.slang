//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_2025_OR_EXP): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_2025_OR_EXP): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -enable-experimental-dynamic-dispatch

// Validate AST side of `dyn` keyword use-cases

// `experimental-dynamic-dispatch` and `-lang 2025` have fewer restrictions
//
// CHECK_2025_OR_EXP-NOT: error 33072
// CHECK_2025_OR_EXP-NOT: error 33073
// CHECK_2025_OR_EXP-NOT: error 33074
// CHECK_2025_OR_EXP-NOT: error 33075
// CHECK_2025_OR_EXP-NOT: error 33076
// CHECK_2025_OR_EXP-NOT: error 33077
// CHECK_2025_OR_EXP-NOT: error 33078
// CHECK_2025_OR_EXP-NOT: error 33082

/////////////////////////////////////////////////////

// `dyn` interfaces are not allowed to be generics
// CHECK_2026-DAG: error 33072
dyn interface interface1<T>
{
	static const int member;
};

// `dyn` interfaces must not define any associated types.
// CHECK_2026-DAG: error 33073
interface IBase1
{
};
dyn interface interface2
{
    associatedtype IBase1;
};

// `dyn` interfaces must not define any generic methods.
// CHECK_2026-DAG: error 33074
dyn interface interface3
{
	T genericFunc<T : IArithmetic>(T val)
	{
		return val*(T)2;
	}
};

// `dyn` interfaces must not define any mutating methods
// CHECK_2026-DAG: error 33075
dyn interface interface4
{
	[mutating]
	void mutate(int val);
};

// `dyn` interfaces cannot inherit from any interfaces that are not dyn.
// CHECK_2026-DAG: error 33077

interface IBase2
{
};

dyn interface interface5 : IBase2
{	
	int myFunc(int val)
	{
		return val*2;
	}
};

dyn interface IBase3
{
};

dyn interface interface6 : IBase3
{	
	int myFunc(int val)
	{
		return val*2;
	}
};

// `dyn` interfaces cannot contain any function requirements that are marked as [Differentiable].
// CHECK_2026-DAG: error 33076
dyn interface interface7
{
	[Differentiable]
	int myFunc(int val)
	{
		return val*2;
	}
};

// The type which is conforming to a dyn (myType in interface myType : IBase) cannot be generic.
// CHECK_2026-DAG: error 33082
// CHECK_2026-DAG: error 33082
dyn interface IBase4
{
	int doMath(int val);
};

struct genericStruct1<T : IArithmetic> : IBase4
{
	T a;
    int doMath(int v)
	{
		return v * (int)2;
	}
};

interface genericInterface1<T : IArithmetic> : IBase4
{
    T doMath(T v)
	{
		return v * (T)3;
	}
};

// Extensions that make a type conform/inherit to dyn interfaces are not allowed.
// CHECK_2026-DAG: error 33078
dyn interface IBase5
{
};
interface interface9
{
};

extension<T:interface9> T : IBase5
{
	int doMath()
	{
		return 5;
	}
};

// Type conforming to `dyn` interface must be an ordinary data type, meaning that it cannot contain any fields that are opaque or non-copyable or unsized.
// CHECK_2026-DAG: error 33079
// CHECK_2026-DAG: error 33080
// CHECK_2026-DAG: error 33081
// CHECK_2026-DAG: error 33081

// CHECK_2025_OR_EXP-DAG: error 33079
// CHECK_2025_OR_EXP-DAG: error 33080
// CHECK_2025_OR_EXP-DAG: error 33081

struct structWithUnsized : IBase6
{
    int v[];
};

struct structWithOpaque : IBase6
{
	Texture2D<float> v;
};

dyn interface IBase6
{
};
[__NonCopyableType] 
struct NonCopyableStruct1
{
    float v;
}
struct structWithNonCopyable1 : IBase6
{
    NonCopyableStruct1 v;
};
[__NonCopyableType]
struct NonCopyableStruct2<T : IArithmetic>
{
    T v;
}
struct structWithNonCopyable2 : IBase6
{
    NonCopyableStruct2<int> v;
};

// `dyn` interfaces cannot contain any methods that has a `some` IFoo return type, or has any `some` IFoo parameters.
// TODO-INTERFACE-QUALIFIERS-ADDITION-OF-SOME
//

RWStructuredBuffer<int> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	outputBuffer[0] = 0;
}
