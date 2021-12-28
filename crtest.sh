#!/bin/zsh

CR="cr"

num_dir=("1" "2" "3")

num_file_mpl1=("11p" "11pp" "12" "18" "19p")
num_file_mpl2=("21" "22" "23" "023" "25t" "29p")
num_file_mpl3=("31p" "032p" "33p" "34" "35")

for i in ${num_file_mpl1[@]}
do
    ./${CR} ../mppl/mpl1/sample$i.mpl > ../out/${CR}/s$i.txt
done

for i in ${num_file_mpl2[@]}
do
    ./${CR} ../mppl/mpl2/sample$i.mpl > ../out/${CR}/s$i.txt
done

for i in ${num_file_mpl3[@]}
do
    ./${CR} ../mppl/mpl3/sample$i.mpl > ../out/${CR}/s$i.txt
done