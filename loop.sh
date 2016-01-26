#!/bin/sh
count=0
while [ 1 ]; do
	count=$(( count + 1 ))
	echo foo $count
	sleep 1
done
