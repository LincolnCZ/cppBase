.PHONY: audio_lib_proto
audio_lib_proto:
	thrift --gen cpp:no_skeleton -out audioproc/gencpp/ audiolib.thrift
	thrift --gen go:thrift_import=github.com/apache/thrift/lib/go/thrift -out gogen/ audiolib.thrift
	rm -rf gogen/audiolib/audio_proc-remote

.PHONY: video_lib_proto
video_lib_proto:
	thrift --gen cpp:no_skeleton -out videoproc/gen/ videolib.thrift
	thrift --gen go:thrift_import=github.com/apache/thrift/lib/go/thrift -out gogen/ videolib.thrift
	rm -rf gogen/videolib/video_proc-remote

.PHONY: image_lib_proto
image_lib_proto:
	thrift --gen cpp:no_skeleton -out imageproc/gencpp/ imagelib.thrift
	thrift --gen go:thrift_import=github.com/apache/thrift/lib/go/thrift -out gogen/ imagelib.thrift
	rm -rf gogen/imagelib/image_proc-remote
