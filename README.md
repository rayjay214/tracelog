# Traceing Framework
A lightweight framework help to trace desired data flow for all kinds of services

# Background
In the architechture of microservices, we got lots of services to maintain. If we have a bug on one of the services, in the tradiontional way, we should first check our maintenance system（eg. CMDB） to find out the service deployment. Then ssh to all theses machines. Finally analyze the log files on each machine to figure out the problems.In most cases, only specific id(user, device, location...) will trigger the bug. So we have to filter out the id specific logs from the whole log files. This is exactly the way how I treat a bug before.<br>

With the support of tracelog framework, we can achieve the above goals on just one web page. For example, assume we have a user login service named LOGIN_SERVICE deployed at four machines, 192.168.1.100:12345, 192.168.1.101:12345, 192.168.1.102:12345, 192.168.1.103:12345. In order to trace the login behavior of user with id 888, we need to the following.
* login the web page, if success, it will return a session_id
* set the config with service_name=LOGIN_SERVICE, p1=1, p2=888 (p1, p2, p3 are user defined params, it is illustrated in the examples)
* keep the web page on and wait for the outputs (it will internally fetch the latest logs at specified intervals)

Of course, you have to do some extra work for your user login service. 1. Create a tracelog thread (which can be extended from the base trace client thread) and keep it running. 2. Add an extra line in you code where your orignal log print code goes. For example, your log print line look like this : Log("user %d login at %s", uid, time). Now add one line TRACE_IF(g_pTracer, (const char*)&uid, "user %d logined at %s", uid, time). You could check the details in the example.<br>

Other from the above functionality，it could also do some other tasks such as,
* Print global variable like some statistics.
* Excute some commands such as sync data from other service.
* ...

# Architechture
![image](https://raw.githubusercontent.com/rayjay214/tracelog/master/images/architechture.png)
In the picture, we can see that the tracelog framework consists of three modules, Trace Server, Trace Cgi and Trace Client. Trace Server maintains configs for each session set by Trace Cgi and logs uploaded by each Trace Client. Trace Cgi is an async fastcgi deployed along with nginx, which can accept http request, parse the params and send it to Trace Server. Trace Client is deployed with your own programs, which at its start time should send self info to Trace Server (send it again if server restarts). Whenever someone set configs for ServiceA, Trace Client embeded in ServiceA will receive the configs, afterwards, if condition meets, the client will upload logs to server. Then Trace Cgi can get the logs uploaded by Trace Client.

# Usage
You can either deploy your own server, cgi and web pages or use mine for experiment. Let's talk about the easier way first, use my server, cgi and web pages (simple and crude for the moment).<br>

## C++ Client
[check the example client code](/trace_client/cpp)
* Add TraceClientThread.cpp and trace.pb.cc into your makefiles or SConstruct(which I use) and link liblog4cxx.so, libprotobuf.so, libnframe.so.
* Create a new Class extended from TraceClientThread and overwrite the check and setConfig method to implement your own logic.
* Start the TraceClientThread and connect it to my Trace Server, then report your service name (assumed TEST_SERVICE) and your ip and port. Keep the thread running all the time.
* Open the url [trace.gmiot.net](http://trace.gmiot.net) and login with 123456.
* Hit the setting tab and set the config like this.
![image](https://raw.githubusercontent.com/rayjay214/tracelog/master/images/set_config.png)
* Wait for the logs to show.

## Python Client
[check the example client code](/trace_client/python)
* Same steps as C++ Client


