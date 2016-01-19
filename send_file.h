/*======================================================
    > File Name: send_file.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2016年01月09日 星期六 13时12分38秒
 =======================================================*/
/*定义发送文件类的头文件*/

#pragma once

#include "package.h"
#include <string>

class SendFile
{
    public:
        SendFile();
        ~SendFile();

        void startSendFile(std::string &filename);        //开始发送文件

    private:
        std::string getBroadCastAddress(void);                  //获取文件广播地址
        std::string extractFileName(std::string filename);      //将文件名从绝对路径中提取出来
        void sendBroadcast(void);                               //持续发送广播
        std::string generateVerificationCode(void);             //随即生成验证码
        void waitRecipientReply(void);                          //线程函数
        
        std::string addressBroadcast_;                          //广播ip地址
        int         udpPort_;                                   //udp端口号
        int         localPort_;                                 //本地端口
        int         udpFd_;                                     //udp套接字描述符
        sockaddr_in udpAddress_;                                //udp地址
        sockaddr_in localAddress_;                              //本地地址
        sockaddr_in recvAddress_;                               //接收端地址
        std::string identifyCode_;                              //验证码
};
