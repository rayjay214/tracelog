# Tracelog Framework
A lightweight framework help to trace desired data flow for all kinds of services

# Background
In the architechture of microservices, we got lots of services to maintain. If we have a bug on one of the services, in the tradiontional way, we should first check our maintenance system（eg. CMDB） to find out the service deployment. Then ssh to all theses machines. Finally analyze the log files on each machine to figure out the problems.In most cases, only specific id(user, device, location...) will trigger the bug. So we have to filter out the id specific logs from the whole log files. This is exactly the way how I treat a bug before.<br><br>
With the support of tracelog framework, we can achieve the above goals on just one web page. For example, assume we have a user login service named LOGIN_SERVICE deployed at four machines, 192.168.1.100:12345, 192.168.1.101:12345, 192.168.1.102:12345, 192.168.1.103:12345. In order to trace the login behavior of user with id 888, we need to the following.
* login the web page, if success, it will return a session_id
* set the config with service_name=LOGIN_SERVICE, p1=1, p2=888 (p1, p2, p3 are user defined params, it is illustrated in the examples)
* keep the web page on and wait for the outputs (it will internally fetch the latest logs at specified intervals)
