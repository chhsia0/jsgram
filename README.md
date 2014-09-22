jsgram
======

=== How to Build ===

Download and build the V8 JavaScript engine:

    make v8

It would build the required V8 libraries in *debug* mode.
(Debug mode is needed for the use of some internal V8 functions.)

Build the JS n-gram builder:

    make

=== How to Use ===

Print canonical JavaScript:

    jsgram -p <jsfile>

List all functions in canonical JavaScript:

    jsgram -l <jsfile>

List all n-grams in canonical JavaScript:

    jsgram [-n <n>] [-s] <jsfile>

    -n <n>: depth of n-gram
    -s: sequential n-gram
