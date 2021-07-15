/*
 * metrics_api.h
 *
 *  Created on: 2015年3月27日
 *      Author: Administrator
 */

#ifndef METRICS_API_H_
#define METRICS_API_H_

#include <stdint.h>

#include <set>
#include <string>
#include <vector>


/**
 * version: 3.2.0
 */
namespace hawkeye_metrics {

  extern const char* svn_revision();
  extern const char* version();

  /*! \brief 若干枚举值，代表不同的应用协议
   *
   * SDK是面向服务端程序的，服务端程序必定是基于某种应用协议提供对外服务的。
   * 大部分指标是针对服务端的服务接口，而同一服务，可能支持多种协议。
   *
   * 因此鹰眼metrics认为协议是统计实体的固有属性.
   *
   * 每个枚举值，有一个对应的字符串标识，在SDK内部用到.
   */
  enum app_protocol {
    /*!
     * 用此枚举值, 表示统计实体是协议无关的.
     *
     * 字符串标识: ""
     */
    UNKNOWN = 0,
    YY = 1, /*!< YY 协议, 字符串标识: "yy"*/
    THRIFT = 2, /*!< thrift 协议, 字符串标识: "thrift"*/
    HTTP = 5, /*!< http 协议, 字符串标识: "http"*/
    METRICS_PROTOCOL_MAX = HTTP,
  };
  typedef app_protocol protocol;

  enum uri_tag {
    URI_TAG_SERVER = 1, /*!< 作为服务端的uri */
    URI_TAG_CLIENT = 2, /*!< 作为客户端调用别人时的uri */
    URI_TAG_INNER = 3 /*!< 标识进程内功能实体的uri */
  };

  /*! \brief 封装一个有序整型数组, 代表一些区间，相连2个数组成员构成一个半开闭区间: [x,y)
   *
   * 对整型连续值的做出一种划分,用来度量一批数值,落在各个区间的分布情况.
   * 不同的数组, 代表对不同的划分
   *
   * 数组最大是20个元素
   */
  class Scale {
  public:
    static const size_t MAX_POINT = 20;

    Scale() : mul_(1), sz_(), scale_() {
    }
    Scale(const Scale& other);

    /*!
     * @param points 数组
     * @param size 数组大小, 超过20个时, 排序后截取前20个
     * @param mul 乘数因子, points的每个元素会乘以该数
     */
    Scale(const int64_t *points, size_t size, int64_t mul);

    virtual ~Scale() {
    }
    int64_t mul() const;
    size_t size() const;
    inline int find(int64_t v) const;
    int64_t get(unsigned i) const;
    bool operator==(const Scale&) const;
  private:
    static std::vector<int64_t> scale(const int64_t *points, size_t size, int mul);
    int64_t mul_;
    size_t sz_;
    int64_t scale_[MAX_POINT];
  };

  /*! \brief Scale的子类, 专门表示对时延(duration)的划分
   *
   * 划分出区间后,某个时间段接口处理时延的分布情况,可反应接口是否正常.
   *
   * 时延有单位: 毫秒 微秒 纳秒等. 内部统一转为纳秒.
   */
  class TimeScale : public Scale {
  public:

    TimeScale():Scale() {
    };
    /*! \brief 构造一个时延划分, 时延单位为微秒
     *
     * @param v 64位整型数组, 代表一种划分.数值的单位是微秒
     */
    TimeScale(const std::vector<int64_t>& v) : Scale(v.data(), v.size(), 1000) {
    };

    /*! \brief 构造一个时延划分, 时延单位为微秒
     *
     * @param points 64位整型数组, 代表一种划分.数值的单位是微秒
     * @param size 数组大小, 排序后只截取前20个
     */
    TimeScale(const int64_t *points, size_t size) : Scale(points, size, 1000) {
    };

    /*! \brief 构造一个时延划分, 时延单位为纳秒
     *
     * @param v 64位整型数组, 代表一种划分.数值的单位是纳秒
     */
    static TimeScale nano(const std::vector<int64_t>& v) {
      return TimeScale(v.data(), v.size(), 1);
    }

    /*! \brief 构造一个时延划分, 时延单位为纳秒
     *
     * @param points 64位整型数组, 代表一种划分.数值的单位是纳秒
     * @param size 数组大小, 排序后只截取前20个
     */
    static TimeScale nano(const int64_t *points, size_t size) {
      return TimeScale(points, size, 1);
    }

