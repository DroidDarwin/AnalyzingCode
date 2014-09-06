#!/bin/bash

	### Description: This assists with the analysis of the malware apps.
	#		Will probably only need to be ran one time


	dbname=malcode/MalwareInfo.sqlite

	#### Directory of APK files to be analyzed
	apkInputDir=scraper/downloads/full


	### Location of all downloaded zip files
	zipInputDir=malcode/zipFiles/

	### Directory where expanded apks will go
	mkdir -p $zipInputDir/tempOutput


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


	clearDatabase ### Clear the database if necessary 
	apkCounter=0 ## This will be the name of all apk files that are created


	### Remove all spaces in files
	### Not doing this may create problems with copying files over
	find $zipInputDir -depth -name "* *" -execdir rename 's/ /_/g' "{}" \;

	### Unzip all of the apk files and place the output into the analysis directory
	FILES=$(find $zipInputDir -type f -name '*.zip')
	for f in $FILES
	do
		echo unzip $f 
		unzip -P infected $f -d $zipInputDir/tempOutput
	done

	## Temp Counter for the file Info to make it unique
	counter=1

	### Loop through the temp output directory and copy all files to input
	FILES=$(find $zipInputDir/tempOutput  -type f -name '*.apk')
	for f in $FILES
	do
		echo Adding $f
		sqlite3 $dbname  "Insert into apkinformation (Name, SourceID, apkID, Developer, DatePublished) values (\"$counter\",1, \"$counter\", \"$counter\", \"$counter\");"	

		mv $f $apkInputDir/$counter.apk
		counter=$((counter+1))
	done

	## Delete contents of temp output
	rm -rf $zipInputDir/tempOutput/*

	## temp
	#rm -rf $apkInputDir/*
