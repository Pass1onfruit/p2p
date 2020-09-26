#include <boost/filesystem.hpp>
#include"util.hpp"
#include"httplib.h"
#include "client.hpp"


void helloworld(const httplib::Request& req, httplib::Response &rsp)
{
	printf("httplib server recv a req:%s\n", req.path.c_str());
	rsp.set_content("<html><h1>HelloWorld</h1></html>", "text/html");
	rsp.status = 200;
}
void Scandir()
{
	const char* ptr = "./";
	boost::filesystem::directory_iterator begin(ptr); //定义一个目录迭代器对象
	boost::filesystem::directory_iterator end;
	for (; begin != end; ++begin)
	{
		//begin->status () 目录中当前文件的状态信息
	//boost:: filesystem :: is directory()判断当前文件是否是一个目录
		if (boost::filesystem::is_directory(begin->status()))
		{ 
			//begin->path ().string()获取当前迭代文件的文件名
			std::cout << begin->path().string() << "是一个目录\n";
		}
		else
		{
			std::cout << begin->path().string() << "是一个普通文件\n";
			//begin->path(). filename()  获取路文件路径名中的文件名称,而不要路径
			std::cout << "文件名: " << begin->path().filename().string() << std::endl;
		}
	}
}

void test()
{
	httplib::Server srv;
	srv.Get("/", helloworld);
	srv.listen("0.0.0.0", 9000);
	//std::vector<Adapter> List;
	//AdapterUtil::GetAllAdapter(&List);
}

void ClientRun()
{
	_Client cli;
	cli.Start();
}

int main()
{
	// 在主线程中要运行客户端模块以及服务端模块
	//创建一个线程运行客户端模块,主线程运行服务端模块
	std::thread thr_clilent(ClientRun);
	_Server srv;
	srv.Start();

	return 0;
}
