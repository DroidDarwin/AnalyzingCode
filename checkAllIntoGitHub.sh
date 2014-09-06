#!/bin/bash 

	### LogsDirectory
	logsDir=logs
	logLocation=$logsDir/GitHubCheckin.log
	
	date1=$(date +"%s") ## Start date of the script
	echo "GitHub Checkin Start:" `date` >> $logLocation
	
	### Find all sqlDB files and check them in
	###		This is done just in case the filename is changed and it needs to be updated
	FILES=$(find . -mindepth 1 -maxdepth 1 -type f -name '*.sqlite')
	for f in $FILES
	do
		echo $f
		echo "Checking in database:" $f `date` >> $logLocation
	done


	echo "Start Pushing to Public Repo:" `date` >> $logLocation

	### Copy the SQLite DB to the publicly available GH Repo
	
		### Get the SQLIte DB to copy from
	echo "Copy DB:" `date` >> $logLocation

	### Make sure to overwite the existing copy of the DB
	cp EvolutionOfAndroidApplications.sqlite ../DarwinData/
	echo "Check into Public GH repo"
	echo "Check the repo into GH:" `date` >> $logLocation
	cd ../DarwinData

	git add EvolutionOfAndroidApplications.sqlite
	git commit -m "Automatic checkin: `date`"
	git push https://EvolutionOfAndroid:ProjectKrutz1@github.com/DroidDarwin/DarwinData.git master
	cd ../ProjectKrutz


	### Now we do all the commiting ##
	echo "Commit Logs and Database to Github"
	##if this is the VM then run this
	##put the password into it
	
	echo "Committing to GitHub:" `date` >> $logLocation

	git add $logsDir/*
	git add EvolutionOfAndroidApplications.sqlite
	git commit -m "Automatic checkin"
	git push https://EvolutionOfAndroid:ProjectKrutz1@github.com/cklimkowsky/ProjectKrutz.git master


	echo "Done Commiting Logs"