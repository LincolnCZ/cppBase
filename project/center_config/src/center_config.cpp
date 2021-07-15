#include <sstream>
#include <utime.h>
#include <sys/time.h>

#include "center_config.h"
#include "ZyLogger.h"

BusiConfigManager *BusiConfigManager::m_instance = NULL;
const int WAIT_SECOND = 10;//文件当前时间比上次更新大于WAIT_SECOND才更新
const int FAILD_DIFF = 100;//更新失败后设置文件的时间戳差值

bool FileWatcher::checkFileUpdate() {
    struct stat stat_info;
    if (stat(m_filePath.c_str(), &stat_info) == -1) {
        ZLOGERROR << "get file stat error, file: " << m_filePath;
        return false;
    }
    time_t change_time = stat_info.st_ctime;
    if (change_time > m_lastUpdateTime + WAIT_SECOND) {
        ZLOGDEBUG << "FileWatcher: file is updated, file: " << m_filePath
                  << " change_time: " << change_time << " m_lastUpdateTime :" << m_lastUpdateTime;
        m_lastUpdateTime = change_time;
        return true;
    }
    return false;
}

string FileWatcher::getFileContent() {
    string result;
    std::ifstream ifile(m_filePath.c_str());
    if (!ifile.is_open()) {
        ZLOGERROR << "open file failed! file: " << m_filePath;
        return result;
    }
    std::ostringstream buf;
    buf << ifile.rdbuf();
    result = buf.str();
    return result;
}

int FileWatcher::changeFileMtime(int flag) {
    struct utimbuf timebuf;
    if (flag == UPDATE_SUCCESS) {
        struct timeval cur_time;
        gettimeofday(&cur_time, NULL);
        timebuf.modtime = cur_time.tv_sec;
        timebuf.actime = m_lastUpdateTime;
    } else {
        timebuf.modtime = m_lastUpdateTime - FAILD_DIFF;
        timebuf.actime = m_lastUpdateTime;
    }

    int ret = utime(m_filePath.c_str(), &timebuf);
    if (ret != 0) {
        ZLOGERROR << "call utime func failed, file:" << m_filePath;
        return -1;
    }
    ZLOGDEBUG << "change file flag:" << flag << ", change file mtime success, file:"
              << m_filePath << "\ttimebuf.modtime: " << timebuf.modtime;

    return 0;
}

int FileWatcherGroup::addFileWatcher(const string &filePath) {
    map<string, shared_ptr<FileWatcher>>::iterator it = m_filePath2Watcher.find(filePath);
    if (it != m_filePath2Watcher.end()) {
        ZLOGDEBUG << "file already exist, file: " << filePath;
    } else {
        ZLOGDEBUG << "add new file wacher, file :" << filePath;
        shared_ptr<FileWatcher> watcher(new FileWatcher(filePath));
        m_filePath2Watcher[filePath] = watcher;
    }
    return 0;
}

int FileWatcherGroup::removeFileWatcher(const string &filePath) {
    map<string, shared_ptr<FileWatcher>>::iterator it = m_filePath2Watcher.find(filePath);
    if (it == m_filePath2Watcher.end()) {
        ZLOGERROR << "file is not exist in FileWatcherGroup, file: " << filePath;
        return 0;
    } else {
        m_filePath2Watcher.erase(it);
        ZLOGDEBUG << "remove file success, file: " << filePath;
    }
    return 0;
}

int FileWatcherGroup::changeFileWatcherMtime(const string &filePath, int flag) {
    map<string, shared_ptr<FileWatcher>>::iterator it = m_filePath2Watcher.find(filePath);
    if (it == m_filePath2Watcher.end()) {
        ZLOGERROR << "file is not exist in FileWatcherGroup, file: " << filePath;
        return -1;
    } else {
        int ret = (it->second)->changeFileMtime(flag);
        if (ret != 0) {
            ZLOGERROR << "change file mtime failed, file: " << filePath;
        }
    }
    return 0;
}

map<string, string> FileWatcherGroup::loadGroupUpdate() {
    map<string, string> result;
    for (map<string, shared_ptr<FileWatcher>>::iterator it = m_filePath2Watcher.begin();
         it != m_filePath2Watcher.end(); ++it) {
        if ((it->second)->checkFileUpdate()) {
            string content = it->second->getFileContent();
            result.insert(make_pair(it->first, content));
        }
    }
    return result;
}

