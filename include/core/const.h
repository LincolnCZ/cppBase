#pragma once

enum ISPType {
    AUTO_DETECT = 0,
    CTL       = 1,    //电信
    CNC       = 2,    //网通
    MUTIL     = 3,    //双线
    CNII      = 4,    //铁通
    EDU       = 8,    //教育网
    INTRANET = 10,    //内网IP
    WBN       = 16,   //长城宽带
    MOB       = 32,   //移动
    BGP       = 64,   //BGP
    HK        = 128,  //香港
    BRA       = 256,  //巴西
    EU        = 512,  //欧洲
    NA        = 1024, //北美
    MAX_ISP   = 1024,  //THIS MUST BE THE MAXIMUM NUMBER AMONG AVAILABLE ISPS!!!
};


enum AreaType { //同网络类型的定义数字必需连续(分配的时候会用)
    AREA_UNKNOWN  = 0,
    CTL_EAST      = 16,     //电信东区     10000  (1<<4) + 0
    CTL_WEST      = 17,     //电信西区     10001  (1<<4) + 1
    CTL_SOUTH     = 18,     //电信南区     10010  (1<<4) + 2
    CTL_NORTH     = 19,     //电信北区     10011  (1<<4) + 3
    CNC_NE        = 32,     //网通东北     100000 (2<<4) + 0
    CNC_NC        = 33,     //网通华北     100001 (2<<4) + 1
    CNC_SC        = 34,     //网通南方     100010 (2<<4) + 2
    CNII_AREA     = 64,     //铁通        1000000 (4<<4) + 1
    EDU_AREA      = 128,    //教育网     10000000 (8<<4) + 0
    WBN_AREA      = 256,    //长城宽带 100000000 (16<<4) + 0
    MOB_AREA      = 512,    //移动    1000000000 (32<<4) + 0
    BGP_AREA      = 1024,   //BGP     10000000000 (64<<4) + 0
    HK_AREA       = 2048,   //香港分区
    PH_AREA       = 2049,   //菲律宾分区
    TW_AREA	= 2050,   //台湾分区
    BRA_AREA      = 4096,   //巴西分区
    EU_AREA       = 8192,   //欧洲分区
    NA_AREA       = 16384,  //北美分区
    TEST_AREA     =  32768,    //test分区
    AREA_INVALID = 0xFFFFFFFF
};
