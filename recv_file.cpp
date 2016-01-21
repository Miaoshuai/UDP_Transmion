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
            ret = recvfrom(recvFd_,udpPtr.get(),);
        }while();
    }
}
