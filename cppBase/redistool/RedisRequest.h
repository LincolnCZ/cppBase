#pragma once

#include <string>
#include <map>
#include <vector>
#include "core/singleton.h"
#include "RedisManager.h"

struct redisReply;
struct redisAsyncContext;

class RedisRequest {
public:
    explicit RedisRequest(const std::string &key, const std::string &traceId = "");
    virtual ~RedisRequest() = default;
    std::string getRequestType() const { return m_type; }
    std::string getTraceId() const { return m_traceId; };

    bool Execute();
    bool OnReply(redisAsyncContext *, const redisReply *);
    bool OnCompleted();

    // 接口函数
    virtual bool ExecuteImpl() { return false; }
    virtual bool OnReplyImpl(redisAsyncContext *, const redisReply *) { return false; }
    virtual bool OnCompletedImpl() { return false; }

protected:
    // 标识请求的类型
    std::string m_type;
    // traceId
    std::string m_traceId;
    // 返回的错误信息
    std::string m_strError;
};

class LoadScriptRequest : public RedisRequest {
public:
    explicit LoadScriptRequest(const std::string & filePath);
    ~LoadScriptRequest() override= default;;

    bool ExecuteImpl() override;
    bool OnReplyImpl(redisAsyncContext *, const redisReply *) override;
    bool OnCompletedImpl() override { return true; }

private:
    std::string m_filePath;
};


class LoadScriptMgr :public Singleton<LoadScriptMgr> {
public:
    // 对外提供的函数
    explicit LoadScriptMgr()=default;
    ~LoadScriptMgr() override = default;;

    bool init(const std::vector<std::string> &paths);
    void load(uint64_t nowMs);
    void clearSha1();

public:
    // 提供给LoadScriptRequest 使用
    std::string getScript(const std::string &filePath);
    std::string getSha1(const std::string &filePath);
    void setSha1(const std::string &filePath, const std::string & sha);

private:
    struct RedisScriptItem {
        std::string file; // lua脚本文件路径
        std::string script;
        std::string sha1;
        uint64_t timestamp{0};
    };
    std::map<std::string, RedisScriptItem> m_scripts;
};