#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include "center_config.h"

using namespace std;

void handle_busi_config1(const string &filePath) {
    map<string, string> result1;
    result1 = BusiConfigManager::instance()->loadFileConfIfUpdate(filePath);
    if (!result1.empty()) {
        cout << "the biz conf need to update :" << endl;
        for (auto it = result1.begin(); it != result1.end(); ++it) {
            cout << "\tbusinessId:" << it->first << "\tconf:" << it->second << endl;
            //todo...业务逻辑处理
        }
        //业务逻辑处理成功，调用updateConfigSuccess函数
        BusiConfigManager::instance()->updateConfigSuccess(filePath);
        //业务逻辑处理失败，调用updateConfigFailed
        //BusiConfigManager::instance()->updateConfigFailed(filePath);
    }
}

void handle_busi_config2(const string &filePath) {
    map<string, string> result2;
    result2 = BusiConfigManager::instance()->loadFileConfIfUpdate(filePath);
    if (!result2.empty()) {
        cout << "the biz conf need to update :" << endl;
        for (auto it = result2.begin(); it != result2.end(); ++it) {
            cout << "\tbusinessId:" << it->first << "\tconf:" << it->second << endl;
            //todo...业务逻辑处理
        }
        //业务逻辑处理成功，调用updateConfigSuccess函数
        BusiConfigManager::instance()->updateConfigSuccess(filePath);
        //业务逻辑处理失败，调用updateConfigFailed
        //BusiConfigManager::instance()->updateConfigFailed(filePath);
    }
}

int main() {
    //在进程启动时调用addFile函数
    string file_path("./doc/l5.config");
    shared_ptr<ContentParser> json_parser(new JsonParser());
    int ret = BusiConfigManager::instance()->addFile(file_path, json_parser);//指定需要监控的文件以及解析方式
    if (ret != 0) {
        cout << "add file failed!" << endl;
    }

    string file_path2("./doc/smu.config");
    shared_ptr<ContentParser> json_parser2(new JsonParser);
    ret = BusiConfigManager::instance()->addFile(file_path2, json_parser2);
    if (ret != 0) {
        cout << "add file failed!" << endl;
    }

    //在框架中的定时轮询
    while (true) {
        handle_busi_config1(file_path);
        handle_busi_config2(file_path2);
    }

    return 0;
}
