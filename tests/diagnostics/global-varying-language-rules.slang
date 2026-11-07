// Module-scope `in`/`out` variables are a GLSL feature. GLSL accepts them silently, legacy Slang
// accepts them with a deprecation warning, and HLSL and Slang 2025 and later reject them. In every
// language they must be declared at the top level of their module, not inside a namespace.

//DIAGNOSTIC_TEST:SIMPLE(diag=LEGACY): -target spirv -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=SLANG2025): -target spirv -no-codegen -std 2025
//DIAGNOSTIC_TEST:SIMPLE(diag=SLANG2026): -target spirv -no-codegen -std 2026
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=HLSL): -lang hlsl tests/diagnostics/global-varying-language-rules.slang -target spirv -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=GLSL): -lang glsl tests/diagnostics/global-varying-language-rules.slang -target spirv -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target spirv -no-codegen -DNESTED -warnings-disable 30627

in float4 gColor;
/*LEGACY:
          ^^^^^^ module-scope varying 'gColor' is deprecated
          ^^^^^^ module-scope 'in'/'out' variables are intended for the GLSL dialect only, and support for them in Slang will be removed in a later compiler release; declare 'gColor' as an entry-point parameter instead
*/
/*SLANG2025:
          ^^^^^^ module-scope varying 'gColor' is not supported in Slang 2025 and later
          ^^^^^^ module-scope 'in'/'out' variables are a GLSL feature; declare 'gColor' as an entry-point parameter instead
*/
/*SLANG2026:
          ^^^^^^ module-scope varying 'gColor' is not supported in Slang 2025 and later
          ^^^^^^ module-scope 'in'/'out' variables are a GLSL feature; declare 'gColor' as an entry-point parameter instead
*/
/*HLSL:
          ^^^^^^ module-scope varying 'gColor' is not supported in HLSL
          ^^^^^^ module-scope 'in'/'out' variables are a GLSL feature; declare 'gColor' as an entry-point parameter instead
*/

#ifdef NESTED
namespace inner
{
in float4 gNested;
/*NESTED:
          ^^^^^^^ module-scope varying 'gNested' must be declared at the top level of its module
          ^^^^^^^ 'gNested' is an 'in'/'out' variable declared inside a namespace
*/
}
#endif

[shader("fragment")]
float4 main() : SV_Target
{
    return gColor;
}
