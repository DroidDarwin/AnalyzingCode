array=()
echo "Begin cleaning up the database"
while IFS='|' read id generatedFileSize; do
	file="$id.apk"
	if [ ! -f "scraper/downloads/full/$file" ]; then
		echo "File $file does not exist"

		if [[ "$generatedFileSize" > "0" ]]; then
			echo "GeneratedFileSize is $generatedFileSize"
		else
			echo "GeneratedFileSize is null"
			array[${#array[*]}]=$id
		fi
	fi
done < <(sqlite3 EvolutionOfAndroidApplications.sqlite "SELECT ApkId, GeneratedFileSize FROM ApkInformation")

for apk in ${array[@]}; do
	echo "Removing record $apk from database"
	sqlite3 EvolutionOfAndroidApplications.sqlite "DELETE FROM ApkInformation WHERE ApkId='$apk'"

	# echo "Setting isDownloaded flag for record $apk"
	# sqlite3 EvolutionOfAndroidApplications.sqlite "UPDATE ApkInformation SET IsDownloaded=0 WHERE ApkId='$apk'"
done
echo "Database cleanup complete"

# echo "Begin deleting files that don't have a database record"
# for file in scraper/downloads/full/*.apk; do
#  	name=${file##*/}
#  	base=${name%.apk}
	
#  	apk=`sqlite3 EvolutionOfAndroidApplications.sqlite "SELECT Name FROM ApkInformation WHERE ApkId='$base'"`
# 	if [ -z "${apk}" ]; then
#  		echo "Deleting file $file"
#  		rm -f $file
#  	fi
# done
# echo "Finished deleting files"
