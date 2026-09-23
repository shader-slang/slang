//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target cpp

// Taking an address does not read the stored value. Returning the value through that address does,
// so the consuming return statement is diagnosed rather than `__getAddress`.
int read_through_address()
{
    int value;
    int* address = __getAddress(value);
    return *address;
/*CHK:
    ^^^^^^ use of uninitialized variable
    ^^^^^^ use of uninitialized variable 'value'
*/
}

// A store through the derived address definitely initializes the original variable.
int write_through_address()
{
    int value;
    int* address = __getAddress(value);
    *address = 1;
    return value;
}

// Advancing from the first element to the second element does not initialize the first. The
// nonzero pointer offset is a possible connection to the array storage, but not a definite write
// to the value returned below.
int write_through_nonzero_offset()
{
    int values[2];
    int* first = &values[0];
    int* second = __getOffsetPtr(first, 1);
    *second = 1;
    return values[0];
/*CHK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'values' may be uninitialized on some paths; it is only conditionally assigned
*/
}

// Converting an address to an integer observes the address, not the value stored at that address.
uint64_t convert_address_to_integer()
{
    int value;
    return reinterpret<uint64_t>(&value);
}

// Comparing an address observes only the address. It does not read the value stored there.
bool compare_address_to_null()
{
    int value;
    return &value == nullptr;
}

// Relational pointer comparisons likewise observe only the two addresses.
bool compare_addresses()
{
    int left;
    int right;
    return &left < &right;
}

// Returning a local address exports the address without reading its pointee.
int* return_address()
{
    int value;
    return &value;
}

// A pointer value is different from the address of the variable that stores it. Converting an
// uninitialized pointer observes the pointer value and must be diagnosed.
uint64_t convert_uninitialized_pointer_to_integer()
{
    int* pointer;
    return reinterpret<uint64_t>(pointer);
//CHK:                          ^ use of uninitialized variable
//CHK:                          ^ use of uninitialized variable 'pointer'
}

// Copying an uninitialized pointer also reads the pointer value. It does not initialize the pointer
// merely because the copied value can later be used as an address.
void copy_uninitialized_pointer(out int* destination)
{
    int* pointer;
    destination = pointer;
//CHK:          ^ use of uninitialized variable
//CHK:          ^ use of uninitialized variable 'pointer'
}

struct EmptyPointee
{}

// Whether a pointer value needs initialization does not depend on the runtime representation of
// its pointee. In particular, a pointer to an empty structure still contains an address.
bool compare_uninitialized_empty_pointer_to_null()
{
    EmptyPointee* pointer;
    return pointer == nullptr;
//CHK:             ^^ use of uninitialized variable
//CHK:             ^^ use of uninitialized variable 'pointer'
}

// An index selects an element; it is not the base value projected by the element-address
// instruction. The checker must therefore diagnose the read of `index` instead of following the
// resulting address as though it were an alias of `index`.
int read_uninitialized_index()
{
    int values[1] = {0};
    int index;
    values[index] = 1;
//CHK:    ^ use of uninitialized variable
//CHK:    ^ use of uninitialized variable 'index'
    return values[0];
}

// Both incoming addresses name `value`, so the merged address does too. A store through that
// address therefore initializes `value` on every path.
int conditional_same_root_pointer_write(bool condition)
{
    int value;
    int* address;
    if (condition)
        address = &value;
    else
        address = __getAddress(value);
    *address = 1;
    return value;
}

// In contrast, this merged address can name either local variable. A store through the address is
// only a possible write to `value`, so it cannot prove that `value` is initialized when
// `condition` is false.
int conditional_pointer_write(bool condition)
{
    int value;
    int other = 0;
    int* address;
    if (condition)
        address = &value;
    else
        address = &other;
    *address = 1;
    return value;
/*CHK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'value' may be uninitialized on some paths; it is only conditionally assigned
*/
}

// The loop-header parameter can receive the address carried by the preceding iteration on a back
// edge, but every incoming address still names `value`. A store through the address therefore
// initializes `value` regardless of the loop's trip count.
int loop_carried_same_root_pointer_write(int count)
{
    int value;
    int* address = &value;
    for (int i = 0; i < count; ++i)
        address = __getAddress(value);
    *address = 1;
    return value;
}

// This loop-header address can instead receive `&other` on a backedge. The final store is only a
// possible write to `value`, so the return must retain the possibly-uninitialized diagnostic.
int loop_carried_mixed_root_pointer_write(int count, bool chooseOther)
{
    int value;
    int other = 0;
    int* address = &value;
    for (int i = 0; i < count; ++i)
    {
        if (chooseOther)
            address = &other;
        else
            address = __getAddress(value);
    }
    *address = 1;
    return value;
/*CHK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'value' may be uninitialized on some paths; it is only conditionally assigned
*/
}

// Loading a pointer value from a global reads the global. The fact that the result is also a
// pointer does not make the load a possible write to the global.
static int* uninitializedPointer;
static EmptyPointee* uninitializedEmptyPointer;

int* read_uninitialized_pointer_global()
{
    return uninitializedPointer;
/*CHK:
    ^^^^^^ use of uninitialized global variable
    ^^^^^^ use of uninitialized global variable 'uninitializedPointer'
*/
}

EmptyPointee* read_uninitialized_empty_pointer_global()
{
    return uninitializedEmptyPointer;
/*CHK:
    ^^^^^^ use of uninitialized global variable
    ^^^^^^ use of uninitialized global variable 'uninitializedEmptyPointer'
*/
}
