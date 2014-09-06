#!/bin/bash


#### Pull data from repo
### Not sure if this is needed
#echo "Pull data"




echo "Update Permissions"

#chmod -R a+x tools/java_Analysis/jlint/jlint 		# Not sure if this is needed
#chmod -R a+x tools/java_Analysis/jlint/run_jlint.sh # Not sure if this is needed

chmod -R a+x toolsScript.sh
chmod -R a+x tools/java_Analysis/APK_to_JAVA/convert_APK_Java.sh
chmod -R a+x tools/java_Analysis/APK_to_JAVA/dex2jar-0.0.9.15/dex2jar.sh
chmod -R a+x tools/java_Analysis/CloneDetection/runclones.sh
chmod -R a+x tools/java_Analysis/checkstyle/checkStyle.sh

chmod -R a+x tools/java_Analysis/CloneDetection/nicad3
chmod -R a+x tools/androguard/androrisk.py
chmod -R a+x tools/java_Analysis/CloneDetection/scripts/NiCadPair-Ubuntu
chmod -R a+x EvolutionOfAndroidApplications.sqlite
chmod -R a+x tools/java_Analysis/CloneDetection/SimCad-2.2/run_simcad.sh


