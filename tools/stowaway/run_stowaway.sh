#!/bin/bash

### Description: Invokes the stowaway application on the target apk files 
###			and then records the over and underpriviledges into the application database.


# Overprivilege
# Underprivilege

	logLocation=logs/stowaway.log

	### Remove the log if it is there
	rm -f $logLocation

	## Create the log
	touch $logLocation

	echo "Stowaway Start:" `date` >> $logLocation

	date1=$(date +"%s")




	inputDir=$1
	outputDir=output/

	### clear just in case, this will prevent errors
	rm -rf $outputDir

	### Check to see if the contents of the file exist in the permission table. If not, then add them
	### 1) Overprivilege or underprivilege (which input is being examined)
	### 2) apkFileName
	###	3) rowID
	function CheckAndAddOverAndUnderPrivs { 
		filetype=$1 # over or under priv
		rowid=$3

		### First check to make sure that the file exists
		if [ -f $2/$filetype ];
		then
			### The file exists, so now loop through it and make sure the privs exist in the database
			filename="$2/$filetype"

			### Add empty last line to the file. This is messy.
			sed -i '' -e '$a\' $filename
			while read -r line
			do
    			priv=$line
    			#echo $priv
    			### IF there is a space, then ignore it.
				if [ "$priv" == "${priv//[\' ]/}" ]
				then 
					##echo $name
					### First make sure the permission exists in the list of available
					addIfNotExistsDBPermissions $priv

					### Get the permissionID from the table
					pid=`sqlite3 ../../EvolutionOfAndroidApplications.sqlite  "SELECT pid FROM Permissions WHERE name='$priv';"`
				
					### check to see if the record exists
					count=`sqlite3 ../../EvolutionOfAndroidApplications.sqlite  "SELECT count (permissionid) as permissionidcount FROM $filetype WHERE permissionid='$pid' and apkid='$rowid'"`

					### If the value does not exist, the add it to the table
					if [ $count -eq 0 ]
					then
						sqlite3 ../../EvolutionOfAndroidApplications.sqlite  "INSERT INTO $filetype (permissionid, apkid) VALUES ($pid, '$rowid');"
						#echo Add $pid - $rowid to $filetype
					fi
				fi

				done < "$filename"

		fi


	} 


	### Check to see if the file exists in the DB, if not then add it
	function addIfNotExistsDBPermissions {

		### Check to see if the value exists in the DB
		count=`sqlite3 ../../EvolutionOfAndroidApplications.sqlite  "SELECT count (name) as NameCount FROM Permissions WHERE name='$1';"`
		if [ $count -eq 0 ]
		then
			## The value does not exist, so add it to the table.
			sqlite3 ../../EvolutionOfAndroidApplications.sqlite  "INSERT INTO Permissions (Name) VALUES ('$1');"
			#echo Add $1 to Permissions
		fi
	}

	FILES=$(find $inputDir -type f -name '*.apk')
	for f in $FILES
	do
		#echo $f

		## Run Stowaway
		APKFile=$(basename $f)
		APKFile=${APKFile//.apk/""} ### Remove the apk exension from the apkID
		echo "Begin analyzing: " $APKFile `date` >> $logLocation

		cd tools/stowaway/
		mkdir -p $outputDir/$APKFile


		### Get the rowID and pass it into the 
		### It is done here so it can be passed into later functions and will not need to be re-run   		
   		rowid=`sqlite3 ../../EvolutionOfAndroidApplications.sqlite  "SELECT rowid FROM ApkInformation WHERE ApkId='$APKFile';"`

		./stowaway.sh ../../$f $outputDir/$APKFile
		
		### Check to see if Underprivilege exists
		CheckFor=Underprivilege
   		CheckAndAddOverAndUnderPrivs $CheckFor $outputDir$APKFile $rowid
		
		### Check to see if Overprivilege exists
		CheckFor=Overprivilege
   		CheckAndAddOverAndUnderPrivs $CheckFor $outputDir$APKFile $rowid

		### Clean up the output files
		#rm -rf $outputDir/$APKFile
		rm -rf $outputDir/*
	
		cd ../../

	done


	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "Stowaway Total Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logLocation
	echo "Stowaway End:" `date` >> $logLocation