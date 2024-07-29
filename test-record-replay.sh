#!/usr/bin/env bash

RED='\033[0;31m'
Green='\033[0;32m'
NC='\033[0m'
matchPattern="entrypoint: [0-9]+, target: [0-9]+, hash: [0-9a-fA-F]+"

getHash()
{
    matchedLine=$1
    local -n outputVar=$2

    entrypointIdx=$(echo $matchedLine | grep -oE "entrypoint: [0-9]+" | grep -oE "[0-9]+")
    targetIdx=$(echo $matchedLine | grep -oE "target: [0-9]+" | grep -oE "[0-9]+")
    hashCode=$(echo $matchedLine | grep -oE "hash: .*" | grep -oE ": [0-9a-fA-F]+" | grep -oE "[0-9a-fA-F]+")

    outputVar="$entrypointIdx-$targetIdx-$hashCode"
}

log()
{
    msg=$1
    color=$2
    printf "${color}$1${NC}\n"
}

parseStandardOutput()
{
    local -n resultArray=$1
    lines=$2

    for line in "${lines[@]}"
    do
        matchLine=$(echo $line | grep -oE "$matchPattern")

        if [ -n "$matchLine" ]; then
            result=""
            getHash "$matchLine" result

            if [ -n "$result" ]; then
                resultArray+=("$result")
            fi
        fi
    done
}

resultCheck()
{
    local -n inExpectedResults=$1
    local -n inReplayResults=$2
    local -n outFailedResults=$3

    found=""
    for expectedResult in ${inExpectedResults[@]}; do

        for replayResult in ${inReplayResults[@]}; do
            if [ "$replayResult" == "$expectedResult" ]; then
                found="1"
            fi
        done

        if [ -z "$found" ]; then
            echo  "$expectedResult is not Found in replay"
            outFailedResults+=("$expectedResult")
        else
            echo "$expectedResult is Found in replay"
        fi
    done
}

# TODO: Add more test commands here in this array
testCommands=("./build/Debug/bin/hello-world")

# Enable hash code generation for the test such that
# we can have something to compare with replaying the test
argsToEnableHashCode="--print-entrypoint-hashes"

declare -A testStats

for ((i = 0; i < ${#testCommands[@]}; i++))
do
    testCommand=${testCommands[$i]}
    echo "Start running test: $testCommand"

    # Run the test executable
    export SLANG_RECORD_LAYER=1
    mapfile -t lines < <(${testCommand} ${argsToEnableHashCode})
    unset SLANG_RECORD_LAYER

    # parse the output from stdout
    expectedResults=()
    parseStandardOutput  expectedResults "${lines[@]}"

    if [ ${#expectedResults[@]} -eq 0 ]; then
        log "No expected results found" $RED
        rm -rf ./slang-record/*
        continue
    fi

    # Replay the record file
    export SLANG_RECORD_LOG_LEVEL=3
    replayTestCommand="./build/Debug/bin/slang-replay ./slang-record/*.cap"
    echo "Start replaying the test ..."
    mapfile -t lines < <(${replayTestCommand})
    unset SLANG_RECORD_LOG_LEVEL

    # parse the output from stdout
    replayResults=()
    parseStandardOutput replayResults "${lines[@]}"

    echo "Replay Results: ${replayResults[@]}"
    if [ ${#replayResults[@]} -eq 0 ]; then
        log "No replay results found" $RED
        rm -rf ./slang-record/*
        continue
    fi

    # Check the results
    failedResults=()
    resultCheck expectedResults replayResults failedResults

    rm -rf ./slang-record/*

    if [ ${#failedResults[@]} -eq 0 ]; then
        testStats[$testCommand]="PASSED"
    else
        testStats[$testCommand]="FAILED"
    fi
done

for testName in "${!testStats[@]}"
do
  if [ "${testStats[$testName]}" == "PASSED" ]; then
      log "$testName: PASSED" $Green
  else
      log "$testName: FAILED" $RED
  fi
done

# Notify the CI if any of the tests failed
if [ ${#testStats[@]} -eq 0 ]; then
    exit 0
else
    exit 1
fi
