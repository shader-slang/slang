// Tests various bad placements for type declarations as prohibited by the language.
//
// First parser pass checks
//
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
struct STRUC1 { } functionReturnType1()
{
    return { };
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func functionReturnType2() -> struct STRUC2 { }
{
    return { };
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
void funcParameter1(struct STRUC3 { } a)
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func funcParameter2(struct STRUC4a { } a) -> void
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func funcParameter3(a : struct STRUC4b { }) -> void
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func genericTypeParam1<T : struct STRUC5a { }>() -> int
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func genericTypeParam2<T = struct STRUC5b { }>() -> int
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func genericValueParam1<let i : struct STRUC6a { } = { }>() -> int
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func genericValueParam2<struct STRUC6b { } a = { }>() -> int
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+2]]:
func genericWhereClause<T>() -> int
    where struct STRUC6c { } : IArithmetic
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+2]]:
func genericWhereClause<T>() -> int
    where T : struct STRUC6d { }
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+2]]:
func genericWhereClause<T>() -> int
    where T == struct STRUC6e { }
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+2]]:
func genericWhereClause<T>() -> int
    where T(struct STRUC6f { })
{
    return 0;
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
void throwingFunction1() throws struct STRUC7a { }
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
func throwingFunction2() throws struct STRUC7b { } -> void
{
}


struct TestStruct1
{
//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    property testProp : struct STRUC8a { }
    {
    }

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    property struct STRUC8b { } testProp
    {
    }

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    __subscript (struct STRUC8c { } i) -> int
    {
    }

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    __subscript (int i) -> struct STRUC8d { }
    {
    }

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    __init(struct STRUC8e { } i)
    {
    }
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
enum TestEnum1 : struct STRUC9a { }
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
enum class TestEnum2 : struct STRUC9b { }
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
typealias FuncType1 = functype(struct STRUC10a { }) -> void;

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
typealias FuncType2 = functype() -> struct STRUC10b { };

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
typealias Alias1 = struct STRUC10c { };

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
struct TestStruct2 : struct STRUC11 { }
{
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
extension struct STRUC12 { }
{
}

interface TestIface1
{
//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    associatedtype T : struct STRUC13 { };
}

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
__generic_value_param kGenericValueParam1 : struct STRUC14 { } = { };

semantic TestSemantics1
{
//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    get : struct STRUC15a;

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    set : struct STRUC15b;
}

struct TestStruct3
{
//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
    var tmp : struct STRUC16 { };
}

// Note: this is legal
//CHECK-NOT: bad-declaration-placement-1.slang:[[# @LINE+1]]:
typedef struct STRUC17 { } TestTypeDef1;

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
attribute_syntax [Attribute1(param1 : struct STRUC18 { })] : TestAttribute1;

//CHECK: E30102
//CHECK: bad-declaration-placement-1.slang:[[# @LINE+1]]:
struct TestStruct4 = struct STRUC19 { };


void computeMain()
{
}
