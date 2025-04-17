#!/bin/bash

# TODO: We should get the latest release file name with REST API
ctsFilename=VK-GL-CTS_WithSlang-0.0.7-win64

packageURL="https://github.com/shader-slang/VK-GL-CTS/releases/download/$ctsFilename/$ctsFilename.zip"
passingList="https://raw.githubusercontent.com/shader-slang/VK-GL-CTS/main/test-lists/slang-passing-tests.txt"
waiverList="https://raw.githubusercontent.com/shader-slang/VK-GL-CTS/main/test-lists/slang-waiver-tests.xml"
binDir=build/Release/bin

if [ ! -d "$binDir" ]
then
        echo "[$(date)] Build slang executables first in: $binDir"
        exit 1
fi

for url in "$packageURL" "$passingList" "$waiverList"
do
        echo "[$(date)] Downloading: $url ..."
        if ! curl -s -L -O "$url"
        then
                echo "File download failed: $url"
                exit 1
        fi
done

echo "[$(date)] Unzip $ctsFilename.zip in $binDir ..."
if ! unzip -q "$ctsFilename.zip" -d "$binDir"
then
        echo "Failed to unzip the downloaded file: $ctsFilename"
        exit 1
fi

cp slang-passing-tests.txt slang-waiver-tests.xml "$binDir/"

echo "[$(date)] Use the following command-line arguments to execute deqp-vk"
echo "cd $binDir; ./deqp-vk.exe --deqp-case=[single test name] --deqp-waiver-file=slang-waiver-tests.xml"
echo "cd $binDir; ./deqp-vk.exe --deqp-caselist-file=slang-passing-tests.txt --deqp-waiver-file=slang-waiver-tests.xml"

