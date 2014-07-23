#!/bin/bash

find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's/iocoin/iocoin/g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's+I/OCoin+I/OCoin+g'
find src/qt/ -type f -iname "*.ui" -or -iname "*.ts" -or -name "bitcoinunits.cpp" -print0 | xargs -0 sed -i 's|BC|I/O|g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's/iphU6HbZCuDsx1nGWncuwxYdHcF74zLv8U/iphU6HbZCuDsx1nGWncuwxYdHcF74zLv8U/g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's|0.00 BC|0.00 I/O|g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's|0 BC|0 I/O|g'
find . -type f -not -path "./.git/*" -print0 | xargs -0 sed -i 's|123.456 BC|123.456 I/O|g'
find src/qt/locale/ -type f -print0 | xargs -0 sed -i 's|510|110|g'
mv blackcoin-qt.pro iocoin-qt.pro
