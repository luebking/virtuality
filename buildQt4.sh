#!/bin/sh
if [[ "$TERM" != *xterm* ]]; then
    if [ ! -z "`which konsole`" ]; then
        konsole -e "$0" "keep"
    else
        xterm -e "$0" "keep"
    fi
    exit
fi

mkdir -p .build4
cd .build4
if ! cmake -DWITH_QT5=OFF -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..; then
    echo "CONFIGURATION FAILED :-("
    exit
fi

echo "BUILDING..."

if ! make; then
    echo "BUILD FAILED :-("
    exit
fi

echo "... INSTALL:"

if sudo make install; then
    echo
    echo "... SUCCESS!"
fi

if [ "$1" = "keep" ]; then
    echo "======================================"
    echo "Press ENTER to quit ..."
    read foo
fi