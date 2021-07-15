#include "taskparams.h"
#include <json/json.h>
#include <iostream>       // std::cout
#include <string>         // std::string
#include "logger.h"

TaskParams::TaskParams() {
}

TaskParams::~TaskParams() {
}

bool TaskParams::parse(std::string &jsonstr) {
    Json::Reader reader;
    Json::Value root;
    try {
        if (!reader.parse(jsonstr.c_str(), root)) {
            FUNLOG(Err, "maybe format is not json");
            return false;
        }

        if (!root["taskid"].isString()) {
            FUNLOG(Err, "no taskid!");
            return false;
        }
        m_taskid = root["taskid"].asString();

        if (!root["secretid"].isString()) {
            FUNLOG(Err, "no secretid or secretid is not string");
            return false;
        }
        m_secretid = root["secretid"].asString();

        if (!root["priority"].isInt()) {
            m_priority = 0;
        } else {
            m_priority = root["priority"].asInt();
        }

        if (!root["traceid"].isNull()) {
            m_traceid = root["traceid"].asString();
        }

        if (root["input"].isNull()) {
            FUNLOG(Err, "%s 'input' not found. ", m_taskid.c_str());
            return false;
        }

        if (root["output"].isNull()) {
            FUNLOG(Err, "%s 'output' not found. ", m_taskid.c_str());
            return false;
        }

        Json::Value inputRoot = root["input"];
        Json::Value outputRoot = root["output"];
        if (inputRoot["datalist"].isNull()) {
            FUNLOG(Err, "datalist is null!");
            return false;
        }

        Json::Value dataList = root["input"]["datalist"];
        int dataSize = dataList.size();
        if (dataSize <= 0) {
            FUNLOG(Err, "datalist size is 0!");
            return false;
        }

        for (int i = 0; i < dataSize; i++) {
            if (!dataList[i]["url"].isString()) {
                FUNLOG(Err, "url is not string!");
                return false;
            }

            InputConfig inputItem;
            inputItem.url = dataList[i]["url"].asString();

            if (!dataList[i]["dstfile"].isNull()) {
                inputItem.dstfilename = dataList[i]["dstfile"].asString();
            }

            if (!dataList[i]["context"].isNull()) {
                inputItem.context = dataList[i]["context"];
            }

            this->m_inputs.push_back(inputItem);
        }

        //if(outputRoot["upload"].isNull())
        //{
        //	FUNLOG(Err, "no upload config!");
        //	return false;
        //}

        //Json::Value uploadNode = outputRoot["upload"];
        //if(uploadNode["bs2"].isNull() && uploadNode["oss"].isNull() && uploadNode["google"].isNull())
        //{
        //	FUNLOG(Err, "upload node has no bs2 and oss and google config!");
        //	return false;
        //}

        //if(!uploadNode["bs2"].isNull())
        //{
        //	this->m_output.upload_type = UPLOAD_BS2;
        //	if(!uploadNode["bs2"]["bucket"].isNull())
        //		this->m_output.bucket = uploadNode["bs2"]["bucket"].asString();
        //	if(!uploadNode["bs2"]["secret_key"].isNull())
        //		this->m_output.secret_key = uploadNode["bs2"]["secret_key"].asString();
        //	if(!uploadNode["bs2"]["secret_code"].isNull())
        //		this->m_output.secret_code = uploadNode["bs2"]["secret_code"].asString();
        //}

        ////use google storage
        //if(!uploadNode["google"].isNull())
        //{
        //    this->m_output.upload_type = UPLOAD_GOOGLE;
        //	if(!uploadNode["google"]["auth"].isNull())
        //    {
        //        this->m_output.google_auth = uploadNode["google"]["auth"].asString();
        //		this->m_output.bucket = uploadNode["google"]["bucket"].asString();
        //    }
        //}

        ////use aliyun oss
        //if(!uploadNode["oss"].isNull())
        //{
        //	this->m_output.upload_type = UPLOAD_OSS;
        //	if(!uploadNode["oss"]["bucket"].isNull())
        //		this->m_output.bucket = uploadNode["oss"]["bucket"].asString();
        //	if(!uploadNode["oss"]["secret_key"].isNull())
        //		this->m_output.secret_key = uploadNode["oss"]["secret_key"].asString();
        //	if(!uploadNode["oss"]["secret_code"].isNull())
        //		this->m_output.secret_code = uploadNode["oss"]["secret_code"].asString();
        //	if(!uploadNode["oss"]["end_point"].isNull())
        //		this->m_output.end_point = uploadNode["oss"]["end_point"].asString();
        //}

        //if(!uploadNode["host"].isNull())
        //{
        //    this->m_output.cdn_host = uploadNode["host"].asString();
        //}

        if (outputRoot["resultpush"].isNull()) {
            FUNLOG(Err, "no resultpush config!");
            return false;
        }

        Json::Value resultNode = outputRoot["resultpush"];
        if (resultNode["http"].isNull() && resultNode["kafka"].isNull()) {
            FUNLOG(Err, "upload node has no http and kafka config!");
            return false;
        }

        if (!resultNode["http"].isNull()) {
            this->m_output.push_type = PUSH_HTTP;
            if (!resultNode["http"]["url"].isNull())
                this->m_output.http_url = resultNode["http"]["url"].asString();
        }
        if (!resultNode["kafka"].isNull()) {
            this->m_output.push_type = PUSH_KAFKA;
            if (!resultNode["kafka"]["hostport"].isNull())
                this->m_output.kafka_hostport = resultNode["kafka"]["hostport"].asString();
            if (!resultNode["kafka"]["topic"].isNull())
                this->m_output.kafka_topic = resultNode["kafka"]["topic"].asString();
        }

        return true;
    }
    catch (std::exception &e) {
        FUNLOG(Err, "TaskParams::parse exception");
    }

    return false;
}



//int main() {
//        std::string raw = "{\"key\":\"value\",\"我\":\"是誰\",\"array\":[\"a\",\"b\",123]}";
//        Json::Reader reader;
//        Json::Value value;
//        std::cout << "Input: " << raw << std::endl;
//        if (reader.parse(raw, value)) {
//                std::cout << "parsing: " << value ;//<< std::endl;
//                std::cout << "get: " << value["我"] ;//<< std::endl;
//                std::string data = value["key"].asString();//toStyledString();
//                std::cout << "string: [" << data << "]" << std::endl;
//
//                // with -std=c++11
//                std::cout << "---" << std::endl;
//                for (auto it : value) {
//                        std::cout << "elm: " << it << std::endl;
//                        if (it.isArray()) {
//                                for (auto it2 : it)
//                                        std::cout << "array data: " << it2 << std::endl;
//                        } 
//                }         
//        } 
//        return 0;
//}

