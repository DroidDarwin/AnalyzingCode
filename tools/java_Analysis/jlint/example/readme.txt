This is an example of running jlint on a file. The purpose of this is to ensure that all jlint preconditions have been installed on the system.


usage & expected output

notroot@ubuntu:~/shared/ProjectKrutz/tools/java_Analysis/jlint$ pwd
/home/notroot/shared/ProjectKrutz/tools/java_Analysis/jlint
notroot@ubuntu:~/shared/ProjectKrutz/tools/java_Analysis/jlint$ ./jlint example/*
example/org/apache/http/auth/AuthScope.java:1: Zero operand for + operation.
example/org/apache/http/auth/AuthScope.java:2: Compare strings as object references.
example/org/apache/http/auth/AuthScope.java:3: Compare strings as object references.
example/org/apache/http/auth/AuthScope.java:4: Compare strings as object references.
example/org/apache/http/auth/AuthScope.java:5: Compare strings as object references.
File 'example/readme.txt' isn't correct Java class file
example/org/apache/http/auth/InvalidCredentialsException.java:6: Component 'serialVersionUID' in class 'org/apache/http/auth/InvalidCredentialsException' shadows one in base class 'org/apache/http/auth/AuthenticationException'.
example/org/apache/http/auth/ContextAwareAuthScheme.java:7: Method org/apache/http/auth/AuthScheme.authenticate(org.apache.http.auth.Credentials, org.apache.http.HttpRequest) is not overridden by method with the same name of derived class 'org/apache/http/auth/ContextAwareAuthScheme'.
java/lang/Object.java:8: equals() was overridden but not hashCode().
Verification completed: 8 reported messages.
notroot@ubuntu:~/shared/ProjectKrutz/tools/java_Analysis/jlint$ 



