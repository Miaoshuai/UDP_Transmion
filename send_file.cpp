/*======================================================
    > File Name: send_file.cpp
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2016年01月09日 星期六 13时44分41秒
 =======================================================*/
/*接受文件类的实现*/

#include "send_file.h"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/type.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <random>
#include <thread>
#include <fcntl.h>
#include <ctime>
#include <functional>
#include <memory>
#include <sys/eventfd.h>

SendFile::SendFile()
    eventFd_(eventfd(0,0))
{
       
}

SendFile::~SendFile()
{

}

SendFile::startSendFile(std::string &filename)
{
    
}


std::string SendFile::getBroadCastAddress(void)
{
   ifaddrs *ifAddrStruct = NULL;
    
    char addressBuffer[100];
    std::string interfaceName1 = "wlan0";
    std::string interfaceName2 = "eth0";
    void *tmpAddressPtr = NULL;

    
    //获取ifconfig相关信息的结构体
    getifaddrs(&ifAddrStruct);

    //从链表中获取广播地址
    while(ifAddrStruct != NULL)
    {
        
        //cout<<ifAddrStruct->ifa_name<<endl;
        if((strcmp(ifAddrStruct->ifa_name,interfaceName1.c_str()) == 0) || (strcmp(ifAddrStruct->ifa_name,interfaceName2.c_str()) == 0)) 
        {
            
            tmpAddressPtr = &((struct sockaddr_in *)(ifAddrStruct->ifa_ifu.ifu_broadaddr))->sin_addr;
            inet_ntop(AF_INET,tmpAddressPtr,addressBuffer,INET_ADDRSTRLEN);
            
            if(addressBuffer[0] == '1' && addressBuffer[1] == '9' && addressBuffer[2] == '2')
            {
                return addressBuffer;
            }
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
        
    return NULL; 
}

std::string SendFile::extractFileName(std::string filename)
{
    int pos = filename.find_last_of('/');

    return std::string(filename.substr(pos + 1));
}


void SendFile::waitRecipientReply(void)
{
    char recvBuf[BUFFER_SIZE];
    int ret;
    socklen_t len = sizeof(recvAddress_);
    uint64_t buffer;
    while(status == UNCONNECT)
    {
        ret = recvfrom(udpFd_,recvBuf,BUFFER_SIZE - 1,0,(sockaddr *)&recvAddress_,&len);
        assert(ret > 0);
        cout<<inet_ntoa(recvAddress_.sin_addr)<<endl;
        recvBuf[ret] = '\0';
        identifyCode_ = recvBuf;
        status = CONNECT;
        ret = read(eventFd_,&buffer,sizeof(buffer));
        assert(ret == sizeof(buffer));      
        bzero(recvBuf,sizeof(recvBuf));
    }

}


void SendFile::sendBroadcast(void)
{
    uint64_t buffer = 1;
    int ret;
    std::unique_ptr<UdpDataPacket> dataPtr;
    std::thread recvThread(std::bind(&SendFile::waitRecipientReply,this));
    
    std::string randomCode;
    randomCode = generateVerificationCode(); 
    std::count<<"验证码为:"<<randomCode<<std::endl;

    while(1)
    {
        //构建广播包
        dataPtr.reset(makeUdpDataPacket(0,SEND_UDP,"hello"));
        //发送广播包
        sendto(udpFd_,dataPtr.get(),sizeof(UdpDataPacket),0,(sockaddr *)&udpAddress_,sizeof(udpAddress_));
        checkConnect(randomCode);
        if(status == CONNECT)
        {
            close(eventFd_);
            break;
        }
        
        ret = write(eventFd_,&buffer,sizeof(buffer));
        assert(ret == sizeof(buffer));
        usleep(AMOUT);      
    }
          
}

void SendFile::checkConnect(const std::string &randomCode)
{
    if(status == CONNECT)
    {
        std::cout<<"hello"<<std::endl;
        if(identifyCode_ == randomCode)
        {
            std::cout<<"连接已确认，现在开始传输文件!\n"<<std::endl;
            std::unique_ptr<UdpDataPacket> dataPtr(makeUdpDataPacket(0,SEND_CORRECT,"\0"));
            sendto(udpFd_,dataPtr.get(),sizeof(UdpDataPacket),0,(sockaddr *)&recvAddress_,sizeof(recvAddress_));           
        }
        else
        {
            status = UNCONNECT;

            //构建连接不成功的数据包
            std::unique_ptr<UdpDataPacket> dataPtr(makeUdpDataPacket(0,SEND_UNCORRECT,"\0"));
            //发送不成功包
            sendto(udpFd_,dataPtr.get(),sizeof(UdpDataPacket),(sockaddr *)recvAddress_,sizeof(recvAddress_));
        }
    }
}

UdpDataPacket *SendFile::makeUdpDataPacket(int number,int type,char *data)
{
    UdpDataPacket *p = new UdpDataPacket;
    p->packetNumber = number;
    p->packetType = type;
    strcpy(p->packetData,data);

    return pu;
}

std::string generateVerificationCode(void)
{
    std::uniform_int_distribution<unsigned> u(0,9);
    std::default_random_engine e(std::time(0));
    
    std::string tempStr;
    for(int i=0; i < 5; i++)
    {
        tempStr.push_back(u(e) + '0');
    }
    identifyCode_ = tempStr;
    
    return tempStr;   
}
