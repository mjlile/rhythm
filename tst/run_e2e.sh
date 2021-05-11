#!/bin/bash

find tst/e2e -type f -name "*.rh" | while read fname; do
    # get files
    test_fname="${fname%.*}"
    echo "$test_fname"
    test_input_fname="${test_fname}.in"
    test_output_fname="${test_fname}.out"

    # run test
    ./rhythmc.sh $fname -o $test_fname
    if [ -f $test_input_fname ]; then
        test_actual_output=$($test_fname < $test_input_fname)
    else
        test_actual_output=$($test_fname)
    fi

    # compare output
    diff $test_output_fname <(echo "$test_actual_output")

    # cleanup executable
    rm $test_fname
done
