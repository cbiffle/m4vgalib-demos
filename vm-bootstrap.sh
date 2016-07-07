#!/bin/bash

sudo apt-get update
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install -y \
  gcc-arm-embedded=5-2016q2-1~trusty1 \
  build-essential \
  git \
  ninja-build
