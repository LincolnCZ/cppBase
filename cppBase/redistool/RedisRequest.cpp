#include <sstream>
#include "hiredis/async.h"
#include "core/logger.h"
#include "utility/sysutil.h"
#include "redistool/RedisRequest.h"

RedisRequest::RedisRequest(const std::string &key, const std::string &traceId)
        : m_type(key) {
    m_traceId = m_type + "_" + traceId;
}

bool RedisRequest::Execute() {
    bool ret = ExecuteImpl();

    if (!ret) {
        m_strError = "Execute error";
    }

    return ret;
}

bool RedisRequest::OnReply(redisAsyncContext *c, const redisReply *reply) {
    std::ostringstream os;
    do {
        if (!c) {
            os << "NULL CONTEXT";
            break;
        }

        if (!reply) {
            os << "NULL REPLY";
            break;
        }

        if (REDIS_REPLY_ERROR == reply->type) {
            os << "reply error:" << reply->str;
            break;
        }

        if (c->err) {
            os << "redisAsyncContext errstr:" << c->errstr;
            break;
        }
    } while (false);
    m_strError = os.str();

    return OnReplyImpl(c, reply);
}

bool RedisRequest::OnCompleted() {

    bool ret = OnCompletedImpl();

    return ret;
}

LoadScriptRequest::LoadScriptRequest(const std::string &filePath)
        : RedisRequest(filePath, ""), m_filePath(filePath) {
}

bool LoadScriptRequest::ExecuteImpl() {
    std::string script = LoadScriptMgr::getInstance()->getScript(m_filePath);

    if (script.empty()) {
        FUNCLOG (Error, "script not loaded, file:%s", m_filePath.c_str());
        return false;
    }

    bool ret = RedisManager::getInstance()->execute(this, "SCRIPT LOAD %s", script.c_str());
    FUNCLOG(Info, "LoadScriptRequest::ExecuteImpl() ret: %d", ret);
    return ret;
}

bool LoadScriptRequest::OnReplyImpl(redisAsyncContext *, const redisReply *reply) {
    if (!m_strError.empty()) {
        FUNCLOG(Error, "loadScriptRequest return inner error:%s", m_strError.c_str());
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        FUNCLOG (Warn, "%s: invalid reply type:%d, file:%s", getRequestType().c_str(), reply->type, m_filePath.c_str());
        return false;
    }

    if (!LoadScriptMgr::getInstance()->getSha1(m_filePath).empty()) {
        FUNCLOG (Warn, "%s: script already loaded: file:%s", getRequestType().c_str(), m_filePath.c_str());
    }

    LoadScriptMgr::getInstance()->setSha1(m_filePath, reply->str);
    FUNCLOG (Info, "%s: script loaded: file:%s, sha:%s", getRequestType().c_str(), m_filePath.c_str(), reply->str);

    return true;
}

// ---------------------------------LoadScriptMgr------------------------------------

bool LoadScriptMgr::init(const std::vector<std::string> &paths) {
    for (size_t i = 0; i < paths.size(); ++i) {
        RedisScriptItem item;
        item.file = paths[i];
        m_scripts.insert(std::make_pair(paths[i], item));
    }

    for (auto it = m_scripts.begin(); it != m_scripts.end(); ++it) {

        if (!utility::readFile(it->first, it->second.script)) {
            FUNCLOG(Error, "read script failed");
            return false;
        }

        if (it->second.script.empty()) {
            FUNCLOG (Error, "fail to read script from file:%s", it->first.c_str());
            return false;
        }

        FUNCLOG (Info, "read script successfully from file:%s, length:%lu", it->first.c_str(),
                 it->second.script.length());
    }

    return true;
}

void LoadScriptMgr::load(uint64_t nowMs) {
    for (auto it = m_scripts.begin(); it != m_scripts.end(); ++it) {
        if (it->second.sha1.empty() && (nowMs - it->second.timestamp > 200)) {
            it->second.timestamp = nowMs;

            RedisRequest *pRequest = new LoadScriptRequest(it->second.file);
            if (pRequest) {
                RedisManager::getInstance()->send(pRequest);

                FUNCLOG (Info, "try to load script from file:%s", it->second.file.c_str());
            }
        }
    }
}

void LoadScriptMgr::clearSha1() {
    for (auto it = m_scripts.begin(); it != m_scripts.end(); ++it) {
        it->second.sha1 = "";
        it->second.timestamp = 0;
    }
}

std::string LoadScriptMgr::getScript(const std::string &filePath) {
    auto it = m_scripts.find(filePath);
    if (it != m_scripts.end()) {
        return it->second.script;
    }
    return "";
}

std::string LoadScriptMgr::getSha1(const std::string &filePath) {
    auto it = m_scripts.find(filePath);
    if (it != m_scripts.end()) {
        return it->second.sha1;
    }
    return "";
}


void LoadScriptMgr::setSha1(const std::string &filePath, const std::string &sha) {
    auto it = m_scripts.find(filePath);
    if (it != m_scripts.end()) {
        it->second.sha1 = sha;
    }
}