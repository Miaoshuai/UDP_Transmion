/*======================================================
    > File Name: send_file.cpp
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2016年01月09日 星期六 13时44分41秒
 =======================================================*/
/*接受文件类的实现*/

#include "send_file.h"
#include "tools.h"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/type.h>
#include <stdlib.h>
#include <random>
#include <thread>
#include <fcntl.h>
#include <ctime>
#include <functional>
#include <memory>
#include <sys/eventfd.h>

SendFile::SendFile()
    :eventFd_(eventfd(0,0))
{
    //生成广播地址
    makeSockAddrss(udpAddress_,getBroadCastAddress().c_str(),atoi(UDP_PORT));
    //生成本地地址
    makeSockAddrss(localAddress_,getLocalIpAddress().c_str(),atoi(LOCAL_PORT));
    //配置udpFd_套接字
    configureUdpFd();     
}

SendFile::~SendFile()
{

}

SendFile::startSendFile(std::string &filename)
{
    //发送广播
    sendBroadcast();
    //给接收端发送所传的文件名
    std::string fileName = extractFileName(filename);
    //构建起始包
    std::unique_ptr<UdpDataPacket> startPtr = makeUdpDataPacket(0,SEND_START,fileName.c_str());
    sendto(udpFd_,startPtr.get(),sizeof(UdpDataPacket),0,(sockaddr *)&recvAddress_,sizeof(recvAddress_));
    
    //获取所发送文件的套接字描述符
    int fd = open(filename.c_str(),O_RDONLY);
    assert(fd > 0);
    std::cout<<"开始发送文件"<<std::endl;
    int i = 0;
    int j = 0;
    int sum  = 0;
    int ret;
    //构建数据包
    std::unique_ptr<UdpDataPacket> dataPtr = makeUdpDataPacket(++i,SEND_DATA,"\0");
    //从文件中读取数据并发送给接收端
    while((ret = read(fd,dataPtr->packetData,BUFFER_SIZE -1)) > 0)
    {
        std::cout"包数:"<<++j<<"   "<<"ret:"<<ret<<std::endl;
        sum += ret;
        dataPtr->dataLength = ret;
        sendto(udpFd_,dataPtr.get(),sizeof(UdpDataPacket),0,(sockaddr *)&recvAddress_,sizeof(recvAddress_));
        dataPtr->packetNumber = ++i;
    }
    std::cout<<"sum = "<<sum<<std::endl;
    std::cout<<"包总数 = "<<j<<std::endl;
    //构建终止包
    std::unique_ptr<UdpDataPacket> endPtr = makeUdpDataPacket(++j,SEND_END,"\0");
    //发送终止包
    sleep(5);
    sendto(udpFd_,endPtr.get(),sizeof(UdpDataPacket),0,(sockaddr *)&recvAddress_,sizeof(recvAddress_));
    close(fd);
        
}


void SendFile::waitRecipientReply(void)
{
    char recvBuf[BUFFER_SIZE];
    int ret;
    socklen_t len = sizeof(recvAddress_);
    uint64_t buffer;
    //如果未验证成功则循环
    while(status == UNCONNECT)
    {
        //接受验证码
        ret = recvfrom(udpFd_,recvBuf,BUFFER_SIZE - 1,0,(sockaddr *)&recvAddress_,&len);
        assert(ret > 0);
        cout<<inet_ntoa(recvAddress_.sin_addr)<<endl;
        recvBuf[ret] = '\0';
        identifyCode_ = recvBuf;
        status = CONNECT;
        //等待主线程确认验证码完毕通知
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
    //用于接受客户端验证码的线程
    std::thread recvThread(std::bind(&SendFile::waitRecipientReply,this));
    
    std::string randomCode;
    //生成验证码
    randomCode = generateVerificationCode(); 
    std::count<<"验证码为:"<<randomCode<<std::endl;

    while(1)
    {
        //构建广播包
        dataPtr.reset(makeUdpDataPacket(0,SEND_UDP,"hello"));
        //发送广播包
        sendto(udpFd_,dataPtr.get(),sizeof(UdpDataPacket),0,(sockaddr *)&udpAddress_,sizeof(udpAddress_));
        //检查连接
        checkConnect(randomCode);
        if(status == CONNECT)
        {
            close(eventFd_);
            break;
        }
        //通知接受线程继续       
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
    p->dataLength = strlen(data);
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

void SendFile::makeSockAddrss(const sockaddr_in &sockaddr,char *address,int port)
{
    bzero(&sockaddr,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET,address,&sockaddr.sin_addr);
    sockaddr.sin_port = htons(port);
}


void SendFile::configureUdpFd(void)
{
    //获得udp套接字
    udpFd_ = socket(AF_INET,SOCK_DGRAM,0);

    //设置端口重用
    int val = 1;
    if(setsockopt(udpFd_.SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int)) < 0)
    {
        std::cout<<"重置失败!\n"<<std::endl;
    }

    int ret = bind(udpFd_,(sockaddr *)&localAddress_,sizeof(localAddress_));
    assert(ret != -1);

    //将udpFd_设为广播
    int on = 1;
    if((setsockopt(udpFd_,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on))) == -1)
    {
        std::cout<<"设置失败!\n"<<std::endl;
    }
}
