#!/usr/bin/env python3

import json
import sys

with open(sys.argv[1]) as file:
    vcpkg_content = json.load(file)
    for dependency in vcpkg_content["dependencies"]:
        print(dependency)
