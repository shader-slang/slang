//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry computeMain

RWStructuredBuffer<uint32_t> buf;

// CHECK: E41033
// CHECK: left shift on narrow integer type 'uint8_t'
uint32_t set_byte32(uint32_t i, uint32_t n, uint8_t byte)
{
    return (i & ~(0xFF << (n * 8))) | (byte << (n * 8));
}

// CHECK: E41033
// CHECK: left shift on narrow integer type 'uint16_t'
uint32_t f_u16_warn(uint16_t x, uint32_t n) { return x << n; }

// CHECK: E41033
// CHECK: left shift on narrow integer type 'vector<uint8_t,4>'
vector<uint8_t, 4> f_u8x4_warn(vector<uint8_t, 4> x, uint32_t n) { return x << n; }

// The following should NOT produce E41033:
//   - uint8_t << uint8_t:                        same-width operands; result staying narrow is expected
//   - uint8_t << 4:                              literal shift; overflow handled separately by E41030
//   - uint8_t >> uint32_t:                       right shifts are not diagnosed by E41033
//   - vector<uint8_t,4> << uint8_t:              RHS not wider than element type
//   - vector<uint8_t,4> << vector<uint8_t,4>:    same element width on both sides
//   - matrix<uint8_t,2,2> << uint8_t:            RHS not wider than element type
//   - matrix<uint8_t,2,2> << matrix<uint8_t,2,2>: same element width on both sides
// CHECK-NOT: E41033
uint8_t f_same_width(uint8_t x, uint8_t n) { return x << n; }
uint8_t f_literal(uint8_t x) { return x << 4; }
uint8_t f_rsh(uint8_t x, uint32_t n) { return x >> n; }
vector<uint8_t, 4> f_u8x4_same_width(vector<uint8_t, 4> x, uint8_t n) { return x << n; }
vector<uint8_t, 4> f_u8x4_vec_rhs(vector<uint8_t, 4> x, vector<uint8_t, 4> n) { return x << n; }
matrix<uint8_t, 2, 2> f_u8x2x2_same_width(matrix<uint8_t, 2, 2> x, uint8_t n) { return x << n; }
matrix<uint8_t, 2, 2> f_u8x2x2_mat_rhs(matrix<uint8_t, 2, 2> x, matrix<uint8_t, 2, 2> n) { return x << n; }

[numthreads(1, 1, 1)]
void computeMain()
{
    buf[0] = set_byte32(buf[0], 1, uint8_t(123));
    buf[0] += f_u16_warn(uint16_t(buf[0]), 1);
}
