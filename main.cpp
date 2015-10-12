/*************************************************************************
    > File Name: main.cpp
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月12日 星期一 21时13分03秒
 ************************************************************************/
#include"udp_transmision.h"
#include<iostream>

using std::cout;
using std::cin;
using std::endl;

int main(int argc,char **argv)
{
	
	cout<<"请选择："<<endl;
	cout<<"1.发送文件"<<endl;
	cout<<"2.接收文件"<<endl;
	
	int x;
	if(argc != 2)
	{
		cout<<"您输入的参数数量有误"<<endl;
	}


	cin>>x;
	switch(x)
	{
		case 1:
			{
 				SendFile sf;  
				sf.StartSendFile(argv[1]);
			 	break; 
			}
		case 2:
			{
  		 	 	RecvFile  rf; 
				rf.StartRecvFile();break;
			}
	}
	
	return 0;
}
