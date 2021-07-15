#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <curl/curl.h>
#include <uuid/uuid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

#include  "logger.h"

//long getCurrentTimeMicro() {
//    struct timeval currentTime;
//    gettimeofday(&currentTime, NULL);
//    return currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec;
//}

//
//inline bool checkFileExist(std::string file_name) {
//    std::ifstream std_file(file_name);
//    return std_file.good();
//}

//bool isFileExist(const char* file_path)
//{
//    struct stat buffer;
//    return (stat(file_path, &buffer) == 0);
//}

bool isDirExist(const char *dir_path) {
    if (NULL == dir_path)
        return false;

    DIR *dp = opendir(dir_path);
    if (NULL == dp)
        return false;

    closedir(dp);
    return true;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int http_download(const char *traceid, const char *url, const char *outfilename, const char *http_proxy,
                  const char *https_proxy) {
    CURL *curl = NULL;
    FILE *fp = NULL;
    CURLcode res;

    //res = curl_global_init(CURL_GLOBAL_ALL);
    //if (CURLE_OK != res)
    //{
    //	FUNLOG(Err, "%s init libcurl failed.", traceid);
    //	curl_global_cleanup();
    //	return -1;
    //}

    curl = curl_easy_init();
    if (NULL == curl) {
        FUNLOG(Err, "%s curl_easy_init failed.", traceid);
        return -1;
    }

    int ret = 0;
    do {
        fp = fopen(outfilename, "wb");
        if (NULL == fp) {
            ret = -2;
            FUNLOG(Err, "%s openfile failed. dst file: %s", traceid, outfilename);
            break;
        }

        std::string url_str = std::string(url);
        std::size_t found = url_str.find("goog.yy.com");
        if (found != std::string::npos) {
            found = url_str.find("https:");
            if (found != std::string::npos) {
                if (strcmp(https_proxy, "") != 0) {
                    FUNLOG(Info, "%s use https proxy: %s", traceid, https_proxy);
                    res = curl_easy_setopt(curl, CURLOPT_PROXY, https_proxy);
                    if (res != CURLE_OK) {
                        FUNLOG(Err, "%s set proxy fail! %s", traceid, curl_easy_strerror(res));
                    }
                }
            } else {
                if (strcmp(http_proxy, "") != 0) {
                    FUNLOG(Info, "%s use http proxy: %s", traceid, http_proxy);
                    res = curl_easy_setopt(curl, CURLOPT_PROXY, http_proxy);
                    if (res != CURLE_OK) {
                        FUNLOG(Err, "%s set proxy fail! %s", traceid, curl_easy_strerror(res));
                    }
                }
            }
        }

        res = curl_easy_setopt(curl, CURLOPT_URL, url);
        if (res != CURLE_OK) {
            FUNLOG(Err, "%s curl_easy_setopt CURLOPT_URL fail! %s", traceid, curl_easy_strerror(res));
            ret = -3;
            break;
        }
        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        if (res != CURLE_OK) {
            FUNLOG(Err, "%s curl_easy_setopt CURLOPT_WRITEFUNCTION fail! %s", traceid, curl_easy_strerror(res));
            ret = -4;
            break;
        }

        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        if (res != CURLE_OK) {
            FUNLOG(Err, "%s curl_easy_setopt CURLOPT_WRITEDATA fail! %s", traceid, curl_easy_strerror(res));
            ret = -5;
            break;
        }

        //fix mutil pthread core dump problem
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);    /*no signal,add by edgeyang*/
        //set timeout
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

        res = curl_easy_perform(curl);
        fclose(fp);
        fp = NULL;
        /* Check for errors */
        if (res != CURLE_OK) {
            ret = -6;
            FUNLOG(Err, "%s curl_easy_perform failed: %s", traceid, curl_easy_strerror(res));
            break;
        }

        ret = 0;
    } while (false);

    if (NULL != fp) {
        fclose(fp);
        fp = NULL;
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    //curl_global_cleanup();
    return ret;
}

std::string getUuid() {
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37] = {0};
    uuid_unparse_lower(uuid, uuid_str);
    return std::string(uuid_str);
}

int getFileSize(const char *file_path) {
    if (NULL == file_path) {
        FUNLOG(Err, "getFileSize file_path is NULL");
        return -1;
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        FUNLOG(Err, "getFileSize can not open file: %s", file_path);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    fclose(fp);
    return fileSize;
}

bool isFileExist(const char *file_path) {
    return 0 == access(file_path, F_OK);
}

int remove_dir(const char *dir_path) {
    if (0 != access(dir_path, F_OK)) {
        return 0;
    }

    struct stat dir_stat;
    if (0 > stat(dir_path, &dir_stat)) {
        FUNLOG(Err, "get dir stat fail!");
        return -1;
    }

    if (S_ISREG(dir_stat.st_mode)) {
        FUNLOG(Info, "try delete file: %s", dir_path);
        remove(dir_path);
    } else if (S_ISDIR(dir_stat.st_mode)) {
        DIR *dirp = opendir(dir_path);
        dirent *dp;
        while ((dp = readdir(dirp)) != NULL) {
            if ((0 == strcmp(dp->d_name, ".")) || (0 == strcmp(dp->d_name, ".."))) {
                continue;
            }

            char sub_dir[256] = {0};
            sprintf(sub_dir, "%s/%s", dir_path, dp->d_name);
            remove_dir(sub_dir);
        }
        closedir(dirp);
        rmdir(dir_path);
    }

    return 0;
}

#endif

