bool rangedown(const std::string &host_ip, const std::string &name, int64_t s, int64_t e){ 
    std::string req_path = "/download/" + name;
    std::string realpath = DOWNLOAD_PATH + name;
    //httplib::Headers header;
    //header = httplib::make_range_header({{s, e}});
    //header.insert(httplib::make_range_header({ {s, e} }));//设置一个range区间
    httplib::Client cli(host_ip.c_str(), P2P_PORT);
    auto rsp = cli.Get(req_path.c_str(), {httplib::make_range_header(s, e)});
    if (rsp == NULL || rsp->status != 206) {
        if (rsp == NULL) { std::cout << "client rsp status NULL\n"; }
		///


        else { std::cout << "响应状态码" << rsp->status << "\n"; }
        std::cout << "range download failed\n";
        return false;
    }   
    std::cout << "client range write [" << rsp->body << "]\n";
    FileUtil::Write(realpath, rsp->body, s); 
    std::cout << "client range write success\n";



    return true;
}
int64_t getfilesize(const std::string &host_ip, const std::string &req_path) {
    //1. 发送HEAD请求，通过响应中的Content-Length获取文件大小
    httplib::Client cli(host_ip.c_str(), P2P_PORT);
    auto rsp = cli.Head(req_path.c_str());
    if (rsp == NULL || rsp->status != 200) {
        std::cout << "获取文件大小信息失败\n";
        return false;
    }
    std::string clen = rsp->get_header_value("Content-Length");
    int64_t filesize = StringUtil::Str2Dig(clen);
    return filesize;
}
bool RangeDownload(const std::string &host_ip, const std::string &name) {
    std::string req_path = "/download/" + name;
    int64_t filesize = getfilesize(host_ip, req_path);
    //2. 根据文件大小进行分块
    //int range_count = filesize / MAX_RANGE;
    //a. 若文件大小小于块大小，则直接下载文件
    if (filesize < MAX_RANGE) {
        std::cout << "文件较小,直接下载文件\n";
        return DonwloadFile(host_ip, name);
    }
    //计算分块个数
    //b. 若文件大小不能整除块大小，则分块个数位文件大小除以分块大小然后+1
    //c. 若文件大小刚好整除块大小，则分块个数就是文件大小除以分块大小
    std::cout << "too max, range download\n";
    int range_count = 0;
    if ((filesize % MAX_RANGE) == 0) {
        range_count = filesize / MAX_RANGE;
    }else {
        range_count = (filesize / MAX_RANGE) + 1;
    }
    // 136   100    0~99  100~135
    int64_t range_start = 0, range_end = 0;
    for (int i = 0; i < range_count; i++) {
        range_start = i * MAX_RANGE;
        if (i == (range_count - 1)) {
            range_end = filesize - 1;
        }else {
            range_end = ((i + 1) * MAX_RANGE) - 1;
        }
        std::cout << "client range req:" << range_start << "-" << range_end << std::endl;
        //3. 逐一请求分块区间数据，得到响应之后写入文件的指定位置
        rangedown(host_ip, name, range_start, range_end);
    }
    std::cout << "Download Success\n";
    return true;
}
    
