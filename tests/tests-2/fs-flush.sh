#!/bin/bash

echo "Make sure that CACHE SIZE is 30"

rm res/*
cd  fs-flush-dir
for i in flush*
do 
echo " $i "
cp $i ../testeII/$i
done
cd ..
cp testeII/flush3.txt res/flush3read.txt
cp testeII/flush1.txt res/flush1read.txt

