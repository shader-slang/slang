//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Error: private visibility on global struct
private struct MyStruct
/*CHECK:
^^^^^^^ invalid private visibility
^^^^^^^ 'MyStruct' cannot have private visibility because it is not a member of a type.
*/
{}

struct IS
{
    // Error: public method in non-public struct
    public void fp();
//CHECK:        ^^ visibility higher than parent
//CHECK:        ^^ 'fp' cannot have a higher visibility than 'IS'.
}

// Error: public function returning internal type
public IS f() {}
//CHECK:  ^ references less visible type
//CHECK:  ^ 'f' references less visible type 'IS'.

public struct PS
//CHECK:      ^^ references less visible type
//CHECK:      ^^ '$init' references less visible type 'IS'.
{
    // Error: public member of internal type
    public IS ii;
//CHECK:      ^^ references less visible type
//CHECK:      ^^ 'ii' references less visible type 'IS'.

    // Error: public property of internal type
    public property IS ii2 { get { return {}; } };
//CHECK:               ^^^ references less visible type
//CHECK:               ^^^ 'ii2' references less visible type 'IS'.
}
