#!/bin/bash

usage="usage: $0 src_filename [-o bin_filename]"

if [ -z "$1" ]
then
    echo $usage
    exit 1
fi

output=""

if [ ! -z "$2" ]
then
    if [[ "$2" != "-o" ]]; then
        echo $usage
        exit 1
    elif [[ -z "$3" ]]; then
        echo $usage
        exit 1
    fi

    output="-o $3"
fi

./rhythmc < $1 2> /dev/null | clang -x ir - -Wno-override-module $output
