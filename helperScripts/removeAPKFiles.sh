#!/bin/bash

	## Description: This will remove apk files from the download directory
	##		This is currently meant to be a standalone program that is run manually	

	### Todo:
	##		Add input parameters
	##		Create Logs


	apkLocation="scraper/downloads/full/";			# Location of all the apk files
	minDLCount=10;		# The minimum number of downloads to keep


	### Loop through all files in the .apk directory
	FILES=$(find $apkLocation -type f -name '*.apk')
	for f in $FILES
	do
		APKFile=$(basename $f)
		APKFile=${APKFile//.apk/""} ### Remove the apk exension from the apkID
		#echo $f
		dlcount=`sqlite3 DummyEvolutionOfAndroidApplications.sqlite  "SELECT lowerdownloads FROM ApkInformation WHERE ApkId='$APKFile';"`
	
		## If dlcount is not found, then just delete the file. This probably means no record for the DB exists, or that something has gone very wrong.
		#echo $rowid
		if [ -z "${dlcount}" ]; then
    		#echo "JAIL is unset or set to the empty string"
			echo Delete file $f		
#    		rm $f

    	else
    		### A record in the db exists for it, 
    		#echo $dlcount
    		#if [[ $dlcount<$minDLCount ]] then
    		if [ $minDLCount -gt $dlcount ]
    			then
    			echo Remove $f - $dlcount
    			rm $f
    		fi
		fi
	done

