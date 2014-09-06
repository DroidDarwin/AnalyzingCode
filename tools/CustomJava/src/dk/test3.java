package dk;
import java.sql.*;
import java.util.ArrayList;

public class test3 {

	
	
	// test inserting values
	
	// test updating values
	
	private String apkFileName;
	private String apkContents;
	private String versionCode;
	private String versionName;
	private String minsdk;
	private ArrayList<String>permissionList=new ArrayList<String>(); 
	private ArrayList<String>intentList=new ArrayList<String>();
	
	
	
	private final String DBLocation = "";
	
	
	public static void main(String[] args) {
		test3 t3 = new test3();
		t3.Run2();
	}
	

	public void Run2(){
		
		//System.out.println("fred");
		
		
		// REMOVE THIS IN THE ACTUAL WORKING VERSION
		setupTestData();
		
		
		Connection c = null;
	    Statement stmt = null;
	    try {
	    	Class.forName("org.sqlite.JDBC");
	      
	    	c = DriverManager.getConnection("jdbc:sqlite:EvolutionOfAndroidApplications.sqlite");
	    	c.setAutoCommit(false);
	   //   System.out.println("Opened database successfully");

		// Check to see if the intent exists in the intent table, if not then add it
			for (int a = 0; a < intentList.size(); a++) {
		
				// Check to see if the value exists in the table, if not then add it
				 stmt = c.createStatement();
			     ResultSet rs = stmt.executeQuery( "SELECT count(IntentName) as countval FROM apkParser_intents where intentName = '" + intentList.get(a)  + "' ;" );
			     
			     // If none are found, then add it
			     if(rs.getInt("countval") < 1){
			    	 System.out.println("Insert:" + intentList.get(a));
			    	 stmt = c.createStatement();
				     String sql = "INSERT INTO apkParser_intents (IntentName) VALUES ('"+intentList.get(a)+"' );"; 
				     stmt.executeUpdate(sql);  
				     c.commit();
			     }
			     stmt.close();
			     rs.close();	
			}
			
			// Next add the permissions
			// Check to see if the intent exists in the intent table, if not then add it
			for (int a = 0; a < permissionList.size(); a++) {
		
				// Check to see if the value exists in the table, if not then add it
				 stmt = c.createStatement();
			     ResultSet rs = stmt.executeQuery( "SELECT count(privName) as countval FROM apkParser_privs where privName = '" + permissionList.get(a)  + "' ;" );
			     
			     // If none are found, then add it
			     if(rs.getInt("countval") < 1){
			    	 System.out.println("Insert:" + permissionList.get(a));
			    	 stmt = c.createStatement();
				     String sql = "INSERT INTO apkParser_privs (privName) VALUES ('"+permissionList.get(a)+"' );"; 
				     stmt.executeUpdate(sql);  
				     c.commit();
			     }
			     stmt.close();
			     rs.close();	
			}
		
			// Get the rowID
			 stmt = c.createStatement();
		     ResultSet rs = stmt.executeQuery( "SELECT rowid  FROM apkInformation where apkid = '" + apkFileName  + "' ;" );
		     int RowID=rs.getInt("rowid");
		     stmt.close();
		     rs.close();

		     
		 	// Add the necessary information to the join table
		    // This could probably all be written cleaner and simpler, but I wanted to do this step by step to ensure that
		     //		there would be no issues with information getting out of sync.
		     
		   
		    // Intents
		    // For all of the intents in the array, get their intentID value to add it to the  join table
		 	for (int a = 0; a < intentList.size(); a++) {
		 		
			     stmt = c.createStatement();
			     rs = stmt.executeQuery( "SELECT intentID  FROM apkParser_intents where intentName = '" + intentList.get(a)  + "' ;" );
			     int intentID=rs.getInt("intentID");
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
				     int matchingCount=rs.getInt("matchcount");
				     stmt.close();
				     rs.close();
				     //System.out.println(matchingCount);
				     
				     // since it doesn't match, it should be added to the table
				     if(matchingCount == 0){
				    	 stmt = c.createStatement();
					     sql = "INSERT INTO apkparser_intents_join (intentID, rowID) VALUES (" + intentID +","+RowID+");"; 
					    
					     System.out.println(sql);
					      stmt.executeUpdate(sql);  
					      c.commit();
					     stmt.close();
					  //   c.close();
				     }
				          
				 
			     // end of intents linking
			   
			     
			     // Permissions
				    // For all of the intents in the array, get their intentID value to add it to the  join table
				 	for (int x = 0; x < permissionList.size(); x++) {
				 		
					     stmt = c.createStatement();
					     rs = stmt.executeQuery( "SELECT privID  FROM apkParser_privs where privName = '" + permissionList.get(x)  + "' ;" );
					     int privID=rs.getInt("privID");
					     stmt.close();
					     rs.close();
					     
					     if(privID > 0){ // Check to make sure an actual value was returned
					    	// System.out.println(privID);
					    	 // Make sure the combination does not exist in the linking table
					    	 // It shouldn't, but this is just being on the safe side
					    	 stmt = c.createStatement();
					    	 sql="SELECT count(privID) as matchcount FROM apkparser_privs_join where privID = " + privID  + " and rowID=" + RowID +  ";";
					    	// System.out.println(sql);
					    	 rs = stmt.executeQuery( sql );
						     matchingCount=rs.getInt("matchcount");
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
		     rs = stmt.executeQuery( "SELECT count(rowID) as countrowID FROM toolResults where rowid = '" + RowID  + "' ;" );
		     
		     // If none are found, then add it
		     if(rs.getInt("countrowID") < 1){
		    	 System.out.println("Insert:" + RowID);
		    	 stmt = c.createStatement();
			     sql = "INSERT INTO ToolResults (apkID) VALUES ("+RowID+" );"; 
			     stmt.executeUpdate(sql);  
			     c.commit();
		     }
		     stmt.close();
		     rs.close();	
		     
		     // Now insert the basic toolresult values
		     stmt = c.createStatement();
		    // String sql = "INSERT INTO ToolResults (apkID, apkParser_versionCode, apkParser_VersionName, 
		     //apkParser_minsdk) VALUES (" + RowID + ", '"+versionCode+"','"+versionName+"','"+minsdk+"' );"; 
		
		     sql = "Update toolResults set apkParser_versionCode='"+versionCode+"', apkParser_VersionName='"+versionName+"', apkParser_minsdk='"+minsdk+"' where rowID=" + RowID;
		    // System.out.println(sql);
		    
		     stmt.executeUpdate(sql);  
		     c.commit();
		     c.close();
		     stmt.close();
		     rs.close();
			     }
		 	}
	      stmt.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	      System.exit(0);
	    }
		
	}
	
	
	

	public void setupTestData(){
	
	//	apkFileName = "74d42d06-3dcc-4652-a266-4460ee327f7d"; //9803 - 
		apkFileName = "1bcfdd77-e4d3-4244-9698-01697ad5dc2a"; // 9801 - Pintrest
		
		// Pinterest
		versionCode = "3.0.1";
		versionName="144"; // convert to int?
		minsdk="6"; // convert to int?
//		permissionList.add("android.permission.INTERNET2"); 
		permissionList.add("android.permission.VIBRATE"); 
	//	permissionList.add("android.permission.WRITE_CONTACTS");
	//	permissionList.add("android.permission.INTERNET");		// double
	//	permissionList.add("android.permission.GET_ACCOUNTS"); 
		permissionList.add("android.permission.PintrestONLY");
		intentList.add("android.intent.category.DEFAULT"); 
//		intentList.add("android.intent.action.SEARCH"); 
	//	intentList.add("android.intent.category.DEFAULTPint"); 		// double
	//	intentList.add("android.intent.category.LAUNCHER"); 
		
		
	
	}
	
	
	
	public void Run(){
		Connection c = null;
	    Statement stmt = null;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:EvolutionOfAndroidApplications.sqlite");
	      c.setAutoCommit(false);
	      System.out.println("Opened database successfully");

	      
	     /*
	   // Practice inserting
	      stmt = c.createStatement();
	      String sql = "INSERT INTO test (NAME) VALUES ('Paul' );"; 
	      stmt.executeUpdate(sql);  
	      c.commit();
	 */
	   
	      /*
	      // delete values
	      stmt = c.createStatement();
	      String sql = "DELETE from test where ID=2;";
	      stmt.executeUpdate(sql);
	      c.commit();
	      */
	      /*
	      // update Values
	      stmt = c.createStatement();
	      String sql = "UPDATE test set Name = 'Dan' where ID=1;";
	      stmt.executeUpdate(sql);
	      c.commit();
	      */
	      
	      
	      stmt = c.createStatement();
	      ResultSet rs = stmt.executeQuery( "SELECT * FROM test;" );
	      while ( rs.next() ) {
	      //   int id = rs.getInt("id");
	         
	         
	    	int  id = rs.getInt("id");
	         String  name = rs.getString("name");
	       //  int age  = rs.getInt("age");
	       //  String  address = rs.getString("address");
	       //  float salary = rs.getFloat("salary");
	         System.out.println( "ID = " + id );
	         System.out.println( "NAME = " + name );
	      //   System.out.println( "AGE = " + age );
	      //   System.out.println( "ADDRESS = " + address );
	      //   System.out.println( "SALARY = " + salary );
	      //   System.out.println();
	      }
	      
	      rs.close();
	      
	      stmt.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	      System.exit(0);
	    }
	    System.out.println("Operation done successfully");
	}

}
