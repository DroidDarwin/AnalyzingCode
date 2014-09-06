#!/bin/bash


### Todo:
## Refactor out to be a function where you can pass in an apk file and it will return the desired information
##




inputFile=2.txt





#./temp2.sh $inputFile account

./temp2.sh $inputFile "application android"


#sed -n 's/^\s`*`&lt;myel\s`*`name="\([^"]`*`\)".`*`$/\1,/p' $inputFile


#cat $inputFile | tr -s "\"" " " | awk '{printf "%s,\n", $3}'

#awk -F "[<>]" '/<manifest>|<id>/ {print $3}' $inputFile

#awk -F "[<>]" '/<manifest>|<android:versionCode>/ {print $2 " " $3}' $inputFile

#cat $inputFile



#for xml in `find . -name "*.xml"`
#do  
#echo $xml `xmllint --xpath "/param-value/value/text()" $inputFile`| awk 'NF>1'
#done


#### Get 


## Android versionCode
## Android versionName
## Application Label : Needed?
## Needed permissions
##		Use a seperate table for this?
## MinSDK version 



