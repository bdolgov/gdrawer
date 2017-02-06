#!/bin/bash
exec gcc -shared -fPIC $1 -o $2 -O2 >&2
