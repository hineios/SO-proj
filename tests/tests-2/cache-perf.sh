#!/bin/bash

echo "here"
START=$(date +%s%N)
echo "started at $START nanoseconds"
ls -alt testeII > write.txt
ls -alt ../include >> write.txt
cp write.txt testeII/wfile.txt
cp testeII/wfile.txt read.txt
echo "start finding differences:"
diff write.txt read.txt
echo "finish of finding differences"
END=$(date +%s%N)
echo "ended at $END nanoseconds"
DIFTIME=$(( $END - $START ))
echo "Processed during $DIFTIME nanoseconds"
rm read.txt write.txt
