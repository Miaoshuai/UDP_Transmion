/*======================================================
    > File Name: recvfile.cpp
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2016年01月20日 星期三 20时04分03秒
 =======================================================*/
#include "recv_file.h"
#include "package.h"
#include "tools.h"

#include <string>
#include <iostream>
#include <memory>
#include <assert.h>

RecvFile::RecvFile()
{
    //生成接受地址
    makeSockAddress(recvAddress_,"0.0.0.0",atoi(UDP_PORT));
    //创建接受套接字
    recvFd_ = socket(AF_INET,SOCK_DGRAM,0);
    //绑定套接字
    int ret;
    ret = bind(recvFd_,(sockaddr *)&recvAddress_,sizeof(recvAddress_));
    assert(ret != -1);
}


RecvFile::~RecvFile()
{

}

void RecvFile::startRecvFile(void)
{
    //获得连接
    getConnection();
    //获得套接字描述符
    int fd = getFilFd();
    //设置接受套接字
    setRecvFd(1024*1024*5);

    //开始接受文件
    int ret;
    int i = 0;
    int j = 0;
    int sum = 0;
    UdpDataPack data;
    bzero(&data,sizeof(data));
    socket_t = len = sizeof(sendAddress_);
    //循环读取数据
    while((ret = recvfrom(recvFd_,&data,sizeof(data),0,(sockaddr *)&sendAddress_,&len)))
    {
        sum += ret;
        j++;
        std::cout<<"包数:"<<j;"     "<<"ret:"<<ret<<"包号:"<<data.packetNumber<<std::endl;
        //判断数据包是否是终止包，若是则退出循环
        if(data.packetType == SEND_END)
        {
            std::cout<<"文件接受完毕"<<std::endl;
            break;
        }
        //如果是数据包，则将其写入文件，或存入set集合
        if(data.packetType == SEND_DATA)
        {
            if(data.packNumber == (i + 1))
            {
                ret = write(fd,data.packetData,data.dataLength);
                assert(ret >= 0);
                i++;
            }
            else if(!recvDataSet_.empty())
            {
                checkSetData(i);
            }
            else
            {
                recvDataSet_.insert(data);
            }
        }
        bzero(&data,sizeof(data));
    }
    std::cout<<"sum = "<<sum<<std::endl;
    std::cout<<"包总数:"<<j<<std::endl;
    close(fd);
    close(recvFd_);
}

void RecvFile::getConnection(void)
{
    std::unique_ptr<UdpDataPack> udpPtr = makeUdpDataPacket(-1,SEND_DATA,"\0");
    ret = recvfrom(recvFd_,udpPtr.get(),sizeof(UdpDataPack),0,(sockaddr *)&sendAddress_,&len);

    std::string identifyCode;
    std::cout<<"请输入验证码"<<std::endl;
    while(1)
    {
        std::cin>>identifyCode;
        sendto(recvFd_,identifyCode.c_str(),identifyCode.length(),0,(sockaddr *)&sendAddress_,sizeof(sendAddress_));
        //先将滞留在接受缓冲区中的广播数据读完
        do
        {
            ret = recvfrom(recvFd_,udpPtr.get(),sizeof(UdpDataPack),0,(sockaddr *)&sendAddress_,&sizeof(sendAddress_));
            assert(ret <= 0);
        }while(udpPtr->packetType == SEND_UDP);
        
        if(udpPtr->packetType == SEND_CORRECT)
        {
            std::cout<<"连接建立成功"<<std::endl;
            break;
        }
        std::cout<<"验证码错误，请重新输入:"<<std::endl;
    }
}


int  getFilFd(void)
{
    std::unique_ptr<UdpDataPack> dataPtr = makeUdpDataPacket(0,SEND_DATA,"/0");
    int ret = recvfrom(recvFd_,dataPtr.get(),sizeof(UdpDataPack),0,(sockaddr *)&sendAddress_,&sizeof(sendAddress_));

    std::string filename("/tmp/");
    filename += dataPtr->packetData;
    
    //打开并创建与上面收到文件名一样的文件
    int fd;
    fd = open(filename.c_str,O_WRONLY | O_CREAT | O_EXCL,S_IRUSR,S_IWUSR,S_IXUSR)
    assert(fd > 0);

    return fd;
}


void setRecvFd(int on)
{
    unsigned length = sizeof(on);
    
    int ret = setsockopt(recvFd_,SOL_SOCKET,SO_RCVBUF,&on,length);
    assert(ret != -1);
}


void checkSetData(int &n)
{
    for(auto it : recvDataSet_)
    {
        if(it.packetNumber == (n + 1))
        {
            ret = write(fd,data.packetData,data.dataLength);
            assert(ret >= 0);
            n++;
            recvDataSet_.erase(it);
            continue;
        }
        else
        {
            break;
        }
    }
}
