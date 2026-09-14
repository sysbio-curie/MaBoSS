#!/bin/bash


if [ -z "${BUILD_PATH}" ]
then 
BUILD_PATH=/tmp/
fi

cd ${BUILD_PATH}/libsbml-5.21.1/build
make install