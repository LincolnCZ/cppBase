#undef __CLASS__
#define __CLASS__ "CBs2Client"

#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <functional>
#include <stdio.h>
#include <time.h>
#include <uuid/uuid.h>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "logger.h"
#include "Bs2Client.h"

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

string urlBase64Encode(const unsigned char *input, int length)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    (void) BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    string result;
    result.resize(bptr->length - 1);
    for (size_t i = 0; i < bptr->length - 1; i++) {
        if (bptr->data[i] == '+') {
            result.at(i) = '-';
        }
        else if (bptr->data[i] == '/') {
            result.at(i) = '_';
        }
        else {
            result.at(i) = bptr->data[i];
        }
    }
    BIO_free_all(b64);
    return result;
}

string& strip(string &str)
{
    // ltrim
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                        std::not1(std::ptr_fun<int, int>(std::isspace))));

    // rtrim
    str.erase(std::find_if(str.rbegin(), str.rend(),
                           std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());

    return str;
}

string authorization(const string &bucket, const string &accessKey, const string &accessSecret,
                     const string &fileName, string expires, const string &method)
{
    if (expires.empty()) {
        struct timeval tm;
        gettimeofday(&tm, NULL);
        stringstream ss;
        ss << tm.tv_sec;
        expires = ss.str();
    }

    string content = method + '\n' + bucket + '\n' + fileName + '\n' + expires + '\n';

    unsigned char *md = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
    uint32_t md_length = 0;
    const EVP_MD * engine = NULL;
    engine = EVP_sha1();
    HMAC(engine, accessSecret.c_str(), accessSecret.length(), (unsigned char *)content.c_str(), content.length(), md, &md_length);

    string contentEncoded = urlBase64Encode(md, md_length);
    free(md);

    contentEncoded = strip(contentEncoded);
    return accessKey + ':' + contentEncoded + ':' + expires;
}

/**
Pass a pointer to your callback function, as the prototype shows above.

This callback function gets called by libcurl as soon as it needs to read data in order to send
it to the peer - like if you ask it to upload or post data to the server. The data area pointed
at by the pointer buffer should be filled up with at most size multiplied with nitems number of
 bytes by your function.

Your function must then return the actual number of bytes that it stored in that memory area.
Returning 0 will signal end-of-file to the library and cause it to stop the current transfer.

If you stop the current transfer by returning 0 "pre-maturely" (i.e before the server expected it,
like when you've said you will upload N bytes and you upload less than N bytes), you may
experience that the server "hangs" waiting for the rest of the data that won't come.

The read callback may return CURL_READFUNC_ABORT to stop the current operation immediately,
resulting in a CURLE_ABORTED_BY_CALLBACK error code from the transfer.

The callback can return CURL_READFUNC_PAUSE to cause reading from this connection to pause.
See curl_easy_pause for further details.

Bugs: when doing TFTP uploads, you must return the exact amount of data that the callback wants,
or it will be considered the final packet by the server end and the transfer will end there.

If you set this callback pointer to NULL, or don't set it at all, the default internal read
function will be used. It is doing an fread() on the FILE * userdata set with CURLOPT_READDATA.
**/
size_t readCallback(char *buffer, size_t size, size_t nitems, void *instream)
{
    MemoryStruct *memoryStruce = (MemoryStruct*) instream;
    if (memoryStruce->size == 0)
        return 0;

    size_t realsize = size * nitems;
    if (realsize > memoryStruce->size)
        realsize = memoryStruce->size;

    memcpy(buffer, memoryStruce->memory, realsize);
    memoryStruce->memory += realsize;
    memoryStruce->size -= realsize;

    return realsize;
}

/**
Pass a pointer to your callback function, which should match the prototype shown above.

This function gets called by libcurl as soon as it has received header data. The header
callback will be called once for each header and only complete header lines are passed
on to the callback. Parsing headers is very easy using this. The size of the data pointed
to by buffer is size multiplied with nmemb. Do not assume that the header line is zero
terminated! The pointer named userdata is the one you set with the CURLOPT_HEADERDATA
option. This callback function must return the number of bytes actually taken care of.
If that amount differs from the amount passed in to your function, it'll signal an error
to the library. This will cause the transfer to get aborted and the libcurl function in
progress will return CURLE_WRITE_ERROR.

A complete HTTP header that is passed to this function can be up to
CURL_MAX_HTTP_HEADER(100K) bytes.

If this option is not set, or if it is set to NULL, but CURLOPT_HEADERDATA is set to
anything but NULL, the function used to accept response data will be used instead. That
is, it will be the function specified with CURLOPT_WRITEFUNCTION, or if it is not
specified or NULL - the default, stream-writing function.

It's important to note that the callback will be invoked for the headers of all responses
received after initiating a request and not just the final response. This includes all
responses which occur during authentication negotiation. If you need to operate on only
the headers from the final response, you will need to collect headers in the callback
yourself and use HTTP status lines, for example, to delimit response boundaries.

When a server sends a chunked encoded transfer, it may contain a trailer. That trailer
is identical to a HTTP header and if such a trailer is received it is passed to the
application using this callback as well. There are several ways to detect it being a
trailer and not an ordinary header: 1) it comes after the response-body. 2) it comes
after the final header line (CR LF) 3) a Trailer: header among the regular
response-headers mention what header(s) to expect in the trailer.

For non-HTTP protocols like FTP, POP3, IMAP and SMTP this function will get called with
the server responses to the commands that libcurl sends.
**/
size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    string headerName("X-Bs2-Filename");
    size_t len = size * nitems;
    if (len < headerName.size())
        return len;

    string header = string(buffer, len);
    if (header.compare(0, headerName.size(), headerName))
        return len;

    size_t pos = header.find(":", 0);
    if (pos == string::npos)
        return len;

    size_t headerValueLen = header.size() - pos + 1;
    MemoryStruct *memoryStruce = (MemoryStruct*) userdata;
    if (headerValueLen + 1 > memoryStruce->size)
        return len;

    memcpy(memoryStruce->memory, header.c_str() + pos + 1, headerValueLen);
    memoryStruce->memory[headerValueLen] = 0;
    memoryStruce->size = 0;

    return len;
}

