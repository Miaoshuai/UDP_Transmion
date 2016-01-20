/*======================================================
    > File Name: tools.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2016年01月20日 星期三 14时35分20秒
 =======================================================*/
#pragma once

#include <string>

//获取广播地址
std::string getBroadcastAddress(void);

//获取本地IP地址
std::string getLocalIpAddress(void);

//将文件名从文件的绝对路径中提取出来
std::string extractFilename(std::string filename);

