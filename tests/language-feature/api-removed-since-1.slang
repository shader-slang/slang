//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// This test verifies various things that can be marked as removed.

[RemovedSince(-1, "This interface is removed")]
interface IRemoved
{

}

struct UserOfRemovedInterface : IRemoved
//CHECK:                        ^^^^^^^^ use of removed declaration
//CHECK:                        ^^^^^^^^ IRemoved has been removed since language version '-1': This interface is removed
{

};

[RemovedSince(-1, "This struct is removed")]
struct RemovedStruct
{

    [RemovedSince(-1, "This field is removed")]
    int removedField;


    __init(int initialFieldValue)
    {
        removedField = initialFieldValue;
//CHECK:^^^^^^^^^^^^ use of removed declaration
//CHECK:^^^^^^^^^^^^ removedField has been removed since language version '-1': This field is removed
    }

    [mutating]
    void decrementRemovedField()
    {
        --removedField;
//CHECK:  ^^^^^^^^^^^^ use of removed declaration
//CHECK:  ^^^^^^^^^^^^ removedField has been removed since language version '-1': This field is removed
    }
}

extension RemovedStruct
//CHECK:  ^^^^^^^^^^^^^ use of removed declaration
//CHECK:  ^^^^^^^^^^^^^ RemovedStruct has been removed since language version '-1': This struct is removed
{
    [RemovedSince(-1, "This extension-provided member function is removed")]
    void removedExtendedFunction()
    {
    }

    [mutating]
    void incrementRemovedField()
    {
        ++removedField;
//CHECK:  ^^^^^^^^^^^^ use of removed declaration
//CHECK:  ^^^^^^^^^^^^ removedField has been removed since language version '-1': This field is removed
    }
}

[RemovedSince(-1, "This function is removed")]
void removedFunction()
{
}

[RemovedSince(-1, "This typealias is removed")]
typealias RemovedTypeDef = int;

[RemovedSince(-1, "This enum is removed")]
enum RemovedEnum
{
    RemovedValue, // note: enum values cannot be decorated with attributes
};

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    var v : RemovedTypeDef;
//CHECK:    ^^^^^^^^^^^^^^ use of removed declaration
//CHECK:    ^^^^^^^^^^^^^^ RemovedTypeDef has been removed since language version '-1': This typealias is removed

    var s : RemovedStruct;
//CHECK:    ^^^^^^^^^^^^^ use of removed declaration
//CHECK:    ^^^^^^^^^^^^^ RemovedStruct has been removed since language version '-1': This struct is removed

    s.removedField = 1;
/*CHECK:
      ^^^^^^^^^^^^ use of removed declaration
      ^^^^^^^^^^^^ removedField has been removed since language version '-1': This field is removed
*/

    var if : IRemoved;
//CHECK:     ^^^^^^^^ use of removed declaration
//CHECK:     ^^^^^^^^ IRemoved has been removed since language version '-1': This interface is removed

    removedFunction();
/*CHECK:
    ^^^^^^^^^^^^^^^ use of removed declaration
    ^^^^^^^^^^^^^^^ removedFunction has been removed since language version '-1': This function is removed
*/

    s.removedExtendedFunction();
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^^^^^ use of removed declaration
      ^^^^^^^^^^^^^^^^^^^^^^^ removedExtendedFunction has been removed since language version '-1': This extension-provided member function is removed
*/

    var e : RemovedEnum;
//CHECK:    ^^^^^^^^^^^ use of removed declaration
//CHECK:    ^^^^^^^^^^^ RemovedEnum has been removed since language version '-1': This enum is removed

    var ev = RemovedEnum.RemovedValue;
//CHECK:     ^^^^^^^^^^^ use of removed declaration
//CHECK:     ^^^^^^^^^^^ RemovedEnum has been removed since language version '-1': This enum is removed

}
