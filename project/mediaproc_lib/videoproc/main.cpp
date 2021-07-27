#include <unistd.h>
#include <stdlib.h>
#include <thread>

#include "gen/VideoProc.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "DecodeProc.h"
#include "core/logger.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::videolib;

class VideoProcHandler : virtual public VideoProcIf {
public:
    VideoProcHandler() {
        // Your initialization goes here
    }

    virtual void cutFrame(CutFrameResult &_return, const VideoDataInput &mediaData, const CutConfig &config) {
        _return.dataId = mediaData.dataId;
        std::vector<CutFrameInfo> infos;
        _return.succ = ::cutFrames(mediaData.dataId, mediaData.localFile, infos,
                                   0, config.interval, config.maxFrames, _return.duration);
        for (auto &i : infos) {
            FrameInfo r;
            r.content = i.content;
            r.timeAt = i.timeAt;
            _return.imgs.push_back(std::move(r));
        }
        FUNLOG(Notice, "input dataID %s, interval %d, maxFrames %d, output succ %d size %zu",
               mediaData.dataId.c_str(), config.interval, config.maxFrames,
               _return.succ, _return.imgs.size());
    }
};

int main(int argc, const char **argv) {
    init_log();
    if (argc != 2) {
        FUNLOG(Warn, "input args error");
        return -1;
    }
    const char *domain = argv[1];

    stdcxx::shared_ptr<VideoProcHandler> handler(new VideoProcHandler());
    stdcxx::shared_ptr<TProcessor> processor(new VideoProcProcessor(handler));
    stdcxx::shared_ptr<TServerTransport> serverTransport(new TServerSocket(domain));
    stdcxx::shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
    stdcxx::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    FUNLOG(Notice, "videoproc have start, domain: %s", domain);
    TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();
    return 0;
}