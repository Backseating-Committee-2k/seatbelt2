#!/bin/sh

for i in $1/*.$2; do
    echo $(basename $i | awk -v LL=$(echo $2 | wc -c) '{ print substr( $0, 4, length($0)-LL-3 ) }')
done
