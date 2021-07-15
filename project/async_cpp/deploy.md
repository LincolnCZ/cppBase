本文档主要讲解如何在一台新的机器上部署视频换脸服务，主要包括以下几点操作，注意服务包必须在最后一步安装。


#### 1. 安装驱动及cuda10.1

目前部分机器上已经有了相关的安装包，可以通过nc命令复制到其他需要安装的机器上。

例如在 10.31.4.109 机器上的/data/luofeilong1目录下

```
root@ubuntu:/data/luofeilong1# ll
total 2677760
-rwxr-xr-x 1 root root 2572375299 Sep 26 20:10 cuda_10.1.243_418.87.00_linux.run
-rw-r--r-- 1 root root  169642466 Sep 26 20:11 libcudnn7_7.5.0.56-1+cuda10.1_amd64.deb
```

需要先安装cuda文件，注意就算原机器已经有驱动和cuda，都需要重新安装一次
```
sudo sh ./cuda_10.1.243_418.87.00_linux.run
```

安装完成cuda后，继续安装cudnn
```
sudo dpkg -i libcudnn7_7.5.0.56-1+cuda10.1_amd64.deb
```

都安装完成后，可使用 nvidia-smi 命令查看到GPU信息即可。

#### 2. 安装hosts_manager包

在包发布系统中，搜索 hosts_manager 包，安装最新版本，配置文件选择 “biugo视频换脸 - 谷歌云域名映射” 即可。

注意：只有国内机器才需要安装这个包，如果是海外机器则不用安装该包。

#### 3. 安装libtorch、opencv等依赖包

在包发布系统中，搜索 aladdin_video_swap_face_depends 包，该包只包含了服务运行所依赖的libtorch和opencv库文件。
安装最新版本，配置文件选择默认即可。

注意：该包的lib目录中同时包含了of库的so文件libof_effect.so，更新版本时需要跟AI同学沟通是否需要更新of库，如果需要则需要重新打包发布。

#### 4. 安装素材包

在包发布系统中，搜索 aladdin_model_video_swap_face_v2 包，安装最新版本，配置选择默认即可。
该包只包含了素材视频相关文件，只有运营要求更新时才需要更新。

注意：后面应该会采用 rsync的方式从svn增量拉去素材，提高更新素材的效率。

#### 5. 安装服务包

在包发布系统中，搜索 aladdin_video_swap_face_v2 包，正式环境v100 8卡机器使用 “正式环境 - v100 8卡” 配置进行安装，如果遇到非8卡机器，则需要重新新建配置，调整配置文件params.conf中使用的卡数量，里面每一行对应一个进程的命令参数，例如：

```
-m /data/services/ai_models/video_swap_face_model_v2 -d 0 -s 1 -p1
-m /data/services/ai_models/video_swap_face_model_v2 -d 0 -s 1 -p2
-m /data/services/ai_models/video_swap_face_model_v2 -d 1 -s 1 -p3
-m /data/services/ai_models/video_swap_face_model_v2 -d 1 -s 1 -p4
-m /data/services/ai_models/video_swap_face_model_v2 -d 2 -s 1 -p5
-m /data/services/ai_models/video_swap_face_model_v2 -d 2 -s 1 -p6
-m /data/services/ai_models/video_swap_face_model_v2 -d 3 -s 1 -p7
-m /data/services/ai_models/video_swap_face_model_v2 -d 3 -s 1 -p8
-m /data/services/ai_models/video_swap_face_model_v2 -d 4 -s 1 -p9
-m /data/services/ai_models/video_swap_face_model_v2 -d 4 -s 1 -p10
-m /data/services/ai_models/video_swap_face_model_v2 -d 5 -s 1 -p11
-m /data/services/ai_models/video_swap_face_model_v2 -d 5 -s 1 -p12
-m /data/services/ai_models/video_swap_face_model_v2 -d 6 -s 1 -p13
-m /data/services/ai_models/video_swap_face_model_v2 -d 6 -s 1 -p14
-m /data/services/ai_models/video_swap_face_model_v2 -d 7 -s 1 -p15
-m /data/services/ai_models/video_swap_face_model_v2 -d 7 -s 1 -p16
```

```
-m 参数用于指定素材所在目录
-d 参数用于指定使用第几张卡
-s 参数用于设置任务失败时不删除本地的临时目录，方便排查问题
-px 参数只是为了令到每一行参数唯一，否则启动脚本会当做同一个进程
```