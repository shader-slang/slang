//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Stress probe of conditional nesting: five levels of `#if`/`#endif` track correctly and the innermost active branch wins.
//META: intent=stress
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Five-level nested `#if` (the encouraged control-flow stress
// depth from the _common.md axes). All five conditions are true,
// so the innermost branch must be picked.

//TEST:INTERPRET(filecheck=CHECK):

#define L1 1
#define L2 1
#define L3 1
#define L4 1
#define L5 1

#if L1
#  if L2
#    if L3
#      if L4
#        if L5
static const int picked = 55;
#        else
static const int picked = 5;
#        endif
#      else
static const int picked = 4;
#      endif
#    else
static const int picked = 3;
#    endif
#  else
static const int picked = 2;
#  endif
#else
static const int picked = 1;
#endif

void main()
{
    // CHECK: picked=55
    printf("picked=%d\n", picked);
}
