Rumprun SMP
==========

This repository contains a fork of [rumprun](https://github.com/rumpkernel/rumprun). This version of rumprun includes SMP support for the _hw_ platform
of rumprun. The SMP support is provided by Ruslan Nikolaev, Virginia Tech.
This version also includes minimalistic Xen HVM support by Ruslan Nikolaev.
Below is the README file that was provided with the original version of
rumprun by Antti Kantee (with changes for Xen HVM).

This repository uses [rump kernels](http://rumpkernel.org) to provide
the Rumprun [unikernel](https://en.wikipedia.org/wiki/Unikernel).
Rumprun works on not only on hypervisors such as KVM and Xen, but also on
bare metal.  Rumprun can be used with or without a POSIX'y interface.
The former allows existing, unmodified POSIX applications to run
out-of-the-box, while the latter allows building highly customized
solutions with minimal footprints.

The Rumprun unikernel supports applications written in, for example
but not limited to: _C_, _C++_, _Erlang_, _Go_, _Java_, _Javascript (node.js)_,
_Python_, _Ruby_ and _Rust_.

You will find ready-made software packages for Rumprun from the
[rumprun-packages repository](http://repo.rumpkernel.org/rumprun-packages).
Some examples of software available from there includes _LevelDB_,
_Memcached_, _nanomsg_, _Nginx_ and _Redis_.  See the packages repository
for further details.

See the [wiki](http://wiki.rumpkernel.org/Repo:-rumprun) for more
information and instructions.  You may also want to watch video
tutorials in the
[Rumprun unikernel video series](http://wiki.rumpkernel.org/Tutorial%3A-Rumprun-unikernel-video-series).

Note: some of our tools will throw a warning about them
being experimental.  It does not mean that they
are not expected to produce a working result, just that the usage
is not necessarily final.  The wiki
[explains](http://wiki.rumpkernel.org/Repo%3A-rumprun#experimental-nature)
further.

**GCC 8, 9, or higher**: newer versions of gcc may produce additional warnings
which are treated as errors; you can use a workaround by specifying
CC=/absolute/path/to/rumprun/gcc(8,9)fix.sh
(e.g., CC=~/rumprun-smp/gcc(8,9)fix.sh ./build-rr.sh hw).

hw
--

The hardware (``hw'') platform is meant for embedded systems
and the cloud.  It works on raw hardware, but also supports
_virtio_ drivers and KVM.  For a demonstration, see this [youtube
video](https://www.youtube.com/watch?v=EyeRplLMx4c) where the hw platform
is booted on a laptop and plays audio using the PCI hdaudio drivers.
The supported CPU architectures are x86_32, x86_64 and ARM.

Xen HVM
-------
Xen HVM technically belongs to the _hw_ platform. You need
to compile the code as _hw_ and bake an ISO image. Then you can boot
the ISO image in the HVM mode.

Xen PV
------

The Xen platform is optimized for running on top of the Xen hypervisor
as a paravirtualized guest, and provides support for virtualization
functions not available on the _hw_ platform.  The Xen platform will
work both against the `xl` tools and the Amazon EC2 cloud.
The supported CPU architectures are x86_32 and x86_64.
