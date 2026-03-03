//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Tests that we will diagnose an error on struct inheritance in language 2026 or later.

#lang 2026

struct Base {}

//CHECK: ([[# @LINE+1]]): error 30811:
struct Derived : Base {}
