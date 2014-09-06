#!/bin/bash

	### Description: This assists with the analysis of the malware apps.
	#		Will probably only need to be ran one time


	dbname=EvolutionOfAndroidApplications.sqlite

	#### Directory of APK files to be analyzed
	apkInputDir=scraper/downloads/full

	#### clear specific database table
	clearTable () {
		echo "Clear Table: " $1
		sqlite3 $dbname "delete from $1;"
	}	

	#### Function to clear out the database from uneeded data
	clearDatabase () {
		echo "Delete Database Info"
		clearTable toolresults
		clearTable ApkInformation
		clearTable Overprivilege
		clearTable Permissions
		clearTable ToolResults
		clearTable Underprivilege
		clearTable apkParser_intents
		clearTable apkParser_intents_join
		clearTable apkParser_privs
		clearTable apkParser_privs_join
	}

	### Remove all spaces in files
	### Not doing this may create problems with copying files over
	find $apkInputDir -depth -name "* *" -execdir rename 's/ /_/g' "{}" \;

	clearDatabase ### Clear the database if necessary 

	## Temp Counter for the file Info to make it unique
	counter=1

	### Loop through the temp output directory and copy all files to input
	FILES=$(find $apkInputDir  -type f -name '*.apk')
	for f in $FILES
	do
		#echo Adding $f

		
		appName=${f//.apk/""}
	appName=${appName//%/""}
	appName=$(basename $appName)
	echo $appName
		sqlite3 $dbname  "Insert into apkinformation (Name, SourceID, apkID, Developer, DatePublished) values (\"$appName\",1, \"$appName\", \"$counter\", \"$counter\");"	
		counter=$((counter+1))

		
	done


