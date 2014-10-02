#!/bin/bash
exec fpc $1 -o$2 -Px86_64 >&2
