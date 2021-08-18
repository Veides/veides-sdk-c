#!/bin/bash

currentDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo `whoami`

echo $currentDir

cd $currentDir/..

cmake -DVEIDES_BUILD_TESTS=ON -DVEIDES_BUILD_DEPENDENCIES=ON -DVEIDES_INSTALL_DEPENDENCIES=OFF -DVEIDES_BUILD_SAMPLES=ON . && make -j3 && make test
