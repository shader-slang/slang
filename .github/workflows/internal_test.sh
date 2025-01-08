#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <PR id>"
  exit 1
fi

echo $PWD
export PATH=$PATH:$PWD

echo $PATH

JQ=$PWD/jq.exe

PR=$1

