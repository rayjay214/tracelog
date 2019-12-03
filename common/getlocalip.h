#ifndef _GM_GET_LOGAL_IP_H_
#define _GM_GET_LOGAL_IP_H_

#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <net/if.h> 
#include <ifaddrs.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string>
#include <vector>

int get_local_ip_v4_and_v6(int family, std::vector<std::string> &ips);

/*从网卡读取本机的一个ip地址，以192开头的ip优先，且忽略127开头的ip*/
inline int GetOneSelfIpAddress (std::string &SelfIpStr , int family=AF_INET) {
    struct ifaddrs * ifAddrStruct=NULL,*ifAddrStruct0=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);
    ifAddrStruct0 = ifAddrStruct;
    while (ifAddrStruct!=NULL) 
    {
        if(ifAddrStruct->ifa_addr!=NULL)
        {
            if (ifAddrStruct->ifa_addr->sa_family==AF_INET && family==AF_INET) 
            {
                // is a valid IP4 Address
                tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                if(strncmp("127.",addressBuffer,4) != 0)
                {
                    if(strncmp(SelfIpStr.c_str(),"192.",4) != 0)
                    {
                        SelfIpStr = addressBuffer;
                    }
                }
            } 
            else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6 && family==AF_INET6) 
            {
                // is a valid IP6 Address
                tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                SelfIpStr = addressBuffer;
            } 
        }
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    freeifaddrs(ifAddrStruct0);
    return 0;
}

inline int GetOneSelfPublicIpAddress (std::string &SelfIpStr , int family=AF_INET) {
    struct ifaddrs * ifAddrStruct=NULL,*ifAddrStruct0=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);
    ifAddrStruct0 = ifAddrStruct;
    while (ifAddrStruct!=NULL) 
    {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET && family==AF_INET) 
        {
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strncmp("127.",addressBuffer,4) != 0)
            {            
                if (SelfIpStr.empty() || strncmp(SelfIpStr.c_str(),"192.",4) == 0)
                {
                    SelfIpStr = addressBuffer;
                }
            }
        } 
        else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6 && family==AF_INET6) 
        {
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            SelfIpStr = addressBuffer;
        } 
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    freeifaddrs(ifAddrStruct0);
    return 0;
}


#endif

