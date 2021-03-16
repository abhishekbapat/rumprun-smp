#!/bin/sh

export PATH="${PATH}:$(pwd)/../rumprun/bin"
rumprun kvm -i -g '-nographic -vga none' -M 1024 myapp.bin
