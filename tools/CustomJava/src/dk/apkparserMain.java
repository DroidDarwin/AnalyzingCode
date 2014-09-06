package dk;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;



// Application will parse the .xml file returned from apkparser. Will then input into the sqliteDB
// Usage: apkParserMain <input.apk>

// 1) Run APK analyzer on the .apk file
// 2) Run APK Parser command on the .apk file
// 3) Input the information into the SQLiteDB

/*Helpful possible usage
 * Dan-macbook:src dan$ pwd
/Users/dan/Documents/workspace/ProjectKrutz/tools/CustomJava/src
Dan-macbook:src dan$ javac dk/*.java; java -classpath ".:sqlite-jdbc-3.7.2.jar" dk/apkparserMain /Users/dan/Desktop/testAPKInput/
 */


public class apkparserMain {
	util u = new util();
	
	//Name of the application being examined 

	private List<apkItem>MasterapkList=new ArrayList<apkItem>();
	final String sqliteDBLocation = "../../EvolutionOfAndroidApplications.sqlite";	// location of the SQLDB that the information will be written to
	
	// shut this off in "real" version, it will be read in as an input variable.
	//final String inputLocation = "src/testinput/testAPKInput"; 
	
	
//	final String inputLocation = "/Users/dan/Desktop/temp/"; 
	
	static String inputLocation = ""; 
	
	public static void main(String[] args) throws IOException, InterruptedException, ParserConfigurationException, SAXException {
		
	
		if(args.length!=1){
			System.out.println("A single argument with the application name was expected");
		}else{
			 inputLocation = args[0];
			apkparserMain ap = new apkparserMain();
			ap.Run();		
		}
		
		
	
	//	apkparserMain ap = new apkparserMain();
	//	ap.Run();	
		
	}
	
