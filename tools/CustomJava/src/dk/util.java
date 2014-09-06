package dk;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;


public class util {

	public String readDoc(String inputFile) throws IOException {
		BufferedReader br = new BufferedReader(new FileReader(inputFile));
		StringBuilder text = new StringBuilder();
        String line = "";
        try{
        	line = br.readLine();
	
        	while(line != null){
        		text.append(line);
        		text.append("\n"); //added this line to fix comparision problem
        		line = br.readLine();
        	}
        }catch (Exception e){
        	e.printStackTrace();
        }finally{
        	br.close();
        	}
        return text.toString();
	}
	
}
