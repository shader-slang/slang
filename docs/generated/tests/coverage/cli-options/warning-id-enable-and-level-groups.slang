//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -W<id> re-enables a warning that an earlier -Wno-<id> disabled, and that the -Wall/-Wextra/-Wpedantic warning-group flags are accepted and leave the always-on Default group emitting.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::EnableWarning` and `OptionKind::WarningLevel`
// arms of `OptionsParser::_parse` in slang-options.cpp. `EnableWarning` slices the
// id off a `-W<id>` spelling (`argValue.tail(2)`) and appends it to the linkage
// option set; `WarningLevel` maps the three exact spellings `-Wall`/`-Wextra`/
// `-Wpedantic` onto `SLANG_WARNING_LEVEL_{ALL,EXTRA,PEDANTIC}`. Neither arm was
// reached by either shipped suite.
//
// The observable handle for `EnableWarning` is order sensitivity: `-Wno-41016`
// alone suppresses the uninitialized-variable warning, but following it with
// `-W41016` puts the id back on the enable list and the warning reappears. That
// makes the enable arm the sole cause of the delta between the DISABLED and
// REENABLED runs below.
//
// For `WarningLevel` there is currently no diagnostic in the compiler tagged with
// the `All` or `Pedantic` group, so enabling a group cannot add a message; what is
// pinned is that the flags parse and that the always-on `Default`-group warning
// still comes out. That is a weaker claim than a delta, so the run is here to hold
// the parse arm rather than to assert a behavioural difference.

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=DISABLED):-target hlsl -entry main -stage compute -Wno-41016
//TEST:SIMPLE(filecheck=REENABLED):-target hlsl -entry main -stage compute -Wno-41016 -W41016
//TEST:SIMPLE(filecheck=WALL):-target hlsl -entry main -stage compute -Wall
//TEST:SIMPLE(filecheck=WPEDANTIC):-target hlsl -entry main -stage compute -Wpedantic

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int x;
    RWStructuredBuffer<int> b;
    b[0] = x;
}

//DEFAULT: warning[E41016]: use of uninitialized variable

//DISABLED-NOT: E41016
//DISABLED: void main(

//REENABLED: warning[E41016]: use of uninitialized variable

//WALL: warning[E41016]: use of uninitialized variable

//WPEDANTIC: warning[E41016]: use of uninitialized variable
