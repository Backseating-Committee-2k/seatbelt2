#!/usr/bin/env python3

import sys
import os

lib_extensions = [sys.argv[i] for i in range(1, len(sys.argv))]
for filename in os.listdir(sys.argv[1]):
    if os.path.splitext(filename)[-1][1:] in lib_extensions:
        basename = os.path.basename(filename)
        library_name = (
            basename[3 : len(basename) - len(sys.argv[2]) - 1]
            if basename.startswith("lib")
            else basename[: len(basename) - len(sys.argv[2]) - 1]
        )
        print(library_name)
