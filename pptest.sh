#!/bin/zsh

num_mpls=("21" "021" "22" "022" "23" "023" "24" "024" "25" "025" "25t" "26" "026" "27" "28p" "29p" "2a" "02a")

for i in ${num_mpls[@]}
do 
    ./pp ../mpl2/sample$i.mpl > ../out/pp/s$i.txt
done