CBs2Client::CBs2Client(const string &taskId, const string &uploadHost, const string &downloadHost,
                        const string &bucket, const string &accessKey, const string accessSecret)
{
    this->m_taskId = taskId;
    this->m_uploadHost = uploadHost;
    this->m_downloadHost = downloadHost;
    this->m_accessKey = accessKey;
    this->m_accessSecret = accessSecret;
    this->m_bucket = bucket;
}

CBs2Client::~CBs2Client()
{
}

bool CBs2Client::upload(const char *data, int len, string &filename, string &downloadUrl)
{
    if (data == NULL || len == 0)
    {
        FUNLOG(Debug, "%s BS2 data is empty, size=%d", m_taskId.c_str(), len);
        return false;
    }

    time_t rawtime;
    time (&rawtime);
    struct tm *timeinfo = gmtime(&rawtime);

    stringstream ss;
    ss << rawtime;
    string expires = ss.str();
    char dateHeaderBuf[256] = {0};
    strftime(dateHeaderBuf, sizeof(dateHeaderBuf), "Date: %a, %d %b %Y %H:%M:%S %Z", timeinfo);

	if (filename.size() == 0)
	{
		uuid_t uuid;
		uuid_generate(uuid);
		char uuid_str[37];
		uuid_unparse_lower(uuid, uuid_str);

		filename = std::string(uuid_str);
	}

    string auth = authorization(m_bucket, m_accessKey, m_accessSecret, filename, expires, "PUT");
    FUNLOG(Debug, "%s BS2 authorization=%s", m_taskId.c_str(), auth.c_str());

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        FUNLOG(Error, "%s Failed to init curl.", m_taskId.c_str());
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 15L);

    struct curl_slist *headerlist = NULL;
    string authHeader = "Authorization: " + auth;
    headerlist = curl_slist_append(headerlist, authHeader.c_str());
    headerlist = curl_slist_append(headerlist, dateHeaderBuf);
    headerlist = curl_slist_append(headerlist, "Content-Type: image/jpeg");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    MemoryStruct dataMemoryStruce;
    dataMemoryStruce.memory = (char*) data;
    dataMemoryStruce.size = len;
    curl_easy_setopt(curl, CURLOPT_READDATA, &dataMemoryStruce);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, len);

    /* tell it to "upload" to the URL */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);

    string url = "http://" + m_bucket + "." + m_uploadHost + "/" + filename;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // upload filename will genrate by BS2, read it from response header 'X-Bs2-Filename'
    char filenameBuf[256] = {0};
    if (filename.size() == 0)
    {
        MemoryStruct fnMemoryStruce;
        fnMemoryStruce.memory = filenameBuf;
        fnMemoryStruce.size = sizeof(filenameBuf);

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &fnMemoryStruce);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
    }

    /* enable verbose for easier tracing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    //set timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);

    // get status code
    int64_t http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    /* always cleanup */
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        FUNLOG(Error, "%s upload() failed. curl_easy_perform return %s", m_taskId.c_str(), curl_easy_strerror(res));
        return false;
    }

    if(http_code != 200)
    {
        FUNLOG(Error, "%s upload() failed. http_code return %ld", m_taskId.c_str(), http_code);
        return false;
    }

    if (filename.size() == 0)
    {
        filename = filenameBuf;
        filename = strip(filename);
    }

    downloadUrl = "http://" + m_bucket + "." + m_downloadHost + "/" + filename;
    return true;
}

bool CBs2Client::uploadAgain(const char *data, int len, uint32_t uid, uint32_t screenshotTs)
{
    if (data == NULL || len == 0)
        return false;

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        FUNLOG(Error, "%s Failed to init curl.", m_taskId.c_str());
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 15L);
    
    struct curl_slist *headerlist = NULL;
    headerlist = curl_slist_append(headerlist, "Content-Type: image/jpeg");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    MemoryStruct dataMemoryStruce;
    dataMemoryStruce.memory = (char*) data;
    dataMemoryStruce.size = len;
    curl_easy_setopt(curl, CURLOPT_READDATA, &dataMemoryStruce);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, len);

    /* tell it to "upload" to the URL */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);

    stringstream urlSs;
    urlSs << "http://videosnapshot.yy.com?uid=" << uid << "&time=" << screenshotTs;
    curl_easy_setopt(curl, CURLOPT_URL, urlSs.str().c_str());

    /* enable verbose for easier tracing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    //set timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    
    /* always cleanup */
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        FUNLOG(Error, "%s uploadAgain() failed. curl_easy_perform return %s", m_taskId.c_str(), curl_easy_strerror(res));
        return false;
    }

    return true;
}