    /*! \brief 构造一个时延划分, 时延单位为毫秒
     *
     * @param points 64位整型数组, 代表一种划分.数值的单位是毫秒
     * @param size 数组大小, 排序后只截取前20个
     */
    static TimeScale milli(const int64_t *points, size_t size) {
      return TimeScale(points, size, 1000000);
    }
  private:

    TimeScale(const int64_t *points, size_t size, int64_t mul) : Scale(points, size, mul) {
    };
  };


  struct code_set;

  /*! \brief 返回码的封装类. 返回码一般有int或string类型.通过此类消除类型差别,方便API接口定义
   *
   */
  struct code_type {
    bool is_int;
    int i_code;
    std::string s_code;
    code_type(int code);
    code_type(const std::string& code);
    code_type(const char* code);
    code_type(const code_type& code);
    bool operator==(const code_type& other) const;
    bool operator<(const code_type& other) const;
    bool is_success(const code_set* set) const;

    code_type& operator=(const code_type& other);
    std::string str() const;
  };

  /*! \brief 多个返回码的集合
   *
   */
  struct code_set {
    int flag; // ORs of these flag: 0x1 i_code is set, 0x2 s_code is set
    int i_code;
    std::string s_code;
    std::vector<std::string> s_codes;
    std::vector<int> i_codes;
    code_set();
    code_set(int code);
    code_set(const std::string& code);
    void push(int);
    void push(const std::string& s);
    // denote that codes in this set are failure codes
    void reverse_to_failure_code();
    bool is_success(const std::string& code) const;
    bool is_success(int code) const;
    bool is_success(const char *code, int len) const;
  protected:
    enum { THESE_ARE_FAILURE_CODES = 0x08 };
    bool is_success() const;
  };


  /*! \brief 设置app name和app version
   *
   * 通过包发布部署的进程，如果没有调用本函数，
   * SDK会尝试从文件/data/service/<app>/admin/pkg.pkgName和
   * /data/service/<app>/admin/pkg.verName
   * 读取包名和版本信息。
   *
   * 如果app name和/或 app version,且不为空串,则使用参数值作为上报的app name和/或app version
   *
   * MetricsModelFactory的构造参数需要指定app name 和 app version.最终也是调回本函数
   *
   * 注意: 一个进程只能设置成功一次，之后就没法修改
   * 不要在main函数启动之前调用
   *
   * @param app_name 包名，或者进程名。命名有规范：必须是合法字符[0-9a-zA-Z_-.]（字母、数字、下划线、横线（-）、英文点（.）），且长度不超过63
   * @param app_version app 版本信息。命名有规范：必须是合法字符[0-9a-zA-Z_-.]（字母、数字、下划线、横线（-）、英文点（.）），且长度不超过63
   * @return 0 代表设置成功，其他返回值失败
   */
  extern int set_app_name(const std::string& app_name, const std::string& app_version);

  /*! 停掉metrics线程. wait_us是等待响应的时间,微秒*/
  bool shutdown_metrics(const char *who, unsigned wait_us);


  class DefMetricsImpl;


  class AdditionalParam {
  private:
	  uri_tag tag_;
	  int period_;
  public:
	  const static AdditionalParam defap;

	  AdditionalParam();

	  void set_tag(uri_tag tag);

      /*! \brief 修改指标的默认统计周期. 默认是1分钟
       *
       *@param step 指标统计周期. 允许的值为60(1分钟),300(5分钟)
       *@return true修改成功。 false修改失败,非法值
      */
	  bool set_period(int period);

	  uri_tag tag() const {
		  return tag_;
	  }
	  int period() const {
		  return period_;
	  }
  };

  /*! \brief 各种指标模型的工厂类的基类
   *
   *继承此类的子类，一般提供一个get方法，用来创建某种指标模型的实例。
   *该实例和一个具体的指标关联
   */
  class MetricsGetter {
    friend class MetricsModelFactory;
  protected:

