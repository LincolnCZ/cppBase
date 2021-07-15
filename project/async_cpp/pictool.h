#ifndef _PICTURE_TOOL_H_
#define _PICTURE_TOOL_H_

#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;

void get_video_info(const char *filepath, std::string &dpi, int &duration, int &width, int &height) {
    VideoCapture cap(filepath);
    width = int(cap.get(CAP_PROP_FRAME_WIDTH));
    height = int(cap.get(CAP_PROP_FRAME_HEIGHT));
    int frame_count = cap.get(CAP_PROP_FRAME_COUNT);
    float fps = cap.get(CAP_PROP_FPS);
    char buff[32] = {0};
    sprintf(buff, "%d*%d", width, height);
    dpi = std::string(buff);
    duration = int(ceil(frame_count / fps));
    cap.release();
}

bool cut_pic(const char *src, const char *dst, int width, int height) {
    Mat image = imread(src);
    if (image.empty()) {
        return false;
    }

    int src_width = image.size().width;
    int src_height = image.size().height;
    int dst_width = src_width;
    int dst_height = int(round(src_width * (1.0 * height / width)));
    if (dst_height > src_height) {
        dst_height = src_height;
        dst_width = dst_height * (1.0 * width / height);
    }

    int x = (src_width - dst_width) / 2;
    int y = (src_height - dst_height) / 2;
    Mat ROI(image, cv::Rect(x, y, dst_width, dst_height));
    Mat croppedImage;

    // Copy the data into new matrix
    ROI.copyTo(croppedImage);
    imwrite(dst, croppedImage);
    return true;
}

#endif

