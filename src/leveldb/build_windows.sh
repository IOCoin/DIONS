#!/bin/bash

TARGET_OS=NATIVE_WINDOWS make clean
make clean
TARGET_OS=NATIVE_WINDOWS make libleveldb.a libmemenv.a