    MetricsGetter() : impl_(), proto_(UNKNOWN), topic_() {
    }
    /*! \brief 构造指标模型的工厂类实例
     *
     * SDK将一个 topic+protocol 组合关联到一种指标模型。
     * 标识一个具体指标，除了指标名称，还要知道其topic和protocol。
     * topic+protocol相同的具体指标，其指标模型是一致的。
     *
     * @param topic 指标主题. 一个主题下面，可有一至多个具体的指标，这些具体指标针对不同的统计实体，但统计内容是一致的。
     * @param proto 协议. SDK是面向服务端进程的，服务端进程必然是基于某种应用协议对外提前服务的，比如http/thrift/yy等。
     * SDK支持的协议见protocol
     */
    MetricsGetter(const std::string& topic, app_protocol proto, DefMetricsImpl *impl);

    DefMetricsImpl *get_impl();

    DefMetricsImpl *impl_;
    app_protocol proto_;
    std::string topic_;
    const static AdditionalParam &defap;
  public:
    /*!
     *@return false: 类实例是无效的, 无效实例的get()方法返回的对象也是无效的.
     *        true: 类实例是有效的
     */
    bool is_valid();
  };

  /*! \brief 获取当前线程调用API是内部产生的错误,比如参数检验失败
   *
   * 一般配合MetricsGetter::is_valid()使用
   *
   * @param s 错误信息写到s中
   * @return 返回true,说明线程调api时有或曾经有错误,将s打印; 返回false,不必打印s
   * SDK支持的协议见protocol
   */
  extern bool get_metrics_error_log(std::string &s);

  class AsyncDefaultMetrics : public MetricsGetter {
  public:

    AsyncDefaultMetrics() : MetricsGetter(), ap_() {
    }
    /*! \brief 简单调用基类的构造方法
     *
     *@param proto 协议, 见MetricsGetter
     *@param scale 时延划分
     *@param success_codes 成功码集合
     *@param impl sdk内部实现
     *@param tag 指标类型标签
     */
    AsyncDefaultMetrics(app_protocol proto, const Scale& scale, const code_set& success_codes, DefMetricsImpl *impl, const AdditionalParam &);

    /*! \brief 给时延分布指标提供数据
     *
     *@param key 指标名
     *@param duration 接口时延,单位微秒
     *@param proc_count 如果为true, 本接口会使内部一个计数值+1. 如果每处理一个请求调用本接口1次,则该计数值就统计了请求次数
     */
    int mark_microsecond(const std::string& key, int64_t duration, bool proc_count = false);

    /*! \brief
     *
     *@param key 指标名
     *@param duration 接口时延,单位纳秒
     *@param proc_count 见mark_microsecond
     */
    int mark_nanosecond(const std::string& key, int64_t duration, bool proc_count = false);

    /*! \brief 给返回码分布指标提供数据
     *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
     *
     *@param key 指标名
     *@param code 返回码
     *@param proc_count 见mark_microsecond
     */
    int mark_code(const std::string& key, int code, bool proc_count = false);

    /*! \brief
     *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
     *
     *@param key 指标名
     *@param code 返回码
     *@param proc_count 见mark_microsecond
     */
    int mark_code(const std::string& key, const std::string& code, bool proc_count = false);

    /*! \brief 同时给 时延分布和返回码分布 指标提供数据
     *
     * 内部一个计数值+1. 如果每处理一个请求调用本接口1次,则该计数值就统计了请求次数
     * 注意如果同时使用了 mark_code或mark_microsecond,要注意他们的proc_count参数
     *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
     *
     *@param key 指标名
     *@param duration 接口时延,单位微秒
     *@param code 返回码
     */
    int mark_microsecond_and_code(const std::string& key, int64_t duration, int code);

    /*! \brief
     *
     * 内部一个计数值+1. 如果每处理一个请求调用本接口1次,则该计数值就统计了请求次数
     * 注意如果同时使用了 mark_code或mark_microsecond,要注意他们的proc_count参数
     *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
     *
     *@param key 指标名
     *@param duration 接口时延,单位微秒
     *@param code 返回码
     */
    int mark_microsecond_and_code(const std::string& key, int64_t duration, const std::string& code);

    /*! \brief 同时给 时延分布和返回码分布 指标提供数据
     *
     * 内部一个计数值+1. 如果每处理一个请求调用本接口1次,则该计数值就统计了请求次数
     * 注意如果同时使用了 mark_code或mark_microsecond,要注意他们的proc_count参数
     *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
     *
     *@param key 指标名
     *@param duration 接口时延,单位纳秒
     *@param code 返回码
     */
    int mark_nanosecond_and_code(const std::string& key, int64_t duration, int code);

