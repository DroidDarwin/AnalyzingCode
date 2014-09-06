package dk;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

// Todo:
//	Try on different input files
//	Logging
//	Write to sqliteDB
//	Test, test, test



public class test2 {
	
	
	
	
	
	
	public static void main(String[] args) throws SAXException, IOException,ParserConfigurationException, TransformerException {
	
	DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory.newInstance();
	DocumentBuilder docBuilder = docBuilderFactory.newDocumentBuilder();
	Document document = docBuilder.parse(new File("src/testinput/0.txt"));
	readXMLInfo(document.getDocumentElement());
}
	
	public static void readXMLInfo(Node node) {
	    // do something with the current node instead of System.out
	 //  System.out.println(node.getNodeName());
	   
		// Contains a list of all the required permissions
		List<String> permissionsList=new ArrayList<String>();
		
		// Contains all used intents 
		List<String> intentList=new ArrayList<String>();
		
		String versionCode ="";
		String versionName ="";
		
		// Get version information
	   if(node.getNodeName().toString().equals("manifest")){
		//   System.out.println(node.getAttributes().item(1).getNodeName());
		//   System.out.println(node.getAttributes().item(2));
		   
		   // Loop through all of the possible values 
		   // This is done just in case the order of the items change at all
		   for (int a = 0; a < node.getAttributes().getLength(); a++) {
			   if(node.getAttributes().item(a).getNodeName().equals("android:versionCode")){
				   versionCode=node.getAttributes().item(a).getNodeValue();
			   }
			   if(node.getAttributes().item(a).getNodeName().equals("android:versionName")){
				   versionName=node.getAttributes().item(a).getNodeValue();
			   }
		   }
		   
	   }
	   	   

	    NodeList nodeList = node.getChildNodes();
	    for (int i = 0; i < nodeList.getLength(); i++) {
	        Node currentNode = nodeList.item(i);
	        
	        
	        // Get the number of permissions used in the Application
	        if(currentNode.getNodeName().toString().equals("uses-permission")){
	        //	System.out.println(currentNode.getNodeName());
	        //	System.out.println(currentNode.getChildNodes().getLength());
	       // 	final String permission =currentNode.getAttributes().item(0).toString().replace("\"", "").replace("android:name=android.permission.", ""); 
	    
	        	final String permission =currentNode.getAttributes().item(0).getNodeValue().toString(); 
	        	permissionsList.add(permission);
	        //	System.out.println(permission);
	        //	doSomething(currentNode);
	        }
	        
	        // Get the min sdk version
	        if(currentNode.getNodeName().toString().equals("uses-sdk")){
	        	//System.out.println("hi"); 
	     //   	final String sdk =currentNode.getAttributes().item(0).toString().replace("\"", "").replace("android:minSdkVersion=", ""); 
	        	final String sdk =currentNode.getAttributes().item(0).getNodeValue().toString(); 
	       // 	System.out.println(sdk);
	        }
	        
	        
	      //  System.out.println(currentNode.getNodeName());
	        // Get Intent Filters
	        // Not working
	        if(currentNode.getNodeName().toString().equals("application")){
	        	
	        	//works on the first item
	        	//System.out.println(currentNode.getChildNodes().item(5).getChildNodes().item(1).getChildNodes().item(1).getAttributes().item(0));
	       
	      //  System.out.println(currentNode.getChildNodes().item(5).getChildNodes().item(1).getChildNodes().item(1).getAttributes().item(0));
	 	    

	        	//System.out.println(currentNode.getChildNodes().item(5).getNodeName());
	        	
	        	for (int b = 0; b < currentNode.getChildNodes().getLength(); b++) {
	        		if(currentNode.getChildNodes().item(b).getNodeName().equals("activity")){
	        		//	System.out.println(currentNode.getChildNodes().item(z).getChildNodes().item(1).getChildNodes().item(1).getAttributes().item(0));
	        		//	System.out.println(currentNode.getChildNodes().item(z).getChildNodes();
	        			for (int c = 0; c < currentNode.getChildNodes().item(b).getChildNodes().getLength(); c++) {
	        				if(currentNode.getChildNodes().item(b).getChildNodes().item(c).getNodeName().equals("intent-filter")){
	        				//	System.out.println(currentNode.getChildNodes().item(b).getChildNodes().item(c).getNodeName());
	        					for (int d = 0; d < currentNode.getChildNodes().item(b).getChildNodes().item(c).getChildNodes().getLength(); d++) {
	        						//System.out.println(currentNode.getChildNodes().item(b).getChildNodes().item(c).getChildNodes().item(d).getNodeName());
	        						String Name = currentNode.getChildNodes().item(b).getChildNodes().item(c).getChildNodes().item(d).getNodeName();
	        						if(!Name.trim().equals("#text")){
	        							//System.out.println(currentNode.getChildNodes().item(b).getChildNodes().item(c).getChildNodes().item(d).getNodeName());
	        							System.out.println(currentNode.getChildNodes().item(b).getChildNodes().item(c).getChildNodes().item(d).getAttributes().item(0).getNodeValue());
	        						       
	        						}
	        					
	        					}
	        				}
	        			
	        			}
	        		}
	        	}
	        	
	        	
	        	
	        	
	        	
	        //	System.out.println(currentNode.getChildNodes().item(5).getChildNodes().item(1).getChildNodes().item(1).getNodeValue());
	        	

	        }
	        
	        
	        // Get Version Info
	        //if(currentNode.getNodeName().toString().equals("manifest")){
		       //	System.out.println("hi"); 
		        //	System.out.println(currentNode.getChildNodes().item(3).getAttributes());
		        	//final String intent =currentNode.getAttributes().item(0).toString().replace("\"", "").replace("android:minSdkVersion=", ""); 
		        	//System.out.println(intent);
		        	//doSomething(currentNode);
		      //  }
	        
	        
	    //    if (currentNode.getNodeType() == Node.ELEMENT_NODE) {
	            //calls this method for all the children which is Element
	        
	       // if(currentNode.getLocalName().toString().equals("Manifest")){
	            //doSomething(currentNode);
	        	
	       // }
	           //System.out.println(currentNode.getLocalName()); // Manifest
	      //      System.out.println(currentNode.getAttributes().getNamedItem("android:versionName"));
	     //   }
	    }

	    
	    
	    //	 System.out.println("Version Code:" + versionCode);
		//   System.out.println("Version Name:" + versionName);
	   // System.out.println(permissionsList.get(1));
	}

	   
	
}
