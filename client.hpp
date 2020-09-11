//客户端

#pragma once
#include<thread>
#include "util.hpp"

class Host
{
public:
	uint32_t _ip_addr; //要配对的主机IP地址
	bool _pair_ret; //存放配对结果，配对成功则返回true
};

class Client
{
public://获取在线主机
	//主机配对的线程入口函数
	void HostPair(Host* host)
	{

	}
	bool GetOnlineHost()
	{
		//1,获取网卡信息,进而得到局域网中所有的IP地址列表
		std::vector<Adapter> List;//List中储存网卡信息
		AdapterUtil::GetAllAdapter(&List);
		//获取所有主机地址，并添加到host_list当中
		std::vector<Host> host_list;
		for (int i = 0; i < List.size(); i++)
		{
			uint32_t ip = List[i]._ip_addr;
			uint32_t mask = List[i]._mask_addr;
			uint32_t net = (ntohs(ip & mask));//计算网络号
			uint32_t max_host = (~ntohs(mask));//计算最大主机号
			std::vector<std::thread*> thr_list(max_host);
			std::vector<bool> ret_list(max_host);
			for (int j = 1; j < max_host; j++)
			{
				uint32_t host_ip = net + j; //这个主机IP的计算应该使用主机字节序的网络号和主机号

				//2,逐个对IP地址列表中的主机发送配对请求
				//thr_list[i] = new std::thread(&Client::HostPair, this, host_ip);
				Host host; 
				host._ip_addr = host_ip; 
				host._pair_ret = false;
				host_list.push_back(host);
			}
			//for (int j = 1; j < max_host; j++)
			//{
			//	thr_list[i]->join();//等待线程退出,线程退出,主机配对却是不一定成功的
			//	delete thr_list[i];
			//}
		}
		//对host-list中的主机创建线程进行配对
		std::vector<std::thread*> thr_list(host_list. size());
		for (int i = 0; i < host_list.size(); i++)
		{
			thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
		}
		//等待所有线程主机配对完毕,判断配对结果,将在线主机添加到online host中
		for (int i = 0 ; i < host_list.size(); i++)
		{
			thr_list[i]->join();
			if (host_list[i]._pair_ret == true)
			{
				_online_host.push_back(host_list[i]);
				delete thr_list[i];
			}
		}
		
		//3,若配对请求得到响应,则对应主机位在线主机,则将IP添加到_onlinehost列表中

		//4.打印在线主机列表，使用户选择

	}
	//获取文件列表
	bool GetSharelist ();
		//下载文件
	bool DonwloadFile();
private:
	std::vector<Host>  _online_host;

};
