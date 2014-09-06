package dk;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.Statement;

// The purpose of this is to act as a helper class to work on the database


public class DBHelper {

	
	public static void main(String[] args) {
		DBHelper db = new DBHelper();
		db.Run();
	}
	
	public void Run(){
		clearDB();
	}
	
	private void clearDB(){
		
		// clear all temporary DB contents
		Connection c = null;
	    Statement stmt = null;
	    try {
	    	Class.forName("org.sqlite.JDBC");
	      
	    	c = DriverManager.getConnection("jdbc:sqlite:EvolutionOfAndroidApplications.sqlite");
	    	c.setAutoCommit(false);
	    	
	    
	    	
	    	stmt = c.createStatement();
	        String sql = "delete from apkparser_intents";
	        System.out.println(sql);
	        stmt.executeUpdate(sql);
	        c.commit();
		     
	    	

	    	sql = "delete from sqlite_sequence where name = \"apkparser_intents\";";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();
	    	
    	
	    	sql = "delete from apkparser_intents_join";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();

	    	sql = "delete from sqlite_sequence where name=\"apkparser_intents_join\";";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();
	    	
	    	
	    	sql = "delete from apkparser_privs";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();

	    	sql = "delete from sqlite_sequence where name=\"apkparser_privs\";";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();
			    
	    	sql = "delete from apkparser_privs_join";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();

	    	sql = "delete from sqlite_sequence where name=\"apkparser_privs_join\";";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();
			
	    	sql = "delete from ToolResults";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();

	    	sql = "delete from sqlite_sequence where name=\"ToolResults\";";
	    	System.out.println(sql);
	    	stmt.executeUpdate(sql);  
	    	c.commit();
	    	
	    	System.out.println("Files cleared");
	
	    	
	    } catch ( Exception e ) {
		      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
		      System.exit(0);
		    }
		
	}
	

}
