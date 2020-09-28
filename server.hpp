#pragma once
#include<thread>
#include "util.hpp"
#include"httplib2.h"
#include <boost/filesystem.hpp>
//#include<unordered_map>
#define P2P_PORT 9000
#define MAX_IPBUFFER 16
#define SHARED_PATH "./Shared/"


class _Server
{
public:
	bool Start()
	{
		//添加针对客户端请求的处理方式对应关系
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);
		//正则表达式:将特殊字符以指定的格式,表示表示具有关键特征的数据
		//正则表达式中.:匹配除\n或\r之外的任意字符*:表示匹配前边的字符任意次
		//防止与上方的请求冲突,因此在请求中加入download路径
		_srv.Get("/download/.*", Download);

		_srv.listen("0.0.0.0", P2P_PORT);
		return true;
	}
private:
	static void HostPair(const httplib::Request& req, httplib::Response& rsp)
	{
		return;
	}
	//获取共享文件列表-在主机上设置一个共享目录,凡是这个目录下的文件都是要共享给别人的
	static void ShareList(const httplib::Request& req, httplib::Response& rsp)
	{
		//查看目录是否存在，若目录不存在，则创建这个目录
		if (!boost::filesystem::exists(SHARED_PATH))//判断文件是否存在
		{
			//创建目录
			boost::filesystem::create_directory(SHARED_PATH);
		}
		boost::filesystem::directory_iterator begin(SHARED_PATH);//实例化目录迭代器
		boost::filesystem::directory_iterator end;//实例化目录迭代器的末尾
											   //开始选代目录
		while (begin != end)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				//如果是个目录就不用它,只获取普通文件名称,不做多层级目录的操作
				continue;
			}
			//获取文件名称
			std::string name = begin->path().filename().string();//只要文件名不要路径
			//将每个文件名称放到rsp的正文部分
			rsp.body += name + "\r\n"; //文件名的格式：filename1\r\nfilename2\r\n....
			++begin;
		}
		rsp.status = 200;
		return;
	}
	//下载文件
	static void Download(const httplib::Request& req, httplib::Response& rsp)
	{
		//req.path - 客户端请求的资源路径   /download/filename.txt
		boost::filesystem::path reqpath(req.path);
		//boost::filesystem::path(). filename()    只获取文件名称; abc/filename.txt -> filename.tx
		std::string name = reqpath.filename().string();//只获取文件名称  filename.txt
		std::string realpath = SHARED_PATH + name;//实际文件的路径应该是在共享的目录下
		//boost: :filesystem: :exists()判断文件是否存在
		std::cout << "服务端收到下载文件"<<std::endl<<"名称:" << name << std::endl << "路径:" << SHARED_PATH << std::endl;
		if (boost::filesystem::exists(realpath) || boost::filesystem::is_directory(realpath))
		{
			rsp.status = 404;
			return;
		}
		//文件出错
		if (FileUtil::Read(realpath, &rsp.body) == false)
		{
			rsp.status = 500;
			return;
		}
		//FileUtil::Read(name, &rsp.body);
		//正确处理
		rsp.status = 200;
		return;
	}
private:
	httplib::Server _srv;
};
