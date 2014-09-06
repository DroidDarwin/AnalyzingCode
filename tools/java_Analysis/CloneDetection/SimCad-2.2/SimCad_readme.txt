SimCad Clone Detector
---------------------
Version 	: 2.2
Released on	: December 2013
Supported OS	: Mac OS X, Linux.
Technical Ref. 	: http://dx.doi.org/10.1109/WCRE.2011.12
Origin 		: Software Research Lab., Dept. of Computer Science, University of Saskatchewan, Canada

Disclaimer
----------
The information is provided "as is", without warranty of any kind, whether expressed or implied.
Please use ALL information, commands and configuration with care and at your OWN sole responsibility.
The Authors' will not be responsible for any damages or loss of any kind resulting from its use.

What's new in SimCad-2.2
----------------------
# added Graphical User Interface, can be invoked by providing option '-gx' in command, e.g., $ ./simcad2 -gx
# added support of reusing existing clone index, can be overridden by providing option '-f' that forces simcad to build clone index from scratch
# added xslt based visualization of detected clones, can be seen by open the detection result xml file in a HTML5 compatible browser  

What's new in SimCad-2.1
----------------------
# added new optional command-line parameter "item_to_search". This parameter takes a file/folder that contains source code to search for similar code in a target project. i.e., the detection behaves like a code search. When it is ignored, all clones inside the target project will be searched.
# added detection from source in pre-defined xml file of the following format. See instructions at step 5 in next section for command details. 
Please remember to use '-n' for non-exclusive fragments e.g, blocks.

   <source file=".../file-1.x" startline="x" 
		endline="y">
	fragment 1 text 
   </source>
   ...
   <source file=".../file-n.x" startline="x" 
		endline="y">
	fragment n text
   </source>


Installation Steps
------------------

