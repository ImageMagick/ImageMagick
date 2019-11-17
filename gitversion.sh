#!/bin/sh

c=$(git log --full-history --format=tformat:. HEAD | wc -l)
h=$(git rev-parse --short HEAD)
d=$(date +%Y%m%d)

printf %s "$c:$h:$d"
