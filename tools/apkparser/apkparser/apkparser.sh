#!/bin/bash 

### Description: This runs the apk parser application on the apk file. 


logLocation=logs/apkparser.log

### Remove the log if it is there
rm -f $logLocation

## Create the log
touch $logLocation

echo "APKParser Start:" `date` >> $logLocation

date1=$(date +"%s")


DBLocation=EvolutionOfAndroidApplications.sqlite


### For each of the apk files, loop through them to get the XML information
count=0 ### Clear the counter
FILES=$(find $1 -type f -name '*.apk')
	for f in $FILES
	do
#		echo $f
#		tempResults=`java -jar tools/apkparser/APKParser.jar $f`


java -jar tools/apkparser/APKParser.jar $f > $count.txt


	#firstline=$(head -n 1 $tempResults)
	#nofront=${firstline#<?xml version=}
	#versionNumber=`echo $nofront| cut -d'"' -f 2`
	echo "version number = " $versionNumber


		echo "Analyze:" `echo $f` `echo $i` `date` >> $logLocation

count=$((count+1))
	done




echo "End APKParser Analyzing:" `echo $i` `date` >> $logLocation

date2=$(date +"%s")
diff=$(($date2-$date1))


echo "APKParser Total Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logLocation
echo "APKParser End:" `date` >> $logLocation
