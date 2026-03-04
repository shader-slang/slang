//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

struct T { int x; int y; }

[vk::constant_id(1)]
/*diag:
     ^^^^^^^^^^^ specialization constant type error
     ^^^^^^^^^^^ specialization constant must be a scalar.
*/
const T st;

[vk::constant_id(1)]
static const int x = 2;
/*diag:
                 ^ push/specialization constant storage class error
                 ^ push or specialization constants cannot be 'static'.
*/

[push_constant]
[vk::constant_id(1)]
/*diag:
     ^^^^^^^^^^^ specialization constant type error
     ^^^^^^^^^^^ specialization constant must be a scalar.
*/
const int y;
/*diag:
          ^ conflicting constant qualifiers
          ^ 'y' cannot be a push constant and a specialization constant at the same time
*/
