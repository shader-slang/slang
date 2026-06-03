// Regression test for #11276 (Case 1): a long-digit floating-point literal
// would silently saturate to +INF. The previous accumulator in
// `getFloatingPointLiteralValue` did `value = value * radix + digit` for
// every digit; with hundreds of significant digits the running double would
// overflow `DBL_MAX` and stick at `+INF`, and the compensating `value /=
// divisor` then produced `INF / 10^N == INF`. The fix routes the literal
// through `std::from_chars`, which performs correctly-rounded IEEE 754
// conversion regardless of digit count and is locale-independent by spec
// (see `slang-emit-source-writer.cpp:234` for the symmetric output-side
// guarantee). Cross-ref the previously closed #332.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // ~1000-digit pi literal. The result must be finite (the closest
    // `double` to pi), not `+INF`. `int(pi * 100)` is `314` for the
    // correctly-rounded value and saturates to `INT_MAX` if the literal
    // came back as `+INF`.
    let pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273724587006606315588174881520920962829254091715364367892590360011330530548820466521384146951941511609433057270365759591953092186117381932611793105118548074462379962749567351885752724891227938183011949129833673362440656643086021394946395224737190702179860943702770539217176293176752384674818467669405132000568127145263560827785771342757789609173637178721468440901224953430146549585371050792279689258923542019956112129021960864034418159813629774771309960518707211349999998372978049951059731732816096318595024459455346908302642522308253344685035261931188171010003137838752886587533208381420617177669147303598253490428755468731159562863882353787593751957781857780532171226806613001927876611195909216420198;
    //CHECK: pi=314
    printf("pi=%d\n", int(pi * 100.0));

    // Hex-float with `f` suffix: `std::from_chars` parses the hex mantissa
    // and exponent, leaves the suffix `f` unread, and the parser reads the
    // suffix as the float-precision specifier. Uncovered anywhere else in
    // `tests/`.
    let hex = 0x1.4p+3f;
    //CHECK: hex=10
    printf("hex=%d\n", int(hex));

    // Spot-check that ordinary decimal/scientific literals still work.
    //CHECK: dec=314 sci=314
    printf("dec=%d sci=%d\n", int(3.14 * 100.0), int(0.0314 * 1e4));
}
