#!/bin/bash

cd newbuild/linux &&
make clean &&
export arch=x86 &&
make &&
make modules &&
make modules_install &&
make install &&
update-grub2