    /*! \brief
     *
     * 内部一个计数值+1. 如果每处理一个请求调用本接口1次,则该计数值就统计了请求次数
     * 注意如果同时使用了 mark_code或mark_microsecond,要注意他们的proc_count参数
     *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
     *
     *@param key 指标名
     *@param duration 接口时延,单位纳秒
     *@param code 返回码
     */
    int mark_nanosecond_and_code(const std::string& key, int64_t duration, const std::string& code);

  private:
    AdditionalParam ap_;
  };

  class AsyncCounterMetrics : public MetricsGetter {
  public:

    AsyncCounterMetrics() : MetricsGetter() {
    }
    /*! \brief 简单调用基类的构造方法
     *
     *@param topic 指标主题名
     *@param proto 协议, 见MetricsGetter
     *@param impl sdk内部实现
     *@param tag 指标类型标签
     */
    AsyncCounterMetrics(const std::string& topic, app_protocol proto, DefMetricsImpl *impl);

    /*! \brief 给 计数器类型 指标提供数据
      *
      *@param key 指标名. 指标都归属到同一个topic(构造方法中的topic参数).后端对同一topic下的指标做聚合
      *@param v  计数器新增值
      */
    int add(const std::string& key, int64_t v, const AdditionalParam* = NULL);

  };

  class AsyncGaugeMetrics : public MetricsGetter {
  public:

    AsyncGaugeMetrics() : MetricsGetter() {
    }
    /*! \brief 简单调用基类的构造方法
     *
     *@param topic 指标主题名
     *@param proto 协议, 见MetricsGetter
     *@param impl sdk内部实现
     *@param tag 指标类型标签
     */
	AsyncGaugeMetrics(const std::string& topic, app_protocol proto, DefMetricsImpl *impl);

    /*! \brief 给 Gauge 指标设置新数值
      *
      *@param key 指标名. 指标都归属到同一个topic(构造方法中的topic参数).后端对同一topic下的指标做聚合
      *@param v  新值
      */
    int set(const std::string& key, int64_t v, const AdditionalParam* = NULL);

    /*! \brief 增减指标数值
      *
      *@param key 指标名. 指标都归属到同一个topic(构造方法中的topic参数).后端对同一topic下的指标做聚合
      *@param v  新值
      */
    int add(const std::string& key, int64_t v, const AdditionalParam* = NULL);
  };

  class AsyncHistogramMetrics : public MetricsGetter {
  public:

	AsyncHistogramMetrics() : MetricsGetter() {
    }

	/*! \brief 简单调用基类的构造方法
	 *
	 *@param topic 指标主题名
	 *@param proto 协议, 见MetricsGetter
	 *@param scale 分布划分
	 *@param impl sdk内部实现
	 *@param tag 指标类型标签
	 */
	AsyncHistogramMetrics(const std::string& topic, app_protocol proto, const Scale& scale, DefMetricsImpl *impl);

	/*! \brief 给 时延分布 指标提供统计数据
	  *
	  *@param key 指标名. 指标都归属到同一个topic(构造方法中的topic参数).后端对同一topic下的指标做聚合
	  *@param val 分布原始数据.
	  * val参数如果是时间，则时间单位是由构造方法的scale参数决定的。(这个和AsyncDefaultMetrics的mark_microsecond/mark_nanosecond不一样，他们的单位是固定的了)
	  * 也就是说, val 在内部会乘以 scale.mul()
	  */
    int mark(const std::string& key, int64_t val, const AdditionalParam* = NULL);

  };

  class AsyncCodeMetrics : public MetricsGetter {
  public:

    AsyncCodeMetrics() : MetricsGetter() {
    }
	/*! \brief 简单调用基类的构造方法
	 *
	 *@param topic 指标主题名
	 *@param proto 协议, 见MetricsGetter
	 *@param success_codes 成功码集合
	 *@param impl sdk内部实现
	 *@param tag 指标类型标签
	 */
    AsyncCodeMetrics(const std::string& topic, app_protocol proto, const code_set& success_codes, DefMetricsImpl *impl);

