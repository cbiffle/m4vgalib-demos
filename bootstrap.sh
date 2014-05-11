#!/bin/sh

set -e

git submodule update --init --recursive

if [ -e build ]; then
  echo "can't create build/ (already exists)"
  echo "remove it to proceed"
  exit 1
fi

echo "initializing Cobble buildroot in build/"
mkdir build
cd build
../cobble/wrapper.py init ..
cd ..

echo "Done."
