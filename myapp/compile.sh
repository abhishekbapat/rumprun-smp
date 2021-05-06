#!/bin/sh

export PATH="${PATH}:$(pwd)/../rumprun-uefi_support/bin"

(cd src && x86_64-rumprun-netbsd-gcc -Wall -O2 -o ../myapp main.c -DPORTMAP -DRUMPRUN)

rumprun-bake hw_generic myapp.bin myapp

sudo ./build_iso.sh