	public void Run() throws IOException, InterruptedException, ParserConfigurationException, SAXException{	
	
		
		System.out.println("Input Location: " + inputLocation);
	//	System.exit(0);
		
		// These actions are done in seperate steps to eliminate the possibility of locking or other timing issues
	
		// Create the raw version of the list
		MasterapkList = buildAPKItems(inputLocation);
		
		
		// Gather the necessary apk information from apk object
		gatherAPKInfo(MasterapkList);
	
		// Insert the values into the database
		enterDataIntoDB();
		
		/*
		// Loop through all of the items just to test
		for (int i = 0; i < MasterapkList.size(); i++){
			
			System.out.println(MasterapkList.get(i).getApkFileName());
			System.out.println(MasterapkList.get(i).getVersionName());
			System.out.println(MasterapkList.get(i).getVersionCode());
			System.out.println(MasterapkList.get(i).getMinsdk());
			System.out.println(MasterapkList.get(i).getTargetsdk());
			System.out.println(MasterapkList.get(i).getPermissionList().size());
			System.out.println(MasterapkList.get(i).getIntentList().size());
			
			for (int z = 0; z <MasterapkList.get(i).getPermissionList().size(); z++){
				System.out.println(MasterapkList.get(i).getPermissionList().get(z));
			}
			
			for (int z = 0; z <MasterapkList.get(i).getIntentList().size(); z++){
				System.out.println(MasterapkList.get(i).getIntentList().get(z));
			}

		}
		*/
		
	}
	
	
	private void enterDataIntoDB(){
		// Loop through all of the apkItems
			Connection c = null;
		    Statement stmt = null;
		    try {
		    	Class.forName("org.sqlite.JDBC");
		    	String prefix = "";
		    	if(System.getProperty("user.dir").contains("src")){
					prefix = "../";
				}
		    	
		    	final String sqlliteLocation = "jdbc:sqlite:"+prefix+sqliteDBLocation;
		    	System.out.println(sqlliteLocation);
		    	c = DriverManager.getConnection(sqlliteLocation);
		    	c.setAutoCommit(false);
		   //   System.out.println("Opened database successfully");
		    	for (int i = 0; i < MasterapkList.size(); i++){
		    		System.out.println("Insert Info for:" + MasterapkList.get(i).getApkFileName());
		
			// Check to see if the intent exists in the intent table, if not then add it
				
				 for (int a = 0; a < MasterapkList.get(i).getIntentList().size(); a++) {

					// Check to see if the value exists in the table, if not then add it
					 stmt = c.createStatement();
					 String sql1a="SELECT count(IntentName) as countval FROM apkParser_intents where intentName = '" + MasterapkList.get(i).getIntentList().get(a)  + "' ;";
				     
					 ResultSet rs2 = stmt.executeQuery( sql1a );
					// System.out.println("NOT GETTING HERE");
				     // If none are found, then add it
					
					 	int countval = 0;
					    if (rs2.next()) {
					    	countval = rs2.getInt("countval");
					    }
					     // If none are found, then add it
					     if(countval < 1){
					    	 stmt = c.createStatement();
						     String sql = "INSERT INTO apkParser_intents (IntentName) VALUES ('"+MasterapkList.get(i).getIntentList().get(a)+"' );"; 
						     stmt.executeUpdate(sql);  
						     c.commit();
					     }
				  //   stmt.close();
				  //   rs2.close();	
				   
				 
				}
				

								// Next add the permissions
				// Check to see if the intent exists in the intent table, if not then add it
				for (int a = 0; a < MasterapkList.get(i).getPermissionList().size(); a++) {
			
					// Check to see if the value exists in the table, if not then add it
					 stmt = c.createStatement();
				     ResultSet rs = stmt.executeQuery( "SELECT count(privName) as countval FROM apkParser_privs where privName = '" + MasterapkList.get(i).getPermissionList().get(a)  + "' ;" );
				    
				    int countval = 0;
				    if (rs.next()) {
				    	countval = rs.getInt("countval");
				    }
				     // If none are found, then add it
				     if(countval < 1){
				    	// System.out.println("Insert140:" + MasterapkList.get(i).getPermissionList().get(a));
				    	 stmt = c.createStatement();
					     String sql = "INSERT INTO apkParser_privs (privName) VALUES ('"+MasterapkList.get(i).getPermissionList().get(a)+"' );"; 
					     stmt.executeUpdate(sql);  
					     c.commit();
				     }
				     stmt.close();
				     rs.close();	
				}
			
				
				// Get the rowID
				 stmt = c.createStatement();
			     ResultSet rs = stmt.executeQuery( "SELECT rowid  FROM apkInformation where apkid = '" + MasterapkList.get(i).getApkFileName()  + "' ;" );
			     
			     int RowID=0;
			     if (rs.next()) {
			    	 RowID=rs.getInt("rowid");
			     }
			     stmt.close();
			     rs.close();

			     
			 	// Add the necessary information to the join table
			    // This could probably all be written cleaner and simpler, but I wanted to do this step by step to ensure that
			     //		there would be no issues with information getting out of sync.
			     
			
			    // Intents
			    // For all of the intents in the array, get their intentID value to add it to the  join table
			 	for (int a = 0; a < MasterapkList.get(i).getIntentList().size(); a++) {
			 		
				     stmt = c.createStatement();
				     rs = stmt.executeQuery( "SELECT intentID  FROM apkParser_intents where intentName = '" + MasterapkList.get(i).getIntentList().get(a)  + "' ;" );
				    
				     int intentID=0;
				     if (rs.next()) {
				    	 intentID=rs.getInt("intentID");
				     }
				     
				     stmt.close();
				     rs.close();
				     
				     if(intentID > 0){ // Check to make sure an actual value was returned
				    	// System.out.println(intentID);
				    	 // Make sure the combination does not exist in the linking table
				    	 // It shouldn't, but this is just being on the safe side
				    	 stmt = c.createStatement();
				    	 String sql="SELECT count(intentID) as matchcount FROM apkparser_intents_join where intentID = " + intentID  + " and rowID=" + RowID +  ";";
				    	// System.out.println(sql);
				    	 rs = stmt.executeQuery( sql );
					    // int matchingCount=rs.getInt("matchcount");
				    	 int matchingCount=0;
					     if (rs.next()) {
					    	 matchingCount=rs.getInt("matchcount");
					     }
					     stmt.close();
					     rs.close();
					     //System.out.println(matchingCount);
					     
					     // since it doesn't match, it should be added to the table
					     if(matchingCount == 0){
					    	 stmt = c.createStatement();
						     sql = "INSERT INTO apkparser_intents_join (intentID, rowID) VALUES (" + intentID +","+RowID+");"; 
						    
						    // System.out.println(sql);
						     stmt.executeUpdate(sql);  
						     c.commit();
						     stmt.close();
						  //   c.close();
					     }			          
				     }
				    
			 	}
			 	
			 	// end of intents linking

				     // Permissions
					    // For all of the intents in the array, get their intentID value to add it to the  join table
					 	for (int x = 0; x < MasterapkList.get(i).getPermissionList().size(); x++) {
					 		
						     stmt = c.createStatement();
						     rs = stmt.executeQuery( "SELECT privID  FROM apkParser_privs where privName = '" + MasterapkList.get(i).getPermissionList().get(x)  + "' ;" );
						     
						     int privID=0;
						     if (rs.next()) {
						    	 privID=rs.getInt("privID");
						     }
						     					     
						     stmt.close();
						     rs.close();
						     
						     if(privID > 0){ // Check to make sure an actual value was returned
						    	// System.out.println(privID);
						    	 // Make sure the combination does not exist in the linking table
						    	 // It shouldn't, but this is just being on the safe side
						    	 stmt = c.createStatement();
						    	String sql="SELECT count(privID) as matchcount FROM apkparser_privs_join where privID = " + privID  + " and rowID=" + RowID +  ";";
						    	// System.out.println(sql);
						    	 rs = stmt.executeQuery( sql );
							    
						    	 //matchingCount=rs.getInt("matchcount");
						    	 
						    	 int matchingCount=0;
							     if (rs.next()) {
							    	 matchingCount=rs.getInt("matchcount");
							     }
							     stmt.close();
							     rs.close();
							     //System.out.println(matchingCount);
							     
							     // since it doesn't match, it should be added to the table
							     if(matchingCount == 0){
							    	 stmt = c.createStatement();
								     sql = "INSERT INTO apkparser_privs_join (privID, rowID) VALUES (" + privID +","+RowID+");"; 
								    
								     System.out.println(sql);
								     stmt.executeUpdate(sql);  
								     c.commit();
								     stmt.close();
								  //   c.close();
							     }
							          
							     stmt.close();
							     rs.close();
						     }
						     // end of priv linking	
			 	}
			    	
				// Next insert the information into the apk tools table
				 stmt = c.createStatement();
				String sql="SELECT count(rowID) as countrowID FROM toolResults where rowid = '" + RowID  + "' ;";
				// System.out.println(sql);
			     rs = stmt.executeQuery( sql );
			   
			     // If none are found, then add it
			     int countrowID=0;
			     if (rs.next()) {
			    	 countrowID=rs.getInt("countrowID");
			     }	     
			     
			     if(countrowID < 1){
			    	// System.out.println("Insert251:" + RowID);
			    	 stmt = c.createStatement();
				     sql = "INSERT INTO ToolResults (apkID) VALUES ("+RowID+" );"; 
				     System.out.println(sql);
				     stmt.executeUpdate(sql);  
				     c.commit();
			     }
			     stmt.close();
			     rs.close();	
	   
			     // Now insert the basic toolresult values
			     stmt = c.createStatement();
			    // String sql = "INSERT INTO ToolResults (apkID, apkParser_versionCode, apkParser_VersionName, 
			     //apkParser_minsdk) VALUES (" + RowID + ", '"+versionCode+"','"+versionName+"','"+minsdk+"' );"; 
			    
			     sql = "Update toolResults set apkParser_versionCode='"+MasterapkList.get(i).getVersionCode()+"', apkParser_VersionName='"+MasterapkList.get(i).getVersionName()+"', apkParser_minsdk='"+ MasterapkList.get(i).getMinsdk()+"'  , apkParser_targetsdk='"+ MasterapkList.get(i).getTargetsdk()+"'       where rowID=" + RowID;
			    
			    System.out.println(sql);
			     
			     stmt.executeUpdate(sql);  
			     c.commit(); 
	
			//     c.close();
			 //    stmt.close();
			//     rs.close();
			    
				 }

//		      stmt.close();
//		      c.close();	
		     
	
		    } catch ( Exception e ) {
		      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
		      System.exit(0);
		    }

	}
	
	
	// Modify the contents of the object to create the necessary information for output
	// ? Might be a good idea to refactor this to take place directly in the apkItem object
	private void gatherAPKInfo(List<apkItem>apkList) {

		
	
			// Loop through objects in the list
			for (int i = 0; i < apkList.size(); i++){
				System.out.println(apkList.get(i).getApkFileName());
				try {
					apkList.get(i).parseXMLInfo();
				} catch (ParserConfigurationException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (SAXException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
	
		
		
		

	}
	
	// Add a check to ensure that some input files exist
	// errors could occur later on if this is not done
	private List<apkItem> buildAPKItems(String inputLocation) throws IOException, InterruptedException{
		
		List<apkItem>apkList=new ArrayList<apkItem>();
		
		// loop through all of the apk files in the input directory
		File path = new File(inputLocation);
		File [] files = path.listFiles();
		
		    for (int i = 0; i < files.length; i++){
		        if (files[i].isFile()){ //this line weeds out other directories/folders
		        	// Make sure the file is an apk file
		        	// String ext = FilenameUtils.getExtension("/path/to/file/foo.txt");
		        	//if(FilenameUtils.){String extension = "";
		        	String extension = "";
					int a = files[i].getName().lastIndexOf('.');
					//if (i > 0) {
					    extension = files[i].getName().substring(a+1);
					//}
				
						if(extension.toLowerCase().equals("apk")){
						//	System.out.println(files[i].getName());
							apkList.add(new apkItem(files[i].getName(),RunAPKParser(files[i])));
						}
		        	}
		        }
		

		return apkList;
	}
	
	// Analyze the target .apk file and return its generated XML information
	private String RunAPKParser(File inputFile) throws IOException, InterruptedException{
	
		String prefix = "";
		
		// Determine if the parser should "move up" one
		// This is needed since the application may be invoked from different locations
		if(System.getProperty("user.dir").contains("src")){
			prefix = "../";
		}
		
		final String apkParserCommand = "java -jar "+prefix+"src/apkparser/APKParser.jar ";
		String s = null;
		StringBuilder sb = new StringBuilder();

	        try {
	            
		    // run the Unix "ps -ef" command
	            // using the Runtime exec method:
	            Process p = Runtime.getRuntime().exec(apkParserCommand + inputFile.getAbsolutePath());
	            
	            BufferedReader stdInput = new BufferedReader(new 
	                 InputStreamReader(p.getInputStream()));

	            BufferedReader stdError = new BufferedReader(new 
	                 InputStreamReader(p.getErrorStream()));

	            // read the output from the command
	          //  System.out.println("Here is the standard output of the command:\n");
	            while ((s = stdInput.readLine()) != null) {
	               // System.out.println(s);
	            	sb.append(s);
	            }
	            
	            // read any errors from the attempted command
	          //  System.out.println("Here is the standard error of the command (if any):\n");
	            while ((s = stdError.readLine()) != null) {
	                System.out.println(s);
	            }
	        }
	        catch (IOException e) {
	            //System.out.println("exception happened - here's what I know: ");
	            e.printStackTrace();
	            System.exit(-1);
	        }
			//	System.out.println("The final output: " + sb.toString());
	        return sb.toString();    
	}

}
