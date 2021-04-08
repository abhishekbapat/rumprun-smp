#!/bin/sh

export PATH="${PATH}:$(pwd)/../rumprun/bin"

rm rumprun-myapp.bin.iso

rumprun iso myapp.bin
