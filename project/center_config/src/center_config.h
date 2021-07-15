#ifndef _BIZ_CONFIG_H_
#define _BIZ_CONFIG_H_

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <ctime>
#include <utility>
#include <memory>

#include "md5.h"
#include "jsoncpp/json/json.h"

using std::endl;
using std::cout;
using std::map;
using std::string;
using std::shared_ptr;
using std::cerr;

enum {
    UPDATE_SUCCESS = 0,
    UPDATE_FAILED = 1
};

//------------------------------------------------------------
//函数功能：
//  1. 监控文件更新时间
//  2. 提取更新的文件内容（只提取，不做业务逻辑）
//  3. 加载完成后，若成功：修改 mtime 为加载完成时的时间；
//               若失败：修改 mtime < ctime 
//------------------------------------------------------------
class FileWatcher {
public:
    FileWatcher(const string &path, time_t time = 0) : m_filePath(path), m_lastUpdateTime(time) {}

    //检测文件更新时间
    bool checkFileUpdate();

    //修改文件的mtime
    int changeFileMtime(int flag = UPDATE_SUCCESS);

    //获取整个文件内容
    string getFileContent();

private:
    string m_filePath;
    time_t m_lastUpdateTime;
};

//------------------------------------------------------------
//函数功能：
//  1. 管理多个文件
//------------------------------------------------------------
class FileWatcherGroup {
public:
    int addFileWatcher(const string &filePath);

    int removeFileWatcher(const string &filePath);

    int changeFileWatcherMtime(const string &filePath, int flag = UPDATE_SUCCESS);

    //检测所有文件是否更新   返回值：filePath，file content
    map<string, string> loadGroupUpdate();

    string loadFileUpdate(const string &filePath);

    ~FileWatcherGroup() {
        m_filePath2Watcher.clear();
    }

private:
    map<string, shared_ptr<FileWatcher>> m_filePath2Watcher;//file_path->watcher
};

//------------------------------------------------------------
//函数功能：
//  1. 提供文件内容解析方式:Json、yaml格式等的统一返回接口；
//  2. 由派生类JsonParser、YamlParser实现具体的解析
//说明：若需要支持其他类型的解析方法，则定义对应解析的类
//------------------------------------------------------------
class ContentParser {
public:
    ContentParser(string idName = "businessId") : m_busiIdName(idName) {}

    //输入为整个文件内容；返回：biz-》对应的内容
    virtual map<string, string> parse(const string &content) {
        map<string, string> resutl;
        return resutl;
    }

private:
    string m_busiIdName;
};

class JsonParser : public ContentParser {
public:
    JsonParser(string idName = "businessId") : m_jsonBusiIdName(idName) {}

    map<string, string> parse(const string &content);

private:
    string m_jsonBusiIdName;
};

class YamlParser : public ContentParser {
public:
    map<string, string> parse(const string &content);
};

//------------------------------------------------------------
//函数功能：
//  1. addFile---》增加需要监控的文件
//  2. removeFile---》删除需要监控的文件
//  3. loadFileConfIfUpdate---》检测文件里面更新的内容
//      返回值：业务id，以及对应的业务配置内容
//  4. updateConfigSuccess---》配置更新成功调用此函数
//  5. updateConfigFailed---》配置更新失败调用此函数
//实现为单例
//------------------------------------------------------------

class BusiConfigManager {
public:
    static BusiConfigManager *instance() {
        if (!m_instance) {
            m_instance = new BusiConfigManager();
        }
        return m_instance;
    }

    ~BusiConfigManager() {
        m_fileParser.clear();
    }

    int addFile(const string &filePath, shared_ptr<ContentParser> parser);

    int removeFile(const string &filePath);

    //更新文件状态时间mtime
    int updateConfigSuccess(const string &filePath);

    int updateConfigFailed(const string &filePath);

    //返回值：biz，对应需要修改的内容
    map<string, string> loadFileConfIfUpdate(const string &filePath);

private:
    BusiConfigManager() {}

private:
    static BusiConfigManager *m_instance;

    MD5 m_md5;//计算md5值的类
    map<string, map<string, string> > m_fileBizMd5;//file->biz,md5
    map<string, shared_ptr<ContentParser> > m_fileParser;
    FileWatcherGroup m_watcherGroup;
};

#endif
