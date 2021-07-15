#include "googleclient.h"
#include "logger.h"

GoogleClient::GoogleClient() {
    m_init = false;
}

bool GoogleClient::init(const char *data) {
    if (NULL == data) {
        FUNLOG(Err, "data is null");
        return false;
    }

    //m_cret = google::cloud::storage::oauth2::CreateServiceAccountCredentialsFromJsonFilePath(cret_file_path);
    m_cret = google::cloud::storage::oauth2::CreateServiceAccountCredentialsFromJsonContents(data);
    m_client = google::cloud::StatusOr<google::cloud::storage::Client>(google::cloud::storage::Client(*m_cret));
    if (!m_client) {
        FUNLOG(Err, "Failed to create Storage Client, status=%s", m_client.status().message().c_str());
        return false;
    }

    m_init = true;
    return true;
}

int GoogleClient::upload(const char *bucket, const char *local_file_path, const char *cloud_save_name,
                         const char *content_type) {
    if (!m_init) {
        FUNLOG(Err, "has not init now!");
        return 1;
    }

    FUNLOG(Info, "try to upload %s to google, bucket: %s, save as: %s", local_file_path, bucket, cloud_save_name);

    auto file_name = local_file_path;//ConsumeArg(argc, argv);
    auto bucket_name = bucket;//ConsumeArg(argc, argv);
    auto object_name = cloud_save_name; //ConsumeArg(argc, argv);

    namespace gcs = google::cloud::storage;
    using ::google::cloud::StatusOr;

    google::cloud::storage::Client client = *m_client;
    StatusOr <gcs::ObjectMetadata> object_metadata = client.UploadFile(file_name, bucket_name,
            //object_name, gcs::IfGenerationMatch(0), gcs::IfGenerationNotMatch(0), gcs::ContentType(content_type),
                                                                       object_name, gcs::ContentType(content_type),
                                                                       gcs::NewResumableUploadSession());

    if (!object_metadata) {
        FUNLOG(Err, "upload fail! msg=%s", object_metadata.status().message().c_str());
        return 2;
    }

    return 0;
}


