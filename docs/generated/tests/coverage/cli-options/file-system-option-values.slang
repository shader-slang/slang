//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -file-system accepts each of default, load-file and os and compiles unchanged through all three, while an unrecognized value is rejected with E00062 listing exactly those three valid values.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::FileSystem` arm of `OptionsParser::_parse`
// in slang-options.cpp. `_expectValue` maps the operand onto
// `TypeTextUtil::FileSystemType`, and the arm then installs one of three file
// systems on the compile request: `nullptr` for `default`,
// `OSFileSystem::getLoadSingleton()` for `load-file`, and
// `OSFileSystem::getExtSingleton()` for `os`.
//
// All three hooks resolve the same on-disk source, so the positive runs pin
// "the same shader still compiles through this hook" rather than a delta — that
// is the only observable the option offers from the CLI, since the hook choice
// only changes which `ISlangFileSystem` interface the request calls into.
//
// The distinguishing observable is the rejection message, which enumerates the
// accepted value set and so would change if a `FileSystemType` were added or
// renamed; BADVALUE pins it verbatim.

//TEST:SIMPLE(filecheck=DEFAULTFS):-target hlsl -entry main -stage compute -file-system default
//TEST:SIMPLE(filecheck=LOADFILE):-target hlsl -entry main -stage compute -file-system load-file
//TEST:SIMPLE(filecheck=OSFS):-target hlsl -entry main -stage compute -file-system os
//TEST:SIMPLE(filecheck=BADVALUE):-target hlsl -entry main -stage compute -file-system bogus

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//DEFAULTFS: void main(

//LOADFILE: void main(

//OSFS: void main(

//BADVALUE: error[E00062]: unknown value for option '-file-system'. Valid values are 'default, load-file, os'
