//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// CHECK: ([[# @LINE+1]]): error 104: 'neural' is an experimental module, need to enable'-experimental-feature' to load this module
import neural;
