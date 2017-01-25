#!/bin/bash

echo "Make sure that CACHE SIZE is 30"
cd perf-dir
for i in flush*
do 
cp $i ../testeII/$i
done
k="true"
i1=0
i2=19
START=$(date +%s)
echo "started at $START seconds"
while [ $k = "true" ];
do
echo "$k $i1 $i2"
cp flush0$i1.txt ../testeII/flush0$i1.txt
cp flush$i2.txt ../testeII/flush$i2.txt
i1=$(( $i1 + 1 ))
i2=$(( $i2 - 1 ))
if [ $i1 = 10 ]
then
  k="false"
fi
done
END=$(date +%s)
echo "ended at $END seconds"
DIFTIME=$(( $END - $START ))
echo "Processed during $DIFTIME seconds"


