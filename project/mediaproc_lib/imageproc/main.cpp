#include <stdio.h>
#include <string.h>
#include <webp/decode.h>

#include "gencpp/ImageProc.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;

#include "core/logger.h"
#include "imageproc.h"

class ImageProcHandler : public imagelib::ImageProcIf {
public:
    ImageProcHandler() {}

    virtual void splitWebp(imagelib::SplitResult &_return, const imagelib::ImageInput &mediaData) {
        _return.dataId = mediaData.dataId;
        _return.succ = (0 == splitImageWebp(mediaData.dataId.c_str(), mediaData.content, _return.imgs));
    }
};

int main(int argc, const char **argv) {
    init_log();
    if (argc != 2) {
        printf("main socket_file\n");
        return 1;
    }

    const char *domain = argv[1];
    FUNLOG(Notice, "audioproc start, domain: %s", domain);
    stdcxx::shared_ptr<ImageProcHandler> handler(new ImageProcHandler());
    stdcxx::shared_ptr<TProcessor> processor(new imagelib::ImageProcProcessor(handler));
    stdcxx::shared_ptr<transport::TServerTransport> serverTransport(new transport::TServerSocket(domain));
    stdcxx::shared_ptr<transport::TTransportFactory> transportFactory(new transport::TFramedTransportFactory());
    stdcxx::shared_ptr<protocol::TProtocolFactory> protocolFactory(new protocol::TBinaryProtocolFactory());

    server::TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();
    return 0;
}
