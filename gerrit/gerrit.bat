set JAVA_HOME=C:\Program Files\Java\jdk1.7.0_79
set PATH=%JAVA_HOME%\bin;%PATH%
java -version

d:
cd \gerrit\bin
java -Xms1024m -Xmx1024m -jar d:\gerrit\bin\gerrit.war daemon -d d:\gerrit