#include <string>
#include <vector>
#include <json/json.h>

struct ResultContent {
    std::string out_url;
    int predict_code;
    Json::Value predict_result;

    Json::Value getJsonObj() {
        Json::Value obj;
        obj["url"] = out_url;
        obj["code"] = predict_code;
        obj["predictions"] = predict_result;
        return obj;
    }
};

struct OneFileResult {
    Json::Value context;
    int code;
    std::string msg;
    ResultContent result;
    std::string url;

    Json::Value getJsonObj() {
        Json::Value obj;
        obj["context"] = context;
        obj["code"] = code;
        obj["msg"] = msg;
        obj["result"] = result.getJsonObj();
        obj["url"] = url;

        return obj;
    }
};

struct FinalResult {
    std::string traceid;
    std::string serverid;
    std::string taskid;
    std::vector<OneFileResult> resultlist;

    std::string getJsonStr() {
        Json::Value root;
        root["traceid"] = traceid;
        root["serverid"] = serverid;
        root["taskid"] = taskid;

        Json::Value array;
        for (int i = 0; i < resultlist.size(); i++) {
            array.append(resultlist[i].getJsonObj());
        }
        root["resultlist"] = array;

        Json::FastWriter writer;
        return writer.write(root);
        //return root.toStyledString();
    }
};
