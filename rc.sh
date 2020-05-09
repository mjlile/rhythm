#!/bin/bash
if [ -z "$1" ]
then
    echo "usage: $0 filename"
    exit 1
fi

./rhythmc < $1 2> /dev/null | lli; echo $?
