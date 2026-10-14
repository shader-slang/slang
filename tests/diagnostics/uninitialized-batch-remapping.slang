//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check 128 variables so the analysis needs more than one mask word. Assigning the
// first 127 or 63 variables on both branches leaves 1 or 65 needing condition tracking.
// Every read remains initialized after the surviving variables receive new bit positions.
struct Data { float value; };

#define FIRST_63(M) \
    M(0) M(1) M(2) M(3) M(4) M(5) M(6) M(7) \
    M(8) M(9) M(10) M(11) M(12) M(13) M(14) M(15) \
    M(16) M(17) M(18) M(19) M(20) M(21) M(22) M(23) \
    M(24) M(25) M(26) M(27) M(28) M(29) M(30) M(31) \
    M(32) M(33) M(34) M(35) M(36) M(37) M(38) M(39) \
    M(40) M(41) M(42) M(43) M(44) M(45) M(46) M(47) \
    M(48) M(49) M(50) M(51) M(52) M(53) M(54) M(55) \
    M(56) M(57) M(58) M(59) M(60) M(61) M(62)
#define NEXT_64(M) \
    M(63) M(64) M(65) M(66) M(67) M(68) M(69) M(70) \
    M(71) M(72) M(73) M(74) M(75) M(76) M(77) M(78) \
    M(79) M(80) M(81) M(82) M(83) M(84) M(85) M(86) \
    M(87) M(88) M(89) M(90) M(91) M(92) M(93) M(94) \
    M(95) M(96) M(97) M(98) M(99) M(100) M(101) M(102) \
    M(103) M(104) M(105) M(106) M(107) M(108) M(109) M(110) \
    M(111) M(112) M(113) M(114) M(115) M(116) M(117) M(118) \
    M(119) M(120) M(121) M(122) M(123) M(124) M(125) M(126)
#define FIRST_127(M) FIRST_63(M) NEXT_64(M)
#define LAST_65(M) NEXT_64(M) M(127)
#define LAST_1(M) M(127)
#define ALL(M) FIRST_127(M) LAST_1(M)
#define DECLARE(N) Data d##N;
#define STORE(N) d##N.value = 1;
#define READ(N) output[N] = d##N.value;

// Check that a repeated guard still prevents warnings after 127 of 128 variables
// are proven initialized without condition tracking.
void oneConditionallyStoredVariable(bool branchCondition, bool assigned, float* output)
{
    ALL(DECLARE)
    if (branchCondition)
    {
        FIRST_127(STORE)
    }
    else
    {
        FIRST_127(STORE)
    }
    if (assigned)
    {
        LAST_1(STORE)
    }
    FIRST_127(READ)
    if (assigned)
    {
        LAST_1(READ)
    }
}

// Check that repeated guards prevent warnings for 65 remaining variables
// whose initialization masks span two words.
void sixtyFiveConditionallyStoredVariables(bool branchCondition, bool assigned, float* output)
{
    ALL(DECLARE)
    if (branchCondition)
    {
        FIRST_63(STORE)
    }
    else
    {
        FIRST_63(STORE)
    }
    if (assigned)
    {
        LAST_65(STORE)
    }
    FIRST_63(READ)
    if (assigned)
    {
        LAST_65(READ)
    }
}

[shader("compute")]
void main() {}
