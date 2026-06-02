//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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
