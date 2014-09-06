#!/bin/bash

### Description: Will break the .apk input directory into groups to futher speed up analysis. 
###		This will likely only need to be run one time for the initial analysis

### Protocol: Run this script to break up the directories into groups. 
###		Move each group to be analyzed by an individual "thread" 


### Usage: ./groupSplit.sh <inputDir> , <outputDir> , <StartCopy>, <EndCopy>
###		Dan-macbook:testFileSplit dan$ ./groupSplit.sh allfiles g4 11 25

### Todo: 
###		Loop through the files but order them by name desc
###			It should work ok now, but it probably wouldn't be a bad idea to do this to be on the safe side.

# Check to make sure that an argument is actually passed in
EXPECTED_ARGS=4
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` {arg} {arg} {arg} {arg}"
  exit $E_BADARGS
fi

### Where the files are coming from
#inputDir=allfiles
inputDir=$1

### Where the files are going
#targetDir=g1
targetDir=$2

### Starting file to copy
#startFile=6
startFile=$3

### Last file number to copy over
#endFile=10
endFile=$4

echo Moving Files - Start: $startFile - End: $endFile

### Count the number of files that are copied
copyCount=0

###Total number of passes through the for loop. This iterates over all files
loopCount=1

### Loop through all the files in the input directory and copy them over
FILES=$(find $inputDir -type f -name '*.apk')
	for f in $FILES
	do

	### Check to see if the loopcount is between two numbers
	if (($startFile<=$loopCount && $loopCount<=$endFile)); 
	then
		echo $loopCount - Copy $f to: $targetDir

		### Copy the actual file
		cp $f $targetDir

		### Increment the number of files copied
		copyCount=$((copyCount+1))
	fi

	loopCount=$((loopCount+1))
	done


echo "Files Copied:" $copyCount
