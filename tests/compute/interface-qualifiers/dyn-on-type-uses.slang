//TEST:SIMPLE(filecheck=CHECK_2026): -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK_2025_OR_EXP): -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_2025_OR_EXP): -stage compute -entry computeMain -lang slang -std 2026 -enable-experimental-dynamic-dispatch

// Validate the existing restrictions on interfaces with an explicit AnyValue size.

// `experimental-dynamic-dispatch` and `-lang 2025` have fewer restrictions
//
// CHECK_2025_OR_EXP-NOT: E33072
// CHECK_2025_OR_EXP-NOT: E33073
// CHECK_2025_OR_EXP-NOT: E33074
// CHECK_2025_OR_EXP-NOT: E33075
// CHECK_2025_OR_EXP-NOT: E33076
// CHECK_2025_OR_EXP-NOT: E33077
// CHECK_2025_OR_EXP-NOT: E33078
// CHECK_2025_OR_EXP-NOT: E33082

/////////////////////////////////////////////////////

// An interface declaration with `[anyValueSize(...)]` must not have generic parameters.
// CHECK_2026-DAG: error[E33072]
[anyValueSize(64)]
interface interface1<T>
{
	static const int member;
};

// Associated types must not be declared in an interface declaration with `[anyValueSize(...)]`.
// CHECK_2026-DAG: error[E33073]
interface IBase1
{
};
[anyValueSize(64)]
interface interface2
{
    associatedtype IBase1;
};

// Methods with generic parameters must not be declared in an interface declaration with
// `[anyValueSize(...)]`.
// CHECK_2026-DAG: error[E33074]
[anyValueSize(64)]
interface interface3
{
	T genericFunc<T : IArithmetic>(T val)
	{
		return val*(T)2;
	}
};

// Methods with `[mutating]` must not be declared in an interface declaration with
// `[anyValueSize(...)]`.
// CHECK_2026-DAG: error[E33075]
[anyValueSize(64)]
interface interface4
{
	[mutating]
	void mutate(int val);
};

// An interface declaration with `[anyValueSize(...)]` must only refine interfaces whose
// declarations also have that attribute.
// CHECK_2026-DAG: error[E33077]

interface IBase2
{
};

[anyValueSize(64)]
interface interface5 : IBase2
{	
	int myFunc(int val)
	{
		return val*2;
	}
};

[anyValueSize(64)]
interface IBase3
{
};

[anyValueSize(64)]
interface interface6 : IBase3
{	
	int myFunc(int val)
	{
		return val*2;
	}
};

// Methods with `[Differentiable]` must not be declared in an interface declaration with
// `[anyValueSize(...)]`.
// CHECK_2026-DAG: error[E33076]
[anyValueSize(64)]
interface interface7
{
	[Differentiable]
	int myFunc(int val)
	{
		return val*2;
	}
};

// An inheritance clause in a generic type declaration must not name an interface whose
// declaration has an `[anyValueSize(...)]` attribute.
// CHECK_2026-DAG: error[E33082]
// CHECK_2026-DAG: error[E33082]
[anyValueSize(64)]
interface IBase4
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

// An extension must not add a conformance to an interface whose declaration has
// an `[anyValueSize(...)]` attribute.
// CHECK_2026-DAG: error[E33078]
[anyValueSize(64)]
interface IBase5
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

// Direct variable members of a type conforming to an interface whose declaration has
// an `[anyValueSize(...)]` attribute must not have opaque, non-copyable, or unsized types.
// CHECK_2026-DAG: error[E33079]
// CHECK_2026-DAG: error[E33080]
// CHECK_2026-DAG: error[E33081]
// CHECK_2026-DAG: error[E33081]

// CHECK_2025_OR_EXP-DAG: error[E33079]
// CHECK_2025_OR_EXP-DAG: error[E33080]
// CHECK_2025_OR_EXP-DAG: error[E33081]

struct structWithUnsized : IBase6
{
    int v[];
};

struct structWithOpaque : IBase6
{
	Texture2D<float> v;
};

[anyValueSize(64)]
interface IBase6
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

// Methods with `some IFoo` result or parameter types must not be declared in an interface
// whose declaration has an `[anyValueSize(...)]` attribute.
// TODO-INTERFACE-QUALIFIERS-ADDITION-OF-SOME
//

RWStructuredBuffer<int> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	outputBuffer[0] = 0;
}
