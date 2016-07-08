#!/bin/bash

sudo apt-get update
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install -y \
  gcc-arm-embedded=5-2016q2-1~trusty1 \
  build-essential \
  git \
  ninja-build \
  openocd

echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", \
      ATTRS{idProduct}=="3748", MODE:="0666"' \
      | sudo tee /etc/udev/rules.d/50-stlink.rules

cd /vagrant
./bootstrap.sh
