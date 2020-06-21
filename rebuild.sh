#!/bin/bash

WORKSPACE=$(cd "$(dirname "$0")";pwd)

FLAG_USE_3519=FALSE
FLAG_USE_3516=FALSE

BUILD_ARCH="hi3519av100"

function usage {
  echo "Usage: $0 [ -h | --help ] [ -t | --type ]"
  echo "  -h | --help      display this help"
  echo "  -t | --type      type of hisi board chip"
  exit 0
}

function get_type {
  type=$1
  echo ${type}
  if [ ${type} == "hi3519av100" -o ${type} == "3519av100" -o ${type} == "3519a" -o ${type} == "3519" ]; then
    BUILD_ARCH="hi3519av100"
    FLAG_USE_3519=TRUE
    FLAG_USE_3516=FALSE
  elif [ ${type} == "hi3516dv300" -o ${type} == "3516dv300" -o ${type} == "3516d" -o ${type} == "3516" ]; then
    BUILD_ARCH="hi3516dv300"
    FLAG_USE_3519=FALSE
    FLAG_USE_3516=TRUE
  else 
    usage
  fi
}

# get options
OPTS=`getopt -o ht: --long help,type: -- "$@"`
if [ $? != 0 ] ; then echo "Failed to parse options." >&2 ; exit 1 ; fi
if [ $# == 0 ]; then usage; fi
eval set -- "$OPTS"

while [ $# -gt 0 ]; do
  case "$1" in
    -h | --help ) usage; shift ;;
    -t | --type ) get_type $2; shift 2 ;;
    -- ) shift; break ;;
    * ) break ;;
  esac
done

# path
BUILD_PATH="${WORKSPACE}/build_${BUILD_ARCH}"
INSTALL_PATH="${WORKSPACE}/release_${BUILD_ARCH}"
THIRDPARTY_PATH="${WORKSPACE}/thirdparty"

# untar
cd ${THIRDPARTY_PATH}
ls *-${BUILD_ARCH}.tar.gz | xargs -n1 tar xzvf

# perform build process
if [ -d ${BUILD_PATH} ]; then
  rm -rf ${BUILD_PATH}
fi
if [ -d ${INSTALL_PATH} ]; then
  rm -rf ${INSTALL_PATH}
fi

mkdir -p ${BUILD_PATH}
cd ${BUILD_PATH}

cmake .. -DCMAKE_C_COMPILER=arm-himix200-linux-gcc \
         -DCMAKE_CXX_COMPILER=arm-himix200-linux-g++ \
         -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH} \
         -DUSE_3519=${FLAG_USE_3519} \
         -DUSE_3516=${FLAG_USE_3516}

make -j4
make install

cp -r "${WORKSPACE}/data/" "${INSTALL_PATH}/test"