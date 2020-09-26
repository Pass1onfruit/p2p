#pragma once
//#ifndef _WIN32

#include<iostream>
#include<fstream>
#include<string>
#include <boost/filesystem.hpp>
#include<stdint.h>
#include<WS2tcpip.h>
#include<iphlpapi.h> //获取网卡信息接口的头文件
#pragma comment(lib, "Iphlpapi.lib") //获取网卡信息接口的库文件包含
#pragma comment(lib, "ws2_32.lib") //windows下的socket库
#include<vector>
//#else
//linux环境头文件
//#endif

class FileUtil
{
public:
	static bool Write(const std::string& name, const std::string& body, int64_t offset = 0) //向指定文件的指定未知写入指定数据 //offset(偏移量)
	{
		std::ofstream ofs(name);
		if (ofs.is_open() == false)
		{
			std::cerr << "打开文件失败" << std::endl;
			return false;
		}
		ofs.seekp(offset, std::ios::beg);  //读写位置跳转到相对于文件起始位置开始偏移offset的偏移量
		//ofstream.seekp(uint32_t offset 偏移量, std::ios::beg 开始偏移的位置)
		ofs.write(&body[0], body.size());
		//ofstream.write(const char* body 要写入的数据, size_t len)
		if (ofs.good() == false)
		{
			std::cerr << "向文件写入数据失败\n";
			ofs.close();
			return false;
		}
		ofs.close();
		return true;
	}
	//指针参数表示这是一个输出型参数
	//const &表示这是一个输入型参数
	//&表示这是一个输入输出型参数
	static bool Read(const std::string& name, std::string* body)
	{
		std::ifstream ifs(name);
		if (ifs.is_open() == false)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		int64_t filesize = boost::filesystem:: file_size(name);
		body->resize(filesize);
		ifs.read(&(*body)[0], filesize); 
		if (ifs.good() == false)
		{
			std::cerr << "读取文件数据失败\n";
			ifs.close();
			return false;
		}
		ifs.close();
		return true;
	}

};

class Adapter //网卡信息结构空间
{
public:
	uint32_t _ip_addr; //网卡上的IP地址
	uint32_t _mask_addr; //网卡上的子网掩码
};

class AdapterUtil //网络适配器
{
//#ifndef _WIN32
public:
	//windows下的获取网卡信息实现
	static bool GetAllAdapter(std::vector<Adapter> * List) //List中储存网卡信息
	{
		//开辟一块网卡信息结构空间
		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO(); //p_adapters是一个链表
		//GetAdaptersInfo win下获取网卡信息的接口-网卡信息有可能有多个,因此传入一个指针
		//获取网卡信息有可能会失败 
		//因为空间不足,因此有一个输出型参数,用于向用户返回所有网卡信息空间展会用大小
		uint64_t all_adapter_size = sizeof(IP_ADAPTER_INFO);
		//all_adapter_size：获取的所有网卡信息结构所占大小
		int ret = GetAdaptersInfo(p_adapters, (PULONG)& all_adapter_size);
		//GetAdaptersInfo：windows下提供的获取网卡的接口
		if (ret == ERROR_BUFFER_OVERFLOW)
		{
			//这个错误表示缓冲区空间不足
			//重新给指针申请空间
			delete p_adapters;
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];
			GetAdaptersInfo(p_adapters, (PULONG)& all_adapter_size);//重新获取网卡信息
		}
		while (p_adapters)
		{
			Adapter adapter; 
			//inet_addr只能转换为IPV4的地址结构
			//adapter._ip_addr = inet_addr(p_adapters->IpAddressList.IpAddress.String);
			//adapter._mask_addr = inet_addr(p_adapters->IpAddressList.IpMask.String);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);
			if (adapter._ip_addr != 0) //因为有些网卡并没有启用,导致IP地址为0
			{
				List->push_back(adapter); //讲网卡信息添加到vector中返回给用户
				std::cout << "网卡名称: " << p_adapters->AdapterName << std::endl;
				std::cout << "网卡描述: " << p_adapters->Description << std::endl;
				std::cout << "IP地址: " << p_adapters->IpAddressList.IpAddress.String << std::endl;
				std::cout << "子网掩码: " << p_adapters->IpAddressList.IpMask.String << std::endl;
				std::cout << std::endl;
			}

			p_adapters = p_adapters->Next;//遍历每一个网卡
		}
		delete p_adapters; //记得释放，不然内存泄漏

		return true;
	}
//#endif

};
