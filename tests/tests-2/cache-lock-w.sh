#!/bin/bash

dir=perf-dir
cd $dir
for i in flush*
do 
echo "started writing file $i "
cp $i ./../testeII/$i
echo "finished writing file $i "
done


