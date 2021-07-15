#ifndef  __BS2_CLIENT_H__
#define __BS2_CLIENT_H__

#include <string>
#include <stdint.h>

using namespace std;

class CBs2Client
{
public:
    CBs2Client(const string &taskId, const string &uploadHost, const string &downloadHost,
    			const string &bucket, const string &accessKey, const string accessSecret);
    ~CBs2Client();

    bool upload(const char *data, int len, string &filename, string &downloadUrl);

    bool uploadAgain(const char *data, int len, uint32_t uid, uint32_t screenshotTs);

private:
	string m_taskId;
    string m_uploadHost;
    string m_downloadHost;
    string m_accessKey;
    string m_accessSecret;
    string m_bucket;
};

#endif
