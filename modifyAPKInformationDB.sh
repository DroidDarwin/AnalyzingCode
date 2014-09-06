#!/bin/bash

### Description: This will modify or add informtion to the SQLite APKInformation Table

	#### Convert the date format to one which we can sort on
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

	echo "Start Modifying DB information"

	#### Directory of APK files to be analyzed
	#	apkInputDir=scraper/downloads/full
	apkInputDir=$1



	### Name of the sqlite file being analyzed
	dbname=EvolutionOfAndroidApplications.sqlite


	date1=$(date +"%s") ## Start date of the script
	logLocation=logs/modifyAPKInfo.log
	touch $logLocation
	echo "ModifyAPKInfo Start:" `date` >> $logLocation
	echo "Start Loop APK Files:" `date` >> $logLocation

	#### Loop through all of the apk files in the directory
	FILES=$(find $apkInputDir -type f -name '*.apk')
		for f in $FILES
		do

		fileName=$(basename $f)
		apkID=${fileName//.apk/""} ### Remove the apk exension from the apkID
		rowid=`sqlite3 $dbname "SELECT rowid FROM ApkInformation WHERE ApkId='$apkID';"`
		
		### Get the size of the apk file. The extra command has to be used to remove the filename
		#set -- $(du $f) ## Old version, only reported ~1/2 the size of the file
		set -- $(du -sb $f)
		fileSize=$1
		sqlite3 $dbname  "UPDATE ApkInformation SET GeneratedFileSize=$fileSize WHERE rowid=$rowid;"	

	done

	echo "End Loop APKFiles:" `date` >> $logLocation


	echo "Start Loop Through Database:" `date` >> $logLocation
	


	### Now loop through all of the records in the database to make sure those values are up to date
	###			Not the most efficient way to do things, but the best way I can think of 
	### Only loop through the information which needs to be updated
	LIST=`sqlite3 $dbname "SELECT rowID FROM apkinformation where ModifiedOSText is null"`;
	 
	# For each row
	for ROW in $LIST; do
		rowid=`echo $ROW | awk '{split($0,a,"|"); print a[1]}'`

		### Modify the minimum operating system
		# If the minimum OS is 1.3 and up, the column "ModifiedOS" should read 1.3
		minos=`sqlite3 $dbname "SELECT OperatingSystems FROM ApkInformation WHERE rowid='$rowid';"`
		minos=${minos//and up/""} ### Remove the and up	
		sqlite3 $dbname  "UPDATE ApkInformation SET ModifiedOSText=\"$minos\" WHERE rowid=$rowid;"	

		#### Generate the modified PublicationDate
		DatePublished=`sqlite3 $dbname "SELECT DatePublished FROM ApkInformation WHERE rowid='$rowid';"`
		
		### Convert to modified date published
		d=$( convert_date $DatePublished )
		sqlite3 $dbname  "UPDATE ApkInformation SET modifiedDatePublished=$d WHERE rowid=$rowid;"

	done

	echo "End Loop Through Database:" `date` >> $logLocation

	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "End:" `date` >> $logLocation
	echo "Total Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logLocation
