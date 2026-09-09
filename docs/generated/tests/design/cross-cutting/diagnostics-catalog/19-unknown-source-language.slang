//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b456186770fb289a479f7bc709b43586b7a982121fba663aa76425f2b976db44
//META: purpose=Fires diagnostic E00019 (unknown-source-language) - unknown source language '~language'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=19
//META: catalog_name=unknown-source-language
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-lang` selects the source language for the following inputs. A name outside
// the known set is rejected during option parsing; the CHECK pins that code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-lang bogus
[numthreads(1,1,1)]
void main() {}

//CHECK: E00019
