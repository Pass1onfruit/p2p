//客户端

#pragma once
#include<thread>
#include "util.hpp"
#include"httplib2.h"
#include <boost/filesystem.hpp>
//#include<unordered_map>

#define P2P_PORT 9000
#define MAX_IPBUFFER 16
#define SHARED_PATH "./Shared/"
#define DOWNLOAD_PATH "./Download/"
//1左移10位是1K，左移20位是1M
//#define MAX_RANGE 100 << 20
#define MAX_RANGE (5)


class Host
{
public:
	uint32_t _ip_addr; //要配对的主机IP地址
	bool _pair_ret; //存放配对结果，配对成功则返回true
};

class _Client
{
public:
	bool Start() 
	{
		//客户端程序需要循环运行,因为下载文件不是只下载一次
		//循环运行每次下载一个文件之后都会重新进行主机配对一不合理
		while (1)
		{
			GetOnlineHost();
		}
		return true;
	}
	//获取在线主机
	//主机配对的线程入口函数
	void HostPair(Host* host)
	{ //使用第三方库httplib 
		//1,组织http协议格式的请求数据
		//2,搭建一个tcp的客户端,将数据发送
		//3,等待服务器端的回复,并进行解析
		char buf[MAX_IPBUFFER] = { 0 };
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);//将网络字节序的IP地址转换为字符串点分十进制IP地址
		//inet_ntop(地址域，网络字节序IP地址，缓冲区 接受转换的字符串，缓冲区大小)

		httplib::Client cli(buf, P2P_PORT);    //实例化httplib客户端对象，连接建立失败Get会返回NLL
		auto rsp = cli.Get("/hostpair");  //向服务端发送资源为/hostpair的GET请求
		if (rsp && rsp->status == 20)//判断响应结果是否正确
		{
			host->_pair_ret = true;//重置主机配对结果
		}
		return;
	}
	bool GetOnlineHost()
	{
		char ch = 'Y'; //是否重新匹配,默认是进行匹配的,若已经匹配过, online主机不为空则让用户选择
		if (!_online_host.empty())
		{
			std::cout << "是否重,查看在线主机(Y/N) : ";
			fflush(stdout);
			std::cin >> ch;
		}
		if (ch == 'Y')
		{
			//1. 获取网卡信息,进而得到局域网中所有的IP地址列表
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

				//std::vector<std::thread*> thr_list(max_host);
				//std::vector<bool> ret_list(max_host);
				for (int j = 1; j < (int32_t)max_host; j++)
				{
					uint32_t host_ip = net + j; //这个主机IP的计算应该使用主机字节序的网络号和主机号
					//2,逐个对IP地址列表中的主机发送配对请求
					//thr_list[i] = new std::thread(&Client::HostPair, this, host_ip);
					Host host;
					host._ip_addr = htonl(host_ip); //将最高主机字节序的IP地址转换为网络字节序
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
			std::vector<std::thread*> thr_list(host_list.size());
			for (int i = 0; i < host_list.size(); i++)
			{
				thr_list[i] = new std::thread(&_Client::HostPair, this, &host_list[i]);
			}
			//等待所有线程主机配对完毕,判断配对结果,将在线主机添加到online host中
			for (int i = 0; i < host_list.size(); i++)
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
				{
					_online_host.push_back(host_list[i]);
					delete thr_list[i];
				}
			}
			return true;
		}
		//将所有在线主机的IP打印出来，供用户选择
		for (int i = 0; i < _online_host.size(); i++)
		{
			char buf[MAX_IPBUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
			std::cout << "\t" << buf << std::endl;
		}

		//3.若配对请求得到响应,则对应主机位在线主机,则将IP添加到_onlinehost列表中

		//4.打印在线主机列表，使用户选择
		std::cout << "请选择配对机，获取共享文件列表: ";
		fflush (stdout);
		std::string select_ip;
		std::cin >> select_ip;
		GetSharelist(select_ip); //用户选择主机之后,调用获取文件列表接口
		//GetSharelist(select_ip);
		return true;
	}
	//获取文件列表
	bool GetSharelist(const std::string &host_ip)
	{
		//向服务端发送一个文件列表获取请求
		//1,先发送请求
		//2,得到响应之后,解析正文(文件名称)
		httplib::Client cli (host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Get("/list");
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "获取文件列表响应错误\n";
			return false; 
			//打印正文------打印服务端响应的文件名称列表供用户选择
			std::cout << rsp->body << std::endl; 
			std::cout << "\n请选择要下载的文件";
			fflush(stdout);
			std::string filename; 
			std::cin >> filename;
			DonwloadFile(host_ip, filename);

			return true;
		}
		return true;
	}

	//下载文件
	bool DonwloadFile(const std::string& host_ip, const std::string& filename)
	{
		//1,向服务端发送文件下载请求"/filename"
		//2,得到响应结果,响应结果中的body正文就是文件数据
		//3,创建文件,将文件数据写入文件中,关闭文件
		std::string req_path = "/download/" + filename;
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		std::cout << "向服务端发送文件下载请求中" << host_ip << ":" << req_path << std::endl;
		auto rsp = cli.Get(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "获取文件下载响应失败\n";
			return false;
		}

		std::cout << "获取文件下载响应成功" << std::endl;
		//判断文件是否存在，如果不存在就创建
		if (!boost::filesystem::exists(DOWNLOAD_PATH))
			boost::filesystem::create_directory(DOWNLOAD_PATH);//创建文件
		std::string realpath = DOWNLOAD_PATH + filename;
		if (FileUtil::Write(filename, rsp->body) == false)
		{
			std::cerr << "文件下载失败\n";
			return false;
		}
		std::cout << "文件下载成功" << std::endl;
		return true;
	}

private:
	std::vector<Host>  _online_host;

};
