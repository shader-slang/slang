//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// This test verifies various things that can be marked as deprecated

[deprecated("This interface is deprecated")]
interface IDeprecated
{

}

struct UserOfDeprecatedInterface : IDeprecated
//CHECK:                           ^^^^^^^^^^^ use of deprecated declaration
//CHECK:                           ^^^^^^^^^^^ IDeprecated has been deprecated: This interface is deprecated
{

};

[deprecated("This struct is deprecated")]
struct DeprecatedStruct
{

    [deprecated("This field is deprecated")]
    int deprecatedField;


    __init(int initialFieldValue)
    {
        deprecatedField = initialFieldValue;
//CHECK:^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:^^^^^^^^^^^^^^^ deprecatedField has been deprecated: This field is deprecated
    }

    [mutating]
    void decrementDeprecatedField()
    {
        --deprecatedField;
//CHECK:  ^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:  ^^^^^^^^^^^^^^^ deprecatedField has been deprecated: This field is deprecated
    }
}

extension DeprecatedStruct
//CHECK:  ^^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:  ^^^^^^^^^^^^^^^^ DeprecatedStruct has been deprecated: This struct is deprecated
{
    [deprecated("This extension-provided member function is deprecated")]
    void deprecatedExtendedFunction()
    {
    }

    [mutating]
    void incrementDeprecatedField()
    {
        ++deprecatedField;
//CHECK:  ^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:  ^^^^^^^^^^^^^^^ deprecatedField has been deprecated: This field is deprecated
    }
}

[deprecated("This function is deprecated")]
void deprecatedFunction()
{
}

[deprecated("This typealias is deprecated")]
typealias DeprecatedTypeDef = int;

[deprecated("This enum is deprecated")]
enum DeprecatedEnum
{
    DeprecatedValue, // note: enum values cannot be decorated with attributes
};

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    var v : DeprecatedTypeDef;
//CHECK:    ^^^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:    ^^^^^^^^^^^^^^^^^ DeprecatedTypeDef has been deprecated: This typealias is deprecated

    var s : DeprecatedStruct;
//CHECK:    ^^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:    ^^^^^^^^^^^^^^^^ DeprecatedStruct has been deprecated: This struct is deprecated

    s.deprecatedField = 1;
/*CHECK:
      ^^^^^^^^^^^^^^^ use of deprecated declaration
      ^^^^^^^^^^^^^^^ deprecatedField has been deprecated: This field is deprecated
*/

    var if : IDeprecated;
//CHECK:     ^^^^^^^^^^^ use of deprecated declaration
//CHECK:     ^^^^^^^^^^^ IDeprecated has been deprecated: This interface is deprecated

    deprecatedFunction();
/*CHECK:
    ^^^^^^^^^^^^^^^^^^ use of deprecated declaration
    ^^^^^^^^^^^^^^^^^^ deprecatedFunction has been deprecated: This function is deprecated
*/

    s.deprecatedExtendedFunction();
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^^^^^^^^ use of deprecated declaration
      ^^^^^^^^^^^^^^^^^^^^^^^^^^ deprecatedExtendedFunction has been deprecated: This extension-provided member function is deprecated
*/

    var e : DeprecatedEnum;
//CHECK:    ^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:    ^^^^^^^^^^^^^^ DeprecatedEnum has been deprecated: This enum is deprecated

    var ev = DeprecatedEnum.DeprecatedValue;
//CHECK:     ^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:     ^^^^^^^^^^^^^^ DeprecatedEnum has been deprecated: This enum is deprecated
}
