#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <unistd.h>
#include "audioproc/AudioProcessor.h"
#include "videoproc/Decode.h"
#include "imageproc/imageproc.h"
#include "core/logger.h"
#include "core/utils.h"
extern "C" {
    #include <libavutil/avutil.h>
}

void cmdDecode(int argc, char **argv)
{
    std::string in_file;
    std::string out_file;
    char ch;
    while(-1 != (ch = getopt(argc, argv, "i:o:")))
    {
        switch (ch)
        {
        case 'o':
            out_file = optarg;
            break;
        case 'i':
            in_file = optarg;
            break;
        }
    }
    if(in_file.empty() || out_file.empty()) {
        printf("need input file and output file\n");
        return;
    }

    AudioProcessor processor;
    if(!processor.init("test", in_file, true)) {
        printf("init processor %s error\n", in_file.c_str());
        return;
    }
    printf("decode start: file %s to %s\n", in_file.c_str(), out_file.c_str());
    std::string outbuf;
    if(!processor.decodeAudio(16000, 1, outbuf)) {
        printf("decode fail\n");
        return;
    }
    double duration = processor.duration();
    printf("decode success, duration %lf\n", duration);
    utility::writeFile(out_file.c_str(), outbuf);
}

void cmdConvert(int argc, char **argv)
{
    std::string in_file;
    std::string out_file;
    std::string format = "mp4";
    char ch;
    while(-1 != (ch = getopt(argc, argv, "i:o:f:")))
    {
        switch (ch)
        {
        case 'o':
            out_file = optarg;
            break;
        case 'i':
            in_file = optarg;
            break;
        case 'f':
            format = optarg;
            break;
        }
    }
    if(in_file.empty() || out_file.empty()) {
        printf("need input file and output file\n");
        return;
    }

    AudioProcessor processor;
    if(!processor.init("test", in_file, true)) {
        printf("init processor %s error\n", in_file.c_str());
        return;
    }
    printf("convert start: file %s to %s\n", in_file.c_str(), out_file.c_str());
    std::string outbuf;
    if(!processor.convertAudio(16000, 1, format, outbuf)) {
        printf("convert fail\n");
        return;
    }
    double duration = processor.duration();
    printf("convert success, duration %lf\n", duration);
    utility::writeFile(out_file.c_str(), outbuf);
}

void cmdSplit(int argc, char **argv)
{
    std::string in_file;
    int segtime = 5;
    char ch;
    while(-1 != (ch = getopt(argc, argv, "i:s:")))
    {
        switch (ch)
        {
        case 's':
            segtime = atoi(optarg);
            break;
        case 'i':
            in_file = optarg;
            break;
        }
    }
    if(segtime <= 0) {
        printf("segtime error: %d\n", segtime);
        return;
    }

    AudioProcessor processor;
    if(!processor.init("test", in_file, true)) {
        printf("init processor %s error\n", in_file.c_str());
        return;
    }
    double duration = processor.calcDuration();
    printf("split start, input %s segtime %d, duration %lf\n", in_file.c_str(), segtime, duration);

    std::vector<std::string> outbuf;
    if(!processor.splitAudio(outbuf, segtime)) {
        printf("convert fail\n");
        return;
    }
    printf("split success, output %zd\n", outbuf.size());
    for(size_t i = 0; i < outbuf.size(); i++) {
        char out[64];
        snprintf(out, sizeof(out), "%ld.m4a", i);
        utility::writeFile(out, outbuf[i]);
    }
}

void cmdScreenShot(int argc, char **argv)
{
    std::string in_file;
    int segtime = 5;
    char ch;
    while(-1 != (ch = getopt(argc, argv, "i:s:")))
    {
        switch (ch)
        {
        case 's':
            segtime = atoi(optarg);
            break;
        case 'i':
            in_file = optarg;
            break;
        }
    }
    if(segtime <= 0) {
        printf("segtime error: %d\n", segtime);
        return;
    }
    printf("screenshot start, input %s segtime %d\n", in_file.c_str(), segtime);

    std::vector<CutFrameInfo> output;
    double duration;
    bool r = cutFrames("test", in_file, output, 0, segtime, 100, duration);
    if(!r) {
        printf("screenshot fail\n");
        return;
    }
    printf("screenshot success, output %zd\n", output.size());
    for(size_t i = 0; i < output.size(); i++) {
        char out[64];
        snprintf(out, sizeof(out), "%ld.jpg", i);
        utility::writeFile(out, output[i].content);
    }
}

void cmdImage(int argc, char **argv)
{
    std::string in_file;
    char ch;
    while(-1 != (ch = getopt(argc, argv, "i:s:")))
    {
        switch (ch)
        {
        case 'i':
            in_file = optarg;
            break;
        }
    }
    std::string input;
    if(!utility::readFile(in_file.c_str(), input)) {
        printf("image file read fail, %s\n", in_file.c_str());
        return;
    }
    printf("image, input %s\n", in_file.c_str());

    std::vector<std::string> output;
    int r = splitImageWebp("test", input, output);
    if(r != 0) {
        printf("screenshot fail\n");
        return;
    }
    printf("image success, output %zd\n", output.size());
    for(size_t i = 0; i < output.size(); i++) {
        char out[64];
        snprintf(out, sizeof(out), "%ld.png", i);
        utility::writeFile(out, output[i]);
    }
}

void cmdError(int argc, char **argv)
{
    std::string in_file;
    int error = 0;
    error = strtol(argv[2], NULL, 10);

    char errbuf[256];
    av_make_error_string(errbuf, sizeof(errbuf), error);
    printf("error %d, message %s\n", error, errbuf);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        printf("%s command [param]\n", argv[0]);
        return 0;
    }
    const char *cmd = argv[1];
    if(strcmp(cmd, "decode") == 0) {
        cmdDecode(argc, argv);
    } else if(strcmp(cmd, "split") == 0) {
        cmdSplit(argc, argv);
    } else if(strcmp(cmd, "convert") == 0) {
        cmdConvert(argc, argv);
    } else if(strcmp(cmd, "error") == 0) {
        cmdError(argc, argv);
    } else if(strcmp(cmd, "screenshot") == 0) {
        cmdScreenShot(argc, argv);
    } else if(strcmp(cmd, "image") == 0) {
        cmdImage(argc, argv);
    } else {
        printf("unknown cmd: %s\n", cmd);
    }
    return 0;
}
