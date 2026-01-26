> Note: This document is a work in progress. It is both incomplete and, in many cases, inaccurate.

# Introduction

Slang is a programming language primarily designed for use in *shader programming*, by which we mean
performance oriented GPU programming for real-time graphics.


## General Topics

* [Language Goals (TODO)](introduction-goals.md)
* [Typographical Conventions](introduction-typographical-conventions.md)


## Purpose of this document

This document aims to provide a detailed reference for the Slang language and its supported constructs.

The Slang compiler *implementation* may deviate from the language as documented here in a few key ways:

* The implementation is necessarily imperfect and can have bugs.

* The implementation may not fully support constructs documented here, or their capabilities may not be as
  complete as what is documented.

* The implementation may support certain constructs that are not properly documented. Constructs that are:
  - *deprecated* --- These are called out with a ⚠️ **Warning**. Other documentation may be removed to
    discourage use.
  - *experimental* --- These are called out with a ⚠️ **Warning**. The constructs are subject to change and the
    documentation may not be yet up to date.
  - *internal* --- These are called out with a ⚠️ **Warning**. The constructs are often not otherwise
    documented to discourage use.

Where possible, this document calls out known deviations between the language as defined here and the
implementation in the compiler, often including github issue links.


## Terminology

> Note: This section is not yet complete.
>
> This section should detail how the document uses terms like "may" and "must," if we intend for those to be used in a manner consistent with [RFC 2119](https://www.ietf.org/rfc/rfc2119.txt).
