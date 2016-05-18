#!/bin/sh

set -x
gcc -std=c99 -D_GNU_SOURCE count.c -lmlm -lczmq -o count && ./count
