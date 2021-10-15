[toc]

## 资料
* 最简单的视音频播放示例系列文章列表：
  * https://blog.csdn.net/leixiaohua1020/article/details/40246783
* 雷博士GitHub
  * https://github.com/leixiaohua1020
* FFMPEG视音频编解码零基础学习方法
  * https://blog.csdn.net/leixiaohua1020/article/details/15811977?spm=1001.2014.3001.5502


## 生成对应YUV数据：

```
ffmpeg -pix_fmt yuv420p -s 1000x667 -i tiger.jpeg tiger_yuv420p_1000x667.yuv
```
* -pix_fmt：表示要用什么格式转换，yuv420p是参数，你也可以通过ffmpeg -pix_fmts查看其它支持的类型。
* -s：表示一帧的尺寸，这个尺寸将是你生成yuv数据的宽和高，需要牢牢记住，因为转换成yuv数据后，数据将不会存储任何无关信息，包括尺寸。
* -i：很简单，就是需要转换的文件，i表示input。参数不仅可以是jpg格式的，也可以是bmp等任何其它常见类型。
* tiger_yuv420p_1000x667.yuv：我的输出文件名，在文件名中，记录了yuv的格式，和尺寸，这些信息在显示过程中比较重要。

## 提取pcm

```bash
ffprobe My_Heart_Will_Go_on.mp3
ffprobe version 4.4 Copyright (c) 2007-2021 the FFmpeg developers
  built with Apple clang version 12.0.5 (clang-1205.0.22.9)
  configuration: --prefix=/usr/local/Cellar/ffmpeg/4.4_2 --enable-shared --enable-pthreads --enable-version3 --cc=clang --host-cflags= --host-ldflags= --enable-ffplay --enable-gnutls --enable-gpl --enable-libaom --enable-libbluray --enable-libdav1d --enable-libmp3lame --enable-libopus --enable-librav1e --enable-librubberband --enable-libsnappy --enable-libsrt --enable-libtesseract --enable-libtheora --enable-libvidstab --enable-libvorbis --enable-libvpx --enable-libwebp --enable-libx264 --enable-libx265 --enable-libxml2 --enable-libxvid --enable-lzma --enable-libfontconfig --enable-libfreetype --enable-frei0r --enable-libass --enable-libopencore-amrnb --enable-libopencore-amrwb --enable-libopenjpeg --enable-libspeex --enable-libsoxr --enable-libzmq --enable-libzimg --disable-libjack --disable-indev=jack --enable-avresample --enable-videotoolbox
  libavutil      56. 70.100 / 56. 70.100
  libavcodec     58.134.100 / 58.134.100
  libavformat    58. 76.100 / 58. 76.100
  libavdevice    58. 13.100 / 58. 13.100
  libavfilter     7.110.100 /  7.110.100
  libavresample   4.  0.  0 /  4.  0.  0
  libswscale      5.  9.100 /  5.  9.100
  libswresample   3.  9.100 /  3.  9.100
  libpostproc    55.  9.100 / 55.  9.100
Input #0, mp3, from 'My_Heart_Will_Go_on.mp3':
  Metadata:
    title           : My Heart Will Go On
    artist          : Céline Dion,James Horner
    album           : Titanic: Original Motion Picture Soundtrack - Anniversary Edition
  Duration: 00:05:10.75, start: 0.025057, bitrate: 320 kb/s
  Stream #0:0: Audio: mp3, 44100 Hz, stereo, fltp, 320 kb/s
    Metadata:
      encoder         : LAME3.97
    Side data:
      replaygain: track gain - 2.100000, track peak - unknown, album gain - unknown, album peak - unknown,
```
可以用到的音频基本数据为：

* 声道数：stereo（双声道）
* 采样率为：44100Hz
* 音频格式为：fltp

下面使用ffmpeg提取pcm数据

```
ffmpeg -y -i MyHeartWillGoOn.mp3 -acodec pcm_s16le -f s16le -ac 2 -ar 44100 MyHeartWillGoOn.pcm
```
* -y	允许覆盖
* -i Forevermore.mp3	源文件
* -acodec pcm_s16le	编码器
* -f s16le	强制文件格式
* -ac 2	双声道
* -ar 44100	采样率

* 用ffplay播放一下：

```
ffplay -ar 44100 -channels 2 -f s16le -i My_Heart_Will_Go_on.pcm
```
可以正常播放，没啥问题。


