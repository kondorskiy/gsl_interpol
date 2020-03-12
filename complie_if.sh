#!/bin/sh
g++ `pkg-config --cflags gsl` -o interp_funct.exe \
    interp_funct.cpp `pkg-config --libs gsl`;

