#!/bin/bash 
### Find Clones


	#### Notes:
	##		You MAY need to rebuild the clones directory if there are errors.


	echo "Run Clones...... " 
	### Date/Time when the script begins to run
	date1=$(date +"%s")

	logLocation=logs/clones.log

	### Location of Clone Detection Functionality
	cloneDetectionLocation=tools/java_Analysis/CloneDetection

	### Location of the clones to be analyzed
	inputLocation=../javaOutput

	### Remove the log if it is there
	rm -f $logLocation

	## Create the log
	touch $logLocation

	echo "Clone Detection Start:" `date` >> $logLocation
	### Messy, but needs to switch here
	cd $cloneDetectionLocation


	### Loop through all of the folders in the input directory
	for i in $(find $inputLocation -mindepth 1 -maxdepth 1 -type d ) 
										### mindepth ignore the top layer
										### Only examine the top layer
	do

		### Run the clone detection tool on each
		
		### Get the total result set from nicad. It is dirty
		cloneResults=`./nicad3 blocks java $i blindrename 2>&1`

		### Clean the resultset from Nicad to only get the clone count
		### This information can also be used to put into the database
		APKFile=$(basename $i)
		APKFile=${APKFile//%apk/""} ### Remove the apk exension from the apkID


		cloneCount=`sed 's/^.*ResultStart //; s/ResultEnd.*$//' <<< $cloneResults`
		echo "Total Clones Found: " $(basename $i) $cloneCount
		
		### Log the necessary information
		echo "Clone Detection Start:" `date` >> ../../../$logLocation
		echo "Total Clones Found: " $(basename $i) $cloneCount >> ../../../$logLocation

		cd ../../../  #moving to the directory with the database
		
		rowid=`sqlite3 EvolutionOfAndroidApplications.sqlite  "SELECT rowid FROM ApkInformation WHERE ApkId='$APKFile';"`
		sqlite3 EvolutionOfAndroidApplications.sqlite  "UPDATE ToolResults SET CodeCloneCount=$cloneCount WHERE ApkId=$rowid;"	
		
		#cd tools/java_Analysis #going back to where you were before
		date2=$(date +"%s")
		diff=$(($date2-$date1))
		echo "Current Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds. for " `echo $(basename $i)` >> $logLocation

		
		cd $cloneDetectionLocation
	done


	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "Total Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> ../../../$logLocation
	echo "Clone Detection End:" `date` >> ../../../$logLocation
