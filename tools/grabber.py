#!/usr/bin/env python3

import sys
import os

for filename in os.listdir(sys.argv[1]):
    if os.path.splitext(filename)[-1] == f".{sys.argv[2]}":
        basename = os.path.basename(filename)
        library_name = basename[3 : len(basename) - len(sys.argv[2]) - 1]
        print(library_name)
