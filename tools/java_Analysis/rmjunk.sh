#!/bin/bash 
### Remove all files not in the manifest file

echo "Start Remove Junk From: " $1


### File count not working
#startFileCount=`find . -type f -printf "%T@ %p\n" | sort -nr | cut -d\  -f2-`;

### Check to make sure the manifest file exists. This eliminates the possibility of deleting all files
if [ -f $1/manifest.sed ]; then
	#echo "File exists" 
	### Delete all of the created files
	rm -rf `find $1 -print | sed -f $1/manifest.sed`
else 
	echo "Manifest File Not Found"
	exit
fi

echo "End Remove Junk From: " $1
#endFileCount=`find . -type f -printf "%T@ %p\n" | sort -nr | cut -d\  -f2-`;
#echo "Files Removed:" `expr $startFileCount - $endFileCount`