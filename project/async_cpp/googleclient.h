#ifndef _GOOGLE_CLIENT_H
#define _GOOGLE_CLIENT_H

#include <string>

#include "google/cloud/storage/client.h"
#include "google/cloud/storage/oauth2/google_credentials.h"
#include "google/cloud/storage/oauth2/credentials.h"
#include "google/cloud/storage/well_known_parameters.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class GoogleClient {
public:
    GoogleClient();

    bool init(const char *data);

    int upload(const char *bucket, const char *local_file_path, const char *cloud_save_name, const char *content_type);

private:
    bool m_init;
    google::cloud::StatusOr <google::cloud::storage::Client> m_client;
    google::cloud::StatusOr <std::shared_ptr<google::cloud::storage::oauth2::Credentials>> m_cret;
};

#endif
