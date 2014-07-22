#!/bin/bash

find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's/iocoin/iocoin/g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's+I/OCoin+I/OCoin+g'
find src/qt/ -type f -iname "*.ui" -or -iname "*.ts" -or -name "bitcoinunits.cpp" -print0 | xargs -0 sed -i 's/BC/IO/g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's/iphU6HbZCuDsx1nGWncuwxYdHcF74zLv8U/iphU6HbZCuDsx1nGWncuwxYdHcF74zLv8U/g'
mv iocoin-qt.pro iocoin-qt.pro
