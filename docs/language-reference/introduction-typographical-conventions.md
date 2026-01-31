# Typographical Conventions

## Grammar

The Slang grammar in this document is presented using a variation of the
[Extended Backus–Naur form](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) as follows:


**Terminal symbol**
> **`'terminal'`** --- terminal symbol spelled exactly as is within quotes<br>
> **\<regex\>** --- terminal symbol expressed by a regular expression

[Terminal symbols](https://en.wikipedia.org/wiki/Terminal_and_nonterminal_symbols#Terminal_symbols) are those
that can appear in the language. In the Slang language reference manual, two forms are used: literal strings that
appear exactly as defined and symbols defined by regular expressions. Whitespace characters between terminal symbols
are generally meaningless and ignored except where explicitly stated.


**Non-terminal symbol**
> *`nonterminal`*

[Non-terminal symbols](https://en.wikipedia.org/wiki/Terminal_and_nonterminal_symbols#Nonterminal_symbols) do
not appear in the language. They are used in defining production rules.


**Production Rule**

> *`lhs`* = \<expr\>

A production rule defines how the left-hand-side non-terminal *`lhs`* may be substituted with the
right-hand-side grammatical expression \<expr\>. The expression consists of non-terminals and terminals using
the grammar expression building blocks described below.

Sometimes more than one production rule definitions are provided for the left-hand-side non-terminal. This
means that the non-terminal may be substituted with any of the definitions, which is grammatically equivalent
to alternation. Multiple production rules are used to associate semantics for individual production rules.

In case more multiple production rules may be successfully used, the semantics of the rule introduced earlier
apply, unless explicit precedence (*i.e.*, priority) is provided.

Note that a production rule definition is not explicitly terminated with a semi-colon (;).


**Concatenation**

> *`lhs`* = *`symbol1`* *`symbol2`* ... *`symbolN`*

[Concatenation](https://en.wikipedia.org/wiki/Concatenation) expresses a sequence of symbols, and it is
expressed without a comma. Symbols may be terminal or non-terminal.


**Alternation**

> *`lhs`* = *`alternative1`* \| *`alternative2`*

[Alternation](https://en.wikipedia.org/wiki/Alternation_(formal_language_theory)) expresses alternative
productions. That is, one (and exactly one) alternative is used.


**Grouping**
> *`lhs`* = ( *`subexpr`* )

Grouping is used to denote the order of production.


**Optional**
> *`lhs`* = [ *`subexpr`* ]

An optional subexpression may occur zero or one times in the production.


**Repetition**
> *`lhs`* = *`expr`*\* &nbsp;&nbsp;&nbsp;&nbsp; --- 0 or more times repetition<br>
> *`lhs`* = *`expr`*+  &nbsp;&nbsp;&nbsp;&nbsp; --- 1 or more times repetition

A repeated expression occurs any number of times (* -repetition); or one or more times (+ -repetition).

(*`expr`*+) is equivalent to (*`expr`* *`expr`*\*).


**Precedence**

The following precedence list is used in the production rule expressions:

|**Precedence**| **Grammar expressions**    | **Description**
|--------------|:---------------------------|:---------------------------------
|Highest       | ( ... ) [ ... ]            | grouping, optional
|              | \* \+                      | repetition
|              | *`symbol1`* *`symbol2`*    | concatenation (left-associative)
|              | *`symbol1`* \| *`symbol2`* | alternation (left-associative)
|Lowest        | =                          | production rule definition

For example, the following production rule definitions are equivalent:
> *`lhs`* = `expr1` `expr2`+ \| `expr3` [ `expr4` `expr5` ] `expr6` <br>
>
> *`lhs`* = (`expr1` `expr2`+) \| ((`expr3` [ (`expr4` `expr5`) ]) `expr6`) <br>


## Code Examples

Code examples are presented as follows.

**Example:**
```hlsl
struct ExampleStruct
{
    int a, b;
}
```

## Remarks and Warnings

Remarks provide supplemental information such as recommendations, background information, rationale, and
clarifications. Remarks are non-normative.

> 📝 **Remark:** Remarks provide useful information.

Warnings provide important information that the user should be aware of. For example, warnings call out
experimental features that are subject to change and internal language features that should not be used in
user code.

> ⚠️ **Warning:** Avoid using Slang internal language features. These exist to support Slang internal modules
> such as `hlsl.meta.slang`. Internal features are generally undocumented. They are subject to change without
> notice, and they may have caveats or otherwise not work as expected.
