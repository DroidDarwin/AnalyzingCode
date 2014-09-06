#!/bin/bash 



	echo Run Simcad
	#inputLocation=$1
	inputLocation=../../javaOutput/  ### Must have trailing space, or things will not work with a symlink


	logLocation=logs/Simcad.log

	### Remove the log if it is there
	rm -f $logLocation

	## Create the log
	touch $logLocation

	echo "Simcad Start:" `date` >> $logLocation

	date1=$(date +"%s")




	### Make the temp output location
	#tempOutputLocation=tools/java_Analysis/CloneDetection/Simcad-2.2/output/
	tempOutputLocation=/dev/shm/output/
	rm -rf $tempOutputLocation
	mkdir -p $tempOutputLocation


	### Default placeholder values
	### -1 in case of error
	sourceFragmentCount=-1
	cloneFragmentCount=-1
	cloneGroupCount=-1


	cd tools/java_Analysis/CloneDetection/SimCad-2.2/ ### Messy, but I think it works

	for i in $(find $inputLocation -mindepth 1 -maxdepth 1 -type d ) 
	do
		

	  	echo "Simcad Analyzing:" `echo $i` `date` >> ../../../../$logLocation
		APKFile=$(basename $i)
		APKFile=${APKFile//%apk/""} ### Remove the apk exension from the apkID
		mkdir -p $tempOutputLocation/$APKFile # Create the temp output location

		### Examine each by simcad
		temp=`./simcad2 -s $i -l java -o $tempOutputLocation/$APKFile`
	
		### If an exception is thrown, then skip the rest
		if [[ $temp == *"Exception in thread"* ]]
		then
			echo Exception Thrown

		### When there is an exception thrown, it is often because there are no contents to the java file being analyzed.
		###	Check the LOC which is generated
		else
			
			### Parse the expected output
			sourceFragmentCount=`sed 's/^.*Total source fragment ://; s/Total clone fragment.*$//' <<< $temp`
			cloneFragmentCount=`sed 's/^.*Total clone fragment ://; s/Total clone group.*$//' <<< $temp`
			cloneGroupCount=`sed 's/^.*Total clone group\\/pair ://; s/Pre-Processing time :.*$//' <<< $temp`
		fi
	
		echo "APKInformation: " $APKFile
		echo Source Fragment: $sourceFragmentCount
		echo Clone Fragment: $cloneFragmentCount
		echo Clone Group: $cloneGroupCount


		echo ".......Source Fragment:" `echo $sourceFragmentCount` `date` >> ../../../../$logLocation
		echo ".......Clone Fragment:" `echo $cloneFragmentCount` `date` >> ../../../../$logLocation
		echo ".......Clone Group:" `echo $cloneGroupCount` `date` >> ../../../../$logLocation


		### Get the row ID
		rowid=`sqlite3 ../../../../EvolutionOfAndroidApplications.sqlite  "SELECT rowid FROM ApkInformation WHERE ApkId='$APKFile';"`

		### Check to see if the APKID exists in tool results  		
		APKToolResultsCount=`sqlite3 ../../../../EvolutionOfAndroidApplications.sqlite  "SELECT count(*) FROM ToolResults WHERE rowid='$rowid';"`

		if [[ $APKToolResultsCount -eq 0 ]]; then
			echo "Insert"
			sqlite3 ../../../../EvolutionOfAndroidApplications.sqlite  "INSERT INTO ToolResults (ApkId,Simcad_SourceFragment, Simcad_CloneFragment, Simcad_CloneGroup) VALUES ($rowid,$sourceFragmentCount, $cloneFragmentCount, $cloneGroupCount);"
  		else
      		echo updated
      		sqlite3 ../../../../EvolutionOfAndroidApplications.sqlite  "UPDATE ToolResults SET Simcad_SourceFragment=$sourceFragmentCount,  Simcad_CloneFragment=$cloneFragmentCount, Simcad_CloneGroup=$cloneGroupCount WHERE ApkId=$rowid;"
    	fi

		## Remove the temporary output
		rm -rf $tempOutputLocation/$APKFile 
	echo "Simcad End Analyzing:" `echo $i` `date` >> ../../../../$logLocation
	done

	cd ../../../../


	### Make sure all output files have been removed
	rm -rf $tempOutputLocation


	echo "End Simcad:" `date` >> $logLocation

	date2=$(date +"%s")
	diff=$(($date2-$date1))


	echo "Simcad Total Running Time $(($diff / 60)) minutes and $(($diff % 60)) seconds."  >> $logLocation
	echo "Simcad End:" `date` >> $logLocation



	###Todo
	# Logs
	# Test on VM
	# Fix the input javaOutput location to be dynamic
