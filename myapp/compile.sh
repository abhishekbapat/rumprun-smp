#!/bin/sh

export PATH="${PATH}:$(pwd)/../rumprun/bin"

(cd src && x86_64-rumprun-netbsd-gcc -Wall -O2 -o ../myapp main.c -DPORTMAP -DRUMPRUN)

cp ../rumprun/rumprun-x86_64/lib/libutil.a ../rumprun/rumprun-x86_64/lib/rumprun-hw/
cp ../rumprun/rumprun-x86_64/lib/libprop.a ../rumprun/rumprun-x86_64/lib/rumprun-hw/

x86_64-rumprun-netbsd-cookfs -s 1 rootfs.fs rootfs

rumprun-bake -m "add rootfs.fs" hw_generic myapp.bin myapp

sudo ./build_iso.sh
