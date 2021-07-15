#include "ossclient.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <unistd.h>
#include <alibabacloud/oss/OssClient.h>

using namespace AlibabaCloud::OSS;

#include "logger.h"

std::map<std::string, std::string> META_MAP = {
        {"mp4",  "video/mp4"},
        {"png",  "image/png"},
        {"jpg",  "image/jpg"},
        {"webp", "image/webp"}
};

int64_t getFileSize(const std::string &file) {
    std::fstream f(file, std::ios::in | std::ios::binary);
    f.seekg(0, f.end);
    int64_t size = f.tellg();
    f.close();
    return size;
}

ossclient::ossclient(const std::string &access_key, const std::string &access_secret, const std::string &endpoint) {
    m_access_key = access_key;
    m_access_secret = access_secret;
    m_endpoint = endpoint;
    /* 初始化网络等资源 */
    InitializeSdk();
}

ossclient::~ossclient() {
    ShutdownSdk();
}

int ossclient::upload(const std::string &local_file, const std::string &save_name, const std::string &bucket,
                      std::string &ret_file_url, std::string &err_msg) {
    if (0 != access(local_file.c_str(), F_OK)) {
        err_msg = "src file is not exist!";
        FUNLOG(Err, "can not upload! src file is empty! src=%s", local_file);
        return 1;
    }

    HeaderCollection header;
    header["Content-type"] = "video/mp4";
    std::size_t found = local_file.find(".");
    if (found != std::string::npos) {
        std::string ext_name = local_file.substr(found + 1, local_file.size() - found);
        std::map<std::string, std::string>::iterator iter = META_MAP.find(ext_name);
        if (iter != META_MAP.end()) {
            header["Content-type"] = iter->second;
            FUNLOG(Info, "find match content-type=%s", iter->second.c_str());
        }
    } else {
        FUNLOG(Err, "can not find match content-type!");
    }

    ObjectMetaData meta(header);
    ClientConfiguration conf;
    conf.connectTimeoutMs = 10000;
    // use proxy
//    conf.proxyHost = "";
//    conf.proxyPort = 0;
//    conf.proxyUserName = "";
//    conf.proxyPassword = "";

    OssClient client(m_endpoint, m_access_key, m_access_secret, conf);
    InitiateMultipartUploadRequest initUploadRequest(bucket, save_name, meta);

    /* 初始化分片上传事件 */
    auto uploadIdResult = client.InitiateMultipartUpload(initUploadRequest);
    auto uploadId = uploadIdResult.result().UploadId();
    int64_t partSize = 100 * 1024;
    PartList partETagList;
    auto fileSize = getFileSize(local_file);
    int partCount = static_cast<int> (fileSize / partSize);
    /* 计算分片个数 */
    if (fileSize % partSize != 0) {
        partCount++;
    }

    /* 对每一个分片进行上传 */
    for (int i = 1; i <= partCount; i++) {
        int max_retry = 3;
        bool success = false;
        for (int x = 0; x < max_retry; x++) {
            auto skipBytes = partSize * (i - 1);
            auto size = (partSize < fileSize - skipBytes) ? partSize : (fileSize - skipBytes);
            std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(local_file,
                                                                                    std::ios::in | std::ios::binary);
            content->seekg(skipBytes, std::ios::beg);

            UploadPartRequest uploadPartRequest(bucket, save_name, content);
            uploadPartRequest.setContentLength(size);
            uploadPartRequest.setUploadId(uploadId);
            uploadPartRequest.setPartNumber(i);
            auto uploadPartOutcome = client.UploadPart(uploadPartRequest);
            if (uploadPartOutcome.isSuccess()) {
                Part part(i, uploadPartOutcome.result().ETag());
                partETagList.push_back(part);
                success = true;
                break;
            } else {
                std::cout << "uploadPart fail" <<
                          ",code:" << uploadPartOutcome.error().Code() <<
                          ",message:" << uploadPartOutcome.error().Message() <<
                          ",requestId:" << uploadPartOutcome.error().RequestId() << std::endl;
                continue;
            }
        }

        if (!success) {
            // upload fail!
            err_msg = "upload fail!";
            return 2;
        }
    }

    /* 完成分片上传 */
    CompleteMultipartUploadRequest request(bucket, save_name);
    request.setUploadId(uploadId);
    request.setPartList(partETagList);
    auto outcome = client.CompleteMultipartUpload(request);

    if (!outcome.isSuccess()) {
        FUNLOG(Err, "CompleteMultipartUpload fail! code=%d, message=%s, requestId=%s", outcome.error().Code(),
               outcome.error().Message(), outcome.error().RequestId());
        return 3;
    }

    char url[512] = {0};
    sprintf(url, "http://%s.%s/%s", bucket.c_str(), m_endpoint.c_str(), save_name.c_str());
    ret_file_url = std::string(url);
    return 0;
}

//int main(void)
//{
//	std::string AccessKeyId = "LTAI5e16ncCf6d2N";
//    std::string AccessKeySecret = "SFtCz1MFeyTXGxBPCR8RRV59vrWmMw";
//    std::string Endpoint = "oss-cn-shenzhen.aliyuncs.com";
//    std::string BucketName = "myvideodata";
//    std::string ObjectName = "swapface-data.mp4";
//
//	ossclient client(AccessKeyId, AccessKeySecret, Endpoint);
//	std::string file_url;
//	std::string msg;
//
//	std::string local_file =std::string("/home/luofeilong1/test.mp4");
//	std::string save_name = std::string("swapface-test-2.mp4");
//	int ret = client.upload(local_file, save_name, BucketName, file_url, msg);
//	if(0 == ret)
//		printf("upload success! url=%s\n", file_url.c_str());
//	else
//		printf("upload fail!\n");
//}