	/*! \brief 给 返回码分布 指标提供统计数据
	  *
     * 一个指标可以报的返回码个数限制63个,如果估算会超过这个限制,需要联系运维提供新版本
     * 整型返回码和字符串返回码是不同的,比如0不同于'0'
	  *
	  * 只统计分布,不做成功率计算
	  *
	  *@param key 指标名. 指标都归属到同一个topic(构造方法中的topic参数).后端对同一topic下的指标做聚合
	  *@param code 返回码.
	  */
    int mark(const std::string& key, int code, const AdditionalParam* = NULL);
    int mark(const std::string& key, const std::string& code, const AdditionalParam* = NULL);

  };

  class AsyncSuccessRatioMetrics : public MetricsGetter {
  public:

	AsyncSuccessRatioMetrics() : MetricsGetter() {
    }
	/*! \brief 简单调用基类的构造方法
	 *
	 *@param topic 指标主题名
	 *@param proto 协议, 见MetricsGetter
	 *@param impl sdk内部实现
	 *@param tag 指标类型标签
	 */
	AsyncSuccessRatioMetrics(const std::string& topic, app_protocol proto, DefMetricsImpl *impl);

	/*! \brief 给 成功率 指标提供统计数据
	  *
	  * 参数值累加到当前值.
	  *
	  *@param key 指标名. 指标都归属到同一个topic(构造方法中的topic参数).后端对同一topic下的指标做聚合
	  *@param success 成功数
	  *@param fail 失败数
	  */
	inline int add(const std::string& key, int64_t success, int64_t fail, const AdditionalParam* ap = NULL)
	{
		return upset(key, success, fail, false, ap);
	}

	/*! \brief 给 成功率 指标提供统计数据
	  *
	  * 参数值替换为当前值.
	  *
	  *@param key 指标名
	  *@param success 成功数
	  *@param fail 失败数
	  */
	inline int set(const std::string& key, int64_t success, int64_t fail, const AdditionalParam* ap = NULL)
	{
		return upset(key, success, fail, true, ap);
	}
  private:
	int upset(const std::string& key, int64_t success, int64_t fail, bool is_set, const AdditionalParam*);
  };

  struct Callback {

    Callback() : impl_() {
    }

    virtual ~Callback() {
      cancel();
    }
    /*! \brief 通知SDK注销掉此回调. 注意SDK不会delete回调对象
     *
     *@return 返回true则可以安全的delete此对象. 注意如果回调对象根本没注册过,也返回true
     *返回false,说明SDK刚好在执行回调
     */
    bool cancel();
    /*! \brief 判断此回调对象是否已从SDK注销
     *
     *@return true表示已注销.注意如果回调对象根本没注册过,也返回true
     */
    bool is_canceled();

    /*! \brief 子类实现本方法. SDK内部在开始序列化指标数据之前回调此方法
     *
     */
    virtual void run() {
    };
  private:
    DefMetricsImpl *impl();
    DefMetricsImpl *impl_;
    friend class DefMetricsImpl;
  };

  /*! \brief 工厂类，提供方法创建Metrics工厂类(继承了MetricsGetter)实例,用返回工厂类实例,可创建指标实例
   *
   */
  class MetricsModelFactory {
  public:

    /*! \brief 让SDK在开始序列化上报数据之前,先执行一个回调.
     *
     *回调任务必须不能block住不返回.
     *回调任务的时延必须尽量小,否则会影响上报的准点,太长了还影响下个周期数据的准确性.
     *
     *一个回调只能注册一次,注销后不能再次注册
     *
     * @param cb 要执行的回调, 如果之前设置过cb, 则旧的回调被注销(见Callback::cancel()注释)
     * @param old_cb 如果不为null,返回旧的回调
     * @return true设置成功, false设置失败
     */
    bool set_callback(Callback *cb, Callback **old_cb);

    /*! \brief 创建 默认的指标模型
     *
     */
    AsyncDefaultMetrics async_default_model(const AdditionalParam *ap = NULL);

    /*! \brief 创建 默认的指标模型,指定指标的uri类型标签
     *
     */
    AsyncDefaultMetrics async_default_model(uri_tag tag);

