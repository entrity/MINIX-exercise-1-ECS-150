#!/bin/sh
count=0
while [ $count -lt 8 ]; do
	count=$(( count + 1 ))
	echo foo $count
	sleep 1
done
