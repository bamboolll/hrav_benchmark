#!/bin/bash
out_file=$1

while read line; do 
echo " "
echo "**************************************"
echo "Test with buffer size ${line};"
./send_file -i nf1 -s ~/tmp/test.file2 -b ${line} > t.tmp
speed=`cat t.tmp | grep "^Speed" | sed 's/Speed//g'`
echo "$speed"
echo "${line} ${speed}" >> ${out_file}

done < buffer_size.input

echo "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
echo "FINISH TEST"