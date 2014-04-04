#!/bin/sh -e

cd etl
git remote update
git checkout -B master origin/master
cd ..

git add etl
