// Tests various bad placements for type declarations as prohibited by the language.
//
// Second parser pass checks
//
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

void testGenericFunc<T>(T t)
{
//CHECK: E30102
//CHECK: bad-declaration-placement-2.slang:[[# @LINE+1]]:
    if (T is struct STRUC1a { })
    {
    }

//CHECK: E30102
//CHECK: bad-declaration-placement-2.slang:[[# @LINE+1]]:
    Optional<int> optV = t as struct STRUC1b { };
}

void computeMain()
{
//CHECK: E30102
//CHECK: bad-declaration-placement-2.slang:[[# @LINE+1]]:
    let i : struct STRUC2a { } = { };

//CHECK: E30102
//CHECK: bad-declaration-placement-2.slang:[[# @LINE+1]]:
    var j : struct STRUC2b { };

// note: this is legal
//CHECK-NOT: bad-declaration-placement-2.slang:[[# @LINE+1]]:
    struct STRUC2c { } k = { };


//CHECK: E30102
//CHECK: bad-declaration-placement-2.slang:[[# @LINE+1]]:
    let m : int = (struct STRUC2d { })3;

}