    /*! \brief 创建 COUNTER指标模型,该模型的指标归属同一个指标主题
     *
     * 返回对象使用MetricsModelFactory::protocol作为构造参数
     *
     * 如果返回对象的is_valid()返回true(valid的),则参数topic就绑定到COUNTER类型的指标.
     * 也就说,如果再用相同的topic值调用MetricsModelFactory的其他async_xx_model方法,
     * 返回的对象的is_valid()会返回false(invalid的).
     *
     * 进程报上去的数据,只会看到topic主题的指标的数据都是COUNTER类型的.
     * 当然SDK无法阻止同一个程序专门写个进程用topic报其他类型的指标.这样做后端统计会有问题.
     *
     * 不同的factory对象(service_name不同,见构造方法的说明),也可以用相同的topic报另一种类型的指标.
     * (很少场景需要这样.topic的意义和指标类型应该是固定的才对)
     *
     * 同一个factory对象,用同一个topic调用本方法多次,
     * 如果第一次返回valid的对象,后续也都是返回valid的对象,且他们是等价的.
     * 如果第一次返回invalid的对象,后续都是返回invalid的对象.
     *
     * 内部用了map来保存topic和指标类型的对应关系,没有限制map的大小,map也不清空,
     * 因此如果写个循环用一堆没有的topic调用本方法,是会造成内存泄漏的.
     *
     * 以下topic被sdk保留自己用了
     * proc_time
     * retcode
     * rcode
     * success_rate
     * SuccessRate
     * proc_count
     * biz_success
     * biz_failed
     *
     * @param topic 指标主题. 传给AsyncCounterMetrics构造方法的topic参数.
     */
    AsyncCounterMetrics async_counter_model(const std::string& topic);

    /*! \brief 创建 GAUGE指标模型,该模型的指标归属同一个指标主题
     *
     * 见 async_counter_model 方法注释
     *
     * @param topic 指标主题. 传给AsyncGaugeMetrics构造方法的topic参数.
     */
    AsyncGaugeMetrics async_gauge_model(const std::string& topic);

    /*! \brief 创建 HISTOGRAM指标模型,该模型的指标归属同一个指标主题
     *
     * 见 async_counter_model 方法注释
     *
     * 用MetricsModelFactory::scale_作为传给AsyncHistogramMetrics构造方法的scale参数.
     * 同一个topic下的指标使用同一个scale, 在第一次创建topic时就定好了.
     *
     * @param topic 指标主题. 传给AsyncHistogramMetrics构造方法的topic参数.
     */
    AsyncHistogramMetrics async_histogram(const std::string& topic);

    /*! \brief 创建 HISTOGRAM指标模型,该模型的指标归属同一个指标主题
     *
     * 见 async_counter_model 方法注释
     *
     * 同一个topic下的指标使用同一个scale, 在第一次创建topic时就定好了.
     *
     * @param topic 指标主题. 传给AsyncHistogramMetrics构造方法的topic参数.
     * @param scale 传给AsyncHistogramMetrics构造方法的scale参数.
     */
    AsyncHistogramMetrics async_histogram(const std::string& topic, const Scale& scale);

    /*! \brief 创建 返回码分布指标模型,该模型的指标归属同一个指标主题
     *
     * 见 async_counter_model 方法注释
     *
     * 用MetricsModelFactory::success_codes_作为传给AsyncCodeMetrics构造方法的success_codes参数.
     * 同一个topic下的指标使用同一个成功码集, 在第一次创建topic时就定好了.
     *
     * @param topic 指标主题. 传给AsyncCodeMetrics构造方法的topic参数.
     */
    AsyncCodeMetrics async_retcode(const std::string& topic);

    /*! \brief 创建 返回码分布指标模型,该模型的指标归属同一个指标主题
     *
     * 见 async_counter_model 方法注释
     *
     * 同一个topic下的指标使用同一个成功码集, 在第一次创建topic时就定好了.
     *
     * @param topic 指标主题. 传给AsyncCodeMetrics构造方法的topic参数.
     * @param codes 传给AsyncCodeMetrics构造方法的success_codes参数.
     */
    AsyncCodeMetrics async_retcode(const std::string& topic, const code_set& codes);

    /*! \brief 创建 成功率指标模型,该模型的指标归属同一个指标主题
     *
     * 见 async_counter_model 方法注释
     *
     * @param topic 指标主题. 传给AsyncSuccessRatioMetrics构造方法的topic参数.
     */
    AsyncSuccessRatioMetrics async_successratio_model(const std::string& topic);

#ifdef YYMS_METRICS
    void discard_all(bool b = true);
#endif

