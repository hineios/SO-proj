#!/bin/bash

rm res/*
cd perf-dir
for i in flush*
do 
echo "started reading file $i "
cp ./../testeII/$i ./../res/$i
echo "stoped reading file $i "
done


