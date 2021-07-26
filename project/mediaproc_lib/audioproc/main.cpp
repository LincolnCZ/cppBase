#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <algorithm>

#include "gencpp/AudioProc.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "AudioProcessor.h"
#include "core/logger.h"
#include "core/utils.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::audiolib;

static int calcInterval(double duration, int lower, int upper) {
    int d = ceil(duration);
    int maxMod = 0;
    int interval = lower;
    for (int i = lower; i < upper; i++) {
        int mod = d % i;
        if (mod >= maxMod) {
            maxMod = mod;
            interval = i;
        }
    }
    return interval;
}

class AudioProcHandler : virtual public AudioProcIf {
public:
    AudioProcHandler() {
    }

    virtual void
    splitAudio(SplitResult &_return, const AudioDataInput &mediaData, const int32_t lower, const int32_t upper) {
        AudioProcessor processor;
        if (!initAudioProcessor(processor, mediaData)) {
            FUNLOG(Warn, "%s, init fail", mediaData.dataId.c_str());
            _return.succ = false;
            return;
        }

        _return.dataId = mediaData.dataId;
        _return.duration = processor.calcDuration();
        // 动态调整分割长度
        // 使得最后一个切片的长度最大
        int segtime = lower;
        if (lower < upper) {
            segtime = calcInterval(_return.duration, lower, upper);
        }
        FUNLOG(Notice, "%s, calcInterval %d-%d format '%s' duration %lf segtime %d", mediaData.dataId.c_str(),
               lower, upper, processor.formatName(), _return.duration, segtime);

        _return.succ = processor.splitAudio(_return.contents, segtime);
        _return.segtime = segtime;
        FUNLOG(Notice, "%s, output succ %d size %zu duration %lf", mediaData.dataId.c_str(),
               _return.succ, _return.contents.size(), _return.duration);
    }

    virtual void decodeAudio(AudioOutput &_return, const AudioDataInput &mediaData, const int32_t sampleRate,
                             const int32_t channels) {
        AudioProcessor processor;
        if (!initAudioProcessor(processor, mediaData)) {
            FUNLOG(Warn, "%s, init fail", mediaData.dataId.c_str());
            _return.succ = false;
            return;
        }
        FUNLOG(Notice, "decode dataID %s, sampleRate %d, channel %d", mediaData.dataId.c_str(), sampleRate, channels);

        _return.format = "pcm";
        _return.dataId = mediaData.dataId;
        _return.succ = processor.decodeAudio(sampleRate, channels, _return.content);
        _return.duration = processor.duration();
        FUNLOG(Notice, "result dataID %s, output succ %d, duration %lf", mediaData.dataId.c_str(), _return.succ,
               _return.duration);
    }

    virtual void convertAudio(AudioOutput &_return, const AudioDataInput &mediaData, const int32_t sampleRate,
                              const int32_t channels, const std::string &format) {
        AudioProcessor processor;
        if (!initAudioProcessor(processor, mediaData)) {
            FUNLOG(Warn, "%s, init fail", mediaData.dataId.c_str());
            _return.succ = false;
            return;
        }
        FUNLOG(Notice, "convert dataID %s, sampleRate %d, channel %d, format %s", mediaData.dataId.c_str(), sampleRate,
               channels, format.c_str());
        _return.format = format;
        if (format.empty()) {
            _return.format = "mp4";
        }

        _return.dataId = mediaData.dataId;
        _return.succ = processor.convertAudio(sampleRate, channels, _return.format, _return.content);
        _return.duration = processor.duration();
        FUNLOG(Notice, "result dataID %s, output succ %d, duration %lf", mediaData.dataId.c_str(), _return.succ,
               _return.duration);
    }

private:
    bool initAudioProcessor(AudioProcessor &processor, const AudioDataInput &mediaData) {
        bool inputPath = false;
        if (mediaData.dataType == "PATH") {
            FUNLOG(Notice, "input dataID %s path %s", mediaData.dataId.c_str(), mediaData.content.c_str());
            inputPath = true;
        } else {
            FUNLOG(Notice, "input dataID %s binary", mediaData.dataId.c_str());
        }
        return processor.init(mediaData.dataId.c_str(), mediaData.content, inputPath);
    }
};

int main(int argc, const char **argv) {
    init_log();
    if (argc != 2) {
        FUNLOG(Warn, "input args error");
        return -1;
    }
    const char *domain = argv[1];

    stdcxx::shared_ptr<AudioProcHandler> handler(new AudioProcHandler());
    stdcxx::shared_ptr<TProcessor> processor(new AudioProcProcessor(handler));
    stdcxx::shared_ptr<TServerTransport> serverTransport(new TServerSocket(domain));
    stdcxx::shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
    stdcxx::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    FUNLOG(Notice, "audioproc have start, domain: %s", domain);
    TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();
    return 0;
}