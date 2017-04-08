# Ubuntu 16.04 部署hadoop 2.7.3 单机版 心得体会
## 写在前面
本来不准备在hadoop的运维部署上花太多时间，可是就是没忍住，了解运维，搭建实际运维环境，有助于让我们理解hadoop的组成结构，清楚程序的定位，更能够理解hadoop的应用场景，我比较认可hadoop重在业务的思路，如何能够在实际生产环境中将hadoop用好，业务场景才是最重要的。

## 准备工作
hadoop已经发展了这么多年，从0.20.x开始，经历了1.0.1、1.2.1等多个版本，网上的配置心得也随着其版本的演进各家都不一样，由于只是学习，自然选择了最新的稳定版2.7.3
ubuntu 16.04无疑也成了我虚拟机的选择
jdk 选择了1.8.0_121

首先规划一下目录结构和用户环境
由于我只是尝试一下hadoop，故我选择将hadoop安装在、/opt下面，账户就用我现成的，但是需要为hadoop规划一个用户组，必要的时候还是扩展出其他账户分享给别人使用
jdk是早就安装好了在/usr/lib/java/jdk1.8.0_121

## 正式开始
### 创建用户组，并与账户关联
1. groupadd hadoop
2. usermod -a -G hadoop xxx
3. 将hadoop组加入到 /etc/sudoers

### 安装hadoop
进入当前用户根目录 cd ~

下载[hadoop 2.7.3](http://www.apache.org/dyn/closer.cgi/hadoop/common/hadoop-2.7.3/hadoop-2.7.3.tar.gz)
可以使用wget http://www.apache.org/dyn/closer.cgi/hadoop/common/hadoop-2.7.3/hadoop-2.7.3.tar.gz

解压  tar zxvf hadoop-2.7.3.tar.gz
解压后目录为 hadoop-2.7.3

将此目录移动到/opt下
mv hadoop-2.7.3 /opt/hadoop-2.7.3

更改/opt/hadoop-2.7.3的用户属性 user为准备的hadoop用户名
chown -R **user**:hadoop /opt/hadoop-2.7.3

### 编辑全局环境变量
修改/etc/profile文件，插入JAVA、hadoop的环境参数，具体路径根据实际情况修改
export JAVA_HOME=/usr/lib/java/jdk1.8.0_121
export JRE_HOME=/usr/lib/java/jdk1.8.0_121/jre
export HADOOP_HOME=/opt/hadoop-2.7.3
export PATH=.:**$JAVA_HOME/bin:$HADOOP_HOME/bin:$HADOOP_HOME/sbin**:$PATH

### 测试环境变量是否生效
$java -version

java version "1.8.0_121"
Java(TM) SE Runtime Environment (build 1.8.0_121-b13)
Java HotSpot(TM) 64-Bit Server VM (build 25.121-b13, mixed mode)

$hadoop version

Hadoop 2.7.3
Subversion https://git-wip-us.apache.org/repos/asf/hadoop.git -r baa91f7c6bc9cb92be5982de4719c1c8af91ccff
Compiled by root on 2016-08-18T01:41Z
Compiled with protoc 2.5.0
From source with checksum 2e4ce5f957ea4db193bce3734ff29ff4
This command was run using /opt/hadoop-2.7.3/share/hadoop/common/hadoop-common-2.7.3.jar

### 伪分布式配置
这里是最容易出错的地方，由于各个版本配置文件中key是会变化的，根据经验没有使用网上提供的各种资料，直奔2.7.3的doc目录
/opt/hadoop-2.7.3/share/doc/hadoop/hadoop-project-dist/
在这里你可以找到所有你需要的配置文件的默认版本，也可以学习到每个2.7.3中所有配置参数的实际含义描述

修改/opt/hadoop-2.7.3/etc/hadoop/hadoop-env.sh
在25行加入
```
export JAVA_HOME=/usr/lib/java/jdk1.8.0_121
```

修改/opt/hadoop-2.7.3/etc/hadoop/core-site.xml

加入
```
<property>
	<name>fs.defaultFS</name>
	<value>hdfs://master:9000</value>
</property>
<property>
	<name>hadoop.tmp.dir</name>
	<value>/opt/hadoop-2.7.3/tmp</value>
</property>
```
**这里要强调一下，不能保留例子中的file:// 必须写成路径，否则在其他地方引用到hadoop.tmp.dir的时候就会抛权限错误，我也是奇怪官方例子为什么会出这样的错误**

修改/opt/hadoop-2.7.3/etc/hadoop/hdfs-site.xml 

加入
```
<property>
	<name>dfs.replication</name>
	<value>1</value>
</property>
 ```
 
修改/opt/hadoop-2.7.3/etc/hadoop/mapred-site.xml 

加入
```
<property>
	<name>mapreduce.framework.name</name>
	<value>yarn</value>
</property>
```

修改/opt/hadoop-2.7.3/etc/hadoop/yarn-site.xml

加入
```
<property>
	<name>yarn.resourcemanager.hostname</name>
	<value>master</value>
</property>
<property>
	<name>yarn.nodemanager.aux-services</name>
	<value>mapreduce_shuffle</value>
</property>
```

### 格式化namenode
hdfs namenode -format

### 启动hadoop
start-all.sh

### 验证安装状态,需要以下服务都正常启动才算正常
```
jay@master:~$ jps
4256 SecondaryNameNode
3364 ResourceManager
5333 Jps
4489 NodeManager
3934 NameNode
4078 DataNode
```
也可登录 http://ip:8088/cluster 查看hadoop状态