    void set_metrics_num_hint(unsigned v);


    /*! \brief 构造方法
     *
     * 一个(服务名+协议)组合,创建一个factory即可.
     * 一个服务名对应一个DefMetricsImpl(内部实现类)对象.最多可注册63个服务名.
     *
     * 同一个service_name下面,完整的指标索引是 topic+protocol+指标名
     * 一个service_name下面的指标索引个数有限制,默认是4096
     *
     * 协议是async_xxx_model等方法内部构建返回对象时用来作为调用参数,传给返回对象.
     * 而返回对象自身的接口内部,则用构建时得到的协议来创建指标.
     * 这样很多接口一般不需要传协议这个参数了.
     *
     * 数据的最顶层分类是app_name+service_name
     * 数据的层次是走 app_name -> service_name -> topic+protocol -> 指标名
     * 指标名内部可以 用斜杠 '/' 再划分层次. 目前支持2层
     *
     * 如果服务名或服务版本校验不过,此factory对象是无效的,其async_xxxx_model方法返回的对象也是无效的,
     * 调用这些返回对象的接口是空操作,不会产生统计数据上报.
     *
     *@param app_name app name.
     *                见set_app_name方法注释
     *                包发布系统进程填""即可. 其他类型进程，填进程名
     *                命名规范：合法字符[0-9a-zA-Z_-.]（字母、数字、下划线、横线（-）、英文点（.）），且长度不超过63
     *@param app_version app version
     *                见set_app_name方法注释
     *                包发布系统进程填""即可. 其他类型进程，填进程版本
     *                命名规范：合法字符[0-9a-zA-Z_-.]（字母、数字、下划线、横线（-）、英文点（.）），且长度不超过63
     *@param service_name 服务名
     *　　　　　　　　 命名规范：合法的标识符（字母、数字和下划线，不能以数字开头），且不能以下划线开头，且长度不超过63
     *@param service_version 服务版本
     *                命名规范：合法字符[0-9a-zA-Z_-.]（字母、数字、下划线、横线（-）、英文点（.）），且长度不超过63
     *@param proto 协议. 服务使用的应用协议
     *@param scale 时延划分, 数值单位是微秒
     *                缺省统计模型中proc_time指标使用的时延划分
     *@param success_code 成功码
     *　　　　　　　　缺省统计模型中retcode指标使用的成功码
     *@deprecated use the concise alternative
     */
    MetricsModelFactory(const std::string& app_name, const std::string& app_version,
            const std::string& service_name, const std::string& service_version, app_protocol proto,
            const TimeScale& scale, const code_set& success_codes);

    /*!\brief since v3.1.2, this is recommend
     * the 'service_version' and 'protocol' parameter is no longer need
     */
    MetricsModelFactory(const std::string& app_name, const std::string& app_version,
            const std::string& service_name,
            const TimeScale& scale, const code_set& success_codes);

    /*! \brief 默认构造方法,方便客户端使用SDK
     *  使用此构造方法构建的对象总是无效的
     */
    MetricsModelFactory();

  private:
    DefMetricsImpl* get_impl(bool chk_topic, const std::string& topic, const void *type_info);
    DefMetricsImpl* get_impl(const std::string& topic, const void *type_info);
    DefMetricsImpl* get_impl();
    DefMetricsImpl *impl_;
    app_protocol proto_;          // obsoleted
    std::string service_name_;
    std::string service_version_; // obsoleted
    TimeScale scale_;
    code_set success_codes_;
    friend class SyncMetricsModelFactory;
    friend class MetricsSetting;
  };

  class MetricsSetting {
  public:
    /*! 默认行为:指标第一个周期的数据默认丢弃,因为通常统计时间不够一个周期.此接口取消这个默认行为 */
    static void set_not_skip_initial_period(MetricsModelFactory &f);
    /*! 默认行为:连续4个周期没数据,就不上报,SDK从内部清除这个指标.此接口让没有数据的指标一直上报0 */
    static void set_remove_nodata_metrics(MetricsModelFactory &f);
    static void set_metrics_num_hint(MetricsModelFactory &f, unsigned v);
    static void set_m3a_bufsize(unsigned);
    static void set_queue_size(int sz);
  };
}



#endif /* METRICS_API_H_ */