string FileWatcherGroup::loadFileUpdate(const string &filePath) {
    string content;
    auto it = m_filePath2Watcher.find(filePath);
    if (it != m_filePath2Watcher.end()) {
        if ((it->second)->checkFileUpdate()) {
            content = it->second->getFileContent();
            //ZLOGDEBUG << "file content update, content: "<< content <<"file :" << filePath;
        }
    }
    return content;
}

map<string, string> JsonParser::parse(const string &content) {
    map<string, string> result;
    if (content.empty()) return result;

    Json::Value conf_json;
    Json::Reader reader;
    if (!reader.parse(content, conf_json)) {
        ZLOGERROR << "parse failed.";
        return result;
    }
    if (!conf_json.isArray()) {
        ZLOGERROR << "conf_json is not a arrary!";
        return result;
    }
    for (int i = 0; i < conf_json.size(); ++i) {
        if (!conf_json[i].isObject())
            continue;
        if (conf_json[i][m_jsonBusiIdName].isString()) {
            string biz = conf_json[i][m_jsonBusiIdName].asString();
            string biz_content = conf_json[i].toStyledString();
            result.insert(make_pair(biz, biz_content));
        }
    }
    return result;
}

int BusiConfigManager::addFile(const string &filePath, shared_ptr<ContentParser> parser) {
    int ret = m_watcherGroup.addFileWatcher(filePath);
    if (ret != 0) {
        ZLOGERROR << "add file failed!, file: " << filePath;
        return -1;
    }

    map<string, shared_ptr<ContentParser> >::iterator it = m_fileParser.find(filePath);
    if (it != m_fileParser.end())//already exist
    {
        ZLOGDEBUG << "content parser is already exist, will reload , file: " << filePath;
    } else {
        ZLOGDEBUG << "add file parser, file : " << filePath;
    }
    m_fileParser[filePath] = parser;

    return 0;
}

int BusiConfigManager::removeFile(const string &filePath) {
    int ret = m_watcherGroup.removeFileWatcher(filePath);
    if (ret != 0) {
        ZLOGERROR << "remove file failed, file: " << filePath;
    }

    map<string, shared_ptr<ContentParser> >::iterator it = m_fileParser.find(filePath);
    if (it != m_fileParser.end()) {
        m_fileParser.erase(it);
    }
    return 0;
}

int BusiConfigManager::updateConfigSuccess(const string &filePath) {
    return m_watcherGroup.changeFileWatcherMtime(filePath, UPDATE_SUCCESS);
}

int BusiConfigManager::updateConfigFailed(const string &filePath) {
    return m_watcherGroup.changeFileWatcherMtime(filePath, UPDATE_FAILED);
}

map<string, string> BusiConfigManager::loadFileConfIfUpdate(const string &filePath) {
    map<string, string> result;
    if (filePath.empty()) {
        ZLOGERROR << "filePath is empty()";
        return result;
    }
    string file_content = m_watcherGroup.loadFileUpdate(filePath);
    if (file_content.empty()) {
        return result;
    }
    auto parse_it = m_fileParser.find(filePath);
    if (parse_it == m_fileParser.end()) {
        ZLOGERROR << "parse is NULL, file: " << filePath;
        return result;
    }

    //biz,biz对应的修改内容
    map<string, string> biz_conf = (parse_it->second)->parse(file_content);
    if (biz_conf.empty()) {
        ZLOGERROR << "busi conf is empty(), file: " << filePath;
        return result;
    }
    //biz，biz对应内容的md5值
    map<string, string> biz_md5val;
    for (auto it = biz_conf.begin(); it != biz_conf.end(); ++it) {
        string md5_val = m_md5.hash(it->second);
        biz_md5val.insert(make_pair(it->first, md5_val));
    }

    //对比更新的文件内容和之前保存内容的md5
    map<string, map<string, string> >::iterator file_it = m_fileBizMd5.find(filePath);
    if (file_it == m_fileBizMd5.end())//
    {
        m_fileBizMd5[filePath] = biz_md5val;
        result = biz_conf;
    } else {
        for (auto it = biz_md5val.begin(); it != biz_md5val.end(); ++it) {
            map<string, string>::iterator conf_it = (file_it->second).find(it->first);
            if (conf_it == (file_it->second).end()) {
                result[it->first] = biz_conf[it->first];
                (file_it->second).insert(make_pair(it->first, it->second));
            } else {
                if (conf_it->second != it->second) {
                    result[it->first] = biz_conf[it->first];
                    (file_it->second).erase(it->first);
                    (file_it->second).insert(make_pair(it->first, it->second));
                }
            }
        }
    }
    return result;
}

