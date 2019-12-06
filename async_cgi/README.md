# Introduction
The async_cgi is a async fastcgi using netframe library. It is deployed along with nginx, communicating with each other througth
fastcgi protocal. The cgicc library is used to parse the http request. The most fastcgi framework we ecountered (including the
examples we google with the key word fastcgi) works in the synchronous way. The process may like this, the user send http
request, intercepted by nginx and sent to fastcgi, fastcgi parse the request, send it to the backend service and then wait for
the result in a blocked way. When the result is returned by backend service, fastcgi build the response and send it back to
nginx. My async_cgi differs from the common fastcgi framework in the way that it sends the parsed request to the backend 
service and returns. The processed result will be received by async_cgi and the corresponding connection with nginx could be
found by checking some fields of the message. Then the reponse could be sent back to nginx, then transferred to browser.<br>
The async_cgi is used in the tracelog framework to interact with user behaviours, sending the configs to the trace server as 
well as fetching the generated logs from trace server for users.

# Requirements
* Log4cxx
* Boost C++ Library  
* Protobuf

# Install
```
yum install scons -y
scons
```

# Usage
Place the cgi.ini and log4cxx.cgi in the same directory as the compiled binary file.<br>
```nohup trace_async_cgi &```

# Test