1. Install java 1.5 or later (http://java.sun.com/javase/downloads/index.jsp)
	1.1 Check java installation
		$ java -version 

2. Install TXL 10.5i or later (http://www.txl.ca)
	2.1 Check TXL installation
		$ txl -V

3. Install SimCad
	3.1 Extract the archive
		$ cd PATH_CONTAINING_SimCad-2.2.zip
		$ unzip SimCad-2.2.zip
	3.2 $ cd SimCad-2.2
	3.3 $ make 
	3.4 Check SimCad installation
		$ ./simcad2 -version

4. Run SimCad
	4.1 $ cd PATH_CONTAINING_SimCad-2.2/SimCad-2.2
	4.2 $ ./simcad2 [-version] [-v] [-f] [-help] [-gx] [-i item_to_search] -s source_path -l language
               [-g granularity] [-t clone_type] [-c clone_grouping] [-x source_transform] [-o output_path]

		 -version          : display simcad version
		 -v                : verbose mode, shows the detection in progress
		 -f                : force detection to discards previous pre-processed resources if exist
		 -help             : print this run instruction
		 -gx               : display graphical user interface
		 language          : name of the source language [ c | java | cs | py ]
		 granularity       : source fragment type [ (block | b ) | (function | f) : default = (function | f) ]
		 clone_type        : types of clone to seek [ 1 | 2 | 3 | 12 | (23 | nearmiss) | 13 | (123 | all) : default = (123 | all) ]
		 clone_grouping    : grouping of clones [ (group | cg) | (pair | cp) : default = (group | cg) ]
		 source_transform  : source transformation strategy [ (generous | g) | (greedy | G) : default = (generous | g) ]
		 item_to_search    : absolute path to file/folder contains search candidates
		 source_path       : absolute path to source folder
		 output_path       : absolute path to output folder [ default = {source_path}_simcad_clones ]
		
	4.3 example
		$ ./simcad2 -s /Users/foo/Documents/workspaces/my-project -l java
		
5. Run SimCadXML
	5.1 $ cd PATH_CONTAINING_SimCad-2.2/SimCad-2.2
	5.2 $ ./ simcad2xml [-version] [-v] [-f] [-help] [-gx] [-n] -s o_source_xml [-x t_source_xml]
                  [-t clone_type] [-c clone_grouping] [-o output_path]

		 -version        : display simcad version
		 -v              : verbose mode, shows the detection in progress
		 -f                : force detection to discards previous pre-processed resources if exist
		 -help           : print this run instruction
		 -gx             : display graphical user interface
		 -n              : non-exclusive fragments, a fragment might contain in another bigger fragment
		 o_source_xml    : absolute path to xml file containing original source fragments
		 t_source_xml    : absolute path to xml file containing transformed source fragments
		 clone_type      : types of clone to seek [ 1 | 2 | 3 | 12 | (23 | nearmiss) | 13 | (123 | all) : default = (123 | all) ]
		 clone_grouping  : grouping of clones [ (group | cg) | (pair | cp) : default = (group | cg) ]
		 output_path     : absolute path to output folder [ default = {source_path}_simcad_clones ]		

6. Script Permission Issue

While running simcad command if you face permission issue like as follows:

	-bash: ./simcad2: Permission denied

Please do the following:
	1. cd PATH_CONTAINING_SimCad-2.2/SimCad-2.2
	2. chmod +x ./simcad2
	3. chmod +x ./simcad2xml
	4. cd scripts
	5. chmod +x ./*
	6. cd ../
	7. now execute command ./simcad2

Customize SimCad Configuration
------------------------------
SimCad uses the following list of configuration parameters that can be overridden by providing a similar entry with new value in the external configuration file "simcad.cfg.xml" located in folder SimCad-2.2/tools/.

<entry key="simcad.settings.advance.strictOnMembership">true</entry>
<entry key="simcad.settings.advance.clusterMembershipRatio">0.5</entry>
Note: If 'strictOnMembership' is true, each fragment in a clone class must have at least 'clusterMembershipRatio' times total pairwise similar fragments in that class.


<entry key="simcad.settings.advance.locTolerance">1.0</entry>
Note: Number of lines ('locTolerance' times size of search candidate fragment) that can be varied in a pair of Type-3 clone fragment.

<entry key="simcad.settings.advance.type3clone.simthreshold">13</entry>
Note: Maximum value of simthreshold for Type-3 clone search. The range of values can be used is 1-13.

<entry key="simcad.settings.advance.thresholdStabilizationValueForGreedyTransform">0.3</entry>
Note: In greedy transform, code fragment gets very many same variable names because of the nature of the transform (which means some fake textual similarity is being injected in the code, leading to increased false positive Type-3 detection at high simthreshold value). 
To compensate this, the max similarity threshold is being lowered to the factor of some constant (thresholdStabilizationValueForGreedyTransform)

<entry key="simcad.settings.advance.tokenFrequencyNormalization">true</entry>
<entry key="simcad.settings.advance.tokenFrequencyNormalizationThreshold">5</entry>
<entry key="simcad.settings.advance.tokenFrequencyNormalizationOverThresholdValue">5</entry>
Note: when the Token Frequency Normalization is enabled (tokenFrequencyNormalization = true), if the frequency (this number affects simhash value) of a token in a code fragment becomes much higher (greater than tokenFrequencyNormalizationThreshold), it can be suppressed to a fixed value as set in tokenFrequencyNormalizationOverThresholdValue or even the high frequency token can be made ignored in simhash calculation if tokenFrequencyNormalizationOverThresholdValue is set to 0.

<entry key="simcad.settings.general.minClusterSize">2</entry>
Note: Minimum number of fragments in a clone class

<entry key="simcad.settings.general.minSizeOfGranularity">5</entry>
Note: Code fragments of size less than 'minSizeOfGranularity' will be ignored in clone detection

<entry key="simcad.settings.general.fragment.file.relativeURL">true</entry>
Note: Defines whether detection result will contain absolute or relative URL of the clone fragment

<entry key="simcad.settings.general.fragment.unicodeFilterOn">false</entry>
Note: If source code contains UNICODE characters, this filter needs to be on/true to have them processed correctly. 

<entry key="simcad.settings.general.install.url">../../SimCad-2.2</entry>
Note: Relative location of the SimCad installation root directory with respect to the clone detection library