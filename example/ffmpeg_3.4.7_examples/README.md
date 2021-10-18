FFmpeg examples README
----------------------

Both following use cases rely on pkg-config and make, thus make sure
that you have them installed and working on your system.


Method 1: build the installed examples in a generic read/write user directory

Copy to a read/write user directory and just use "make", it will link
to the libraries on your system, assuming the PKG_CONFIG_PATH is
correctly configured.

Method 2: build the examples in-tree

Assuming you are in the source FFmpeg checkout directory, you need to build
FFmpeg (no need to make install in any prefix). Then just run "make examples".
This will build the examples using the FFmpeg build system. You can clean those
examples using "make examplesclean"

If you want to try the dedicated Makefile examples (to emulate the first
method), go into doc/examples and run a command such as
PKG_CONFIG_PATH=pc-uninstalled make.

例子说明：

* transcode_aac ：Convert an input audio file to AAC in an MP4 container using FFmpeg.
* transcoding ：API example for demuxing, decoding, filtering, encoding and muxing.
    涉及音频、视频

* muxing ：libavformat API example. 音视频流封装。
    * Output a media file in any supported libavformat format. The default codecs are used.
* demuxing_decoding ：Demuxing and decoding example. 音视频文件解封装。
* remuxing ：音视频文件转封装。视频截取。
* avio_reading ：avio内存数据操作。
* avio_reading ：avio内存数据操作。


https://www.bilibili.com/read/cv12654758