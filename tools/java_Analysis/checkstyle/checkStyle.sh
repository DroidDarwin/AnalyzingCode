#!/bin/bash 
### Run checkstyle on all files
### A generic listing of warnings is output to the user
###		This list of evaluation criteria may be found at: all-checkstyle-checks.xml

logLocation=logs/checkstyle.log

### Remove the log if it is there
rm -f $logLocation

## Create the log
touch $logLocation

echo "checkstyle Start:" `date` >> $logLocation

date1=$(date +"%s")


### Location of the clones to be analyzed
### This is back up a level since when it is called, it is from the checkstyle directory
inputDirectory=../javaOutput/

### I do not like doing this, but I do not really know a way around it.
cd tools/java_Analysis/checkstyle/

### Loop through all files in the input directory
#for i in $(find $inputDirectory -mindepth 1 -type d ) # mindepth ignore the top layer
for i in $(find $inputDirectory -maxdepth 1 -mindepth 1 -type d ) # maxdepth to only start with the top level, but mindepth to ignore it.
do
	### This could be written cleaner
	appName=${i//apk/""}
	appName=${appName//%/""}
	appName=$(basename $appName)
		
	### Begin running checkstyle on each
	rm -f temp.txt
	touch temp.txt
	
	####java -jar checkstyle-5.7-all.jar -c all-checkstyle-checks.xml $inputDirectory/$i/*
	
	### Written to temp file since a variable is too long to parse afterwards
	temp="`java -jar checkstyle-5.7-all.jar -c all-checkstyle-checks.xml $inputDirectory/$i/* 2>&1 > temp.txt`"

	fileTarget=temp.txt
	defectCount=`grep -o '.java' $fileTarget | wc -l`


	date2=$(date +"%s")
	diff=$(($date2-$date1))

	touch temp.txt
	echo "$(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed." 

	rm -f temp.txt

	### These values can be used to put into the database
	echo FileName: $appName
	echo DefectCount: $defectCount

	cd ../../../  #moving to the directory with the database

	echo "Current Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds. for " `echo $appName` "Defect Count:" `echo $defectCount` >> $logLocation

	rowid=`sqlite3 EvolutionOfAndroidApplications.sqlite  "SELECT rowid FROM ApkInformation WHERE ApkId='$appName';"`
	sqlite3 EvolutionOfAndroidApplications.sqlite  "UPDATE ToolResults SET DefectCount=$defectCount WHERE ApkId=$rowid;"	
	
	cd tools/java_Analysis/checkstyle/

done

###TODO
# Clear out everything in the javaOutput directory but "placeholder.txt"

cd ../../../  #moving to the directory with the database
date2=$(date +"%s")
diff=$(($date2-$date1))
echo "CheckStyle Total Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logLocation
echo "CheckStyle End:" `date` >> $logLocation

