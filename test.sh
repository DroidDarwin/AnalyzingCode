#!/bin/bash



#### Convert the date format to one which we can sort
convert_date () {

	day=${2//,/""}
    local months=( January February March April May June July August September October November December )
    local i
    for (( i=0; i<11; i++ )); do
        [[ $1 = ${months[$i]} ]] && break
    done
    #printf "%4d%02d%02d\n" $3 $(( i+1 )) $2
    printf "%4d%02d%02d\n" $3 $(( i+1 )) $day
}




date="June 5, 2014"
date2="March 17, 2014"

#echo $date


#d=$( convert_date 27 JUN 2011 )
d=$( convert_date $date )
echo $d

d=$( convert_date $date2 )
echo $d


#### Split into 3 Groups  Month, day, Year

### Run if statements


### Break up into dates
###		Make sure we ill be able to sort by these dates










#echo test
