#!/bin/bash

find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's/iocoin/iocoin/g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's+I/OCoin+I/OCoin+g'
mv iocoin-qt.pro iocoin-qt.pro
