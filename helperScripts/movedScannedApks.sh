#!/bin/bash

	### Description: 
	##		Checks to see if the apk file has already been scanned by the database. If so, then place the 
	##		This is just for temporary use. If this is a problem long term, then this logic should be placed into the analyzing code.


	### Usuage: ./helperScripts/movedScannedApks.sh

	### Database Location
	db=EvolutionOfAndroidApplications.sqlite

	### Starting APK directory
	StartDir=scraper/downloads/full/

	### Temp APK Directory - Holds already scanned apks
	AlreadyScannedDir=scraper/downloads/temp_Already_Scanned/

	## Create output directory if it does not exist
	mkdir -p $AlreadyScannedDir

	### Loop through all of the apk files
	FILES=$(find $StartDir -type f -name '*.apk')
	for f in $FILES
	do
	    fileName=$(basename $f)
	    fileName=${fileName//.apk/""} ### Remove the apk exension from the apkID 
	  	
	    ### If they are found in the DB, then move the file.
	    #APKToolResultsCount=`sqlite3 $db  "SELECT count(*) FROM ToolResults WHERE rowid='$rowid';"`
		APKToolResultsCount=`sqlite3 $db  "select count(ai.apkID) from toolresults tr inner join apkinformation ai on tr.rowID=ai.rowid where ai.apkID = '$fileName';"`

  		if [[ $APKToolResultsCount -gt 0 ]]; then
			echo Move $f
			mv $f $AlreadyScannedDir$fileName.apk
       	fi

  	done

  	### State how many files stayed were moved
  	echo "******* Files Now in AlreadyScanned:" `ls -l $AlreadyScannedDir | wc -l `

  	### State how many files stayed
  	echo "******* Files Now in Full:" `ls -l $StartDir | wc -l` 

	echo "Script Completed"
