#!/bin/sh

export PATH="${PATH}:$(pwd)/../rumprun-uefi_support/bin"

rm rumprun-myapp.bin.iso

rumprun iso myapp.bin
