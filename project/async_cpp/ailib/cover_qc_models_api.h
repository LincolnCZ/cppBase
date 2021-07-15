#ifndef _YYAI_COVER_QC_MODELS_API_H_
#define _YYAI_COVER_QC_MODELS_API_H_

#include <string>
#include <vector>

namespace yyai {

struct conf_param_t {
    /**
     * all models dir
     **/
    std::string models_dir;
    /**
     * GPU device id
     **/
    int gpu_id;

    /**
     * max models will be load in this process, 
     * usually it will be a little smaller than the process thread count
     **/
    int model_pool_size;

    conf_param_t() {
        models_dir = "./models";
        gpu_id = 0;
        model_pool_size = 4;
    }
};

class CoverQualityModelsApiImpl;

class CoverQualityModelsApi {
public:
    CoverQualityModelsApi(conf_param_t& conf_params);
    ~CoverQualityModelsApi();

    /**
     * do the inference job
     * 
     * @param cover_id      id of this image
     * @param cover_image   path of this image
     * @param results       full output of these models
     * 
     * @return 0 for success, other for failed
     **/
    int inference(const std::string& cover_id, const std::string& cover_image, std::string& results_json);

private:
    CoverQualityModelsApiImpl* _ptr_impl;

};

};

#endif