#include <stdarg.h>
#include <cstdio>
#include <map>
#include <unistd.h>
#include <string.h>

#include "MetricsEx.h"
#include "logger.h"
#include <time.h>

uint64_t getNowMonotonicUs()
{
	struct timespec stv;
	clock_gettime(CLOCK_MONOTONIC, &stv);
	uint64_t nowUsTime = stv.tv_sec * 1000 * 1000 + stv.tv_nsec / 1000;
	return nowUsTime;
}

long getCurrentTimeMicro1() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec;
}


//using namespace hawkeye_metrics;
#ifdef __cplusplus
extern "C" {
#endif

hawkeye_metrics::MetricsModelFactory *factory = NULL;

typedef std::map<std::string, hawkeye_metrics::AsyncCounterMetrics > COUNTER_MAP;
typedef std::map<std::string, hawkeye_metrics::AsyncCounterMetrics >::iterator COUNTER_MAP_ITERATOR;
COUNTER_MAP counterM;

typedef std::map<std::string, hawkeye_metrics::AsyncGaugeMetrics > GAUGE_MAP;
typedef std::map<std::string, hawkeye_metrics::AsyncGaugeMetrics >::iterator GAUGE_MAP_ITERATOR;
GAUGE_MAP gaugeM;

typedef std::map<std::string, hawkeye_metrics::AsyncDefaultMetrics > DEFAULT_MAP;
typedef std::map<std::string, hawkeye_metrics::AsyncDefaultMetrics >::iterator DEFAULT_MAP_ITERATOR;
DEFAULT_MAP defaultM;

void MetricsExInit(const char* app_name, const char* service_name)
{
	int64_t  v1[20] = {100,200,400,600,800,1000,1200,1400,1600,1800,2000,2500,3000,4000,5000,6000,8000,10000,15000,20000};
	std::vector<int64_t> scale;
	
	for (unsigned i = 0; i <hawkeye_metrics::Scale::MAX_POINT; i++)
	{
		scale.push_back(v1[i]);
	}

	hawkeye_metrics::TimeScale scaleMili = hawkeye_metrics::TimeScale::milli(scale.data(), 20);	

	factory = new hawkeye_metrics::MetricsModelFactory(
		app_name,                        		/*!<应用名 */
		"1.0",                      		/*!<应用版本 */
		service_name,			 /*!<服务名 */
		"1.0",				            /*!<服务版本 */
		hawkeye_metrics::YY, 			/*!<服务接口协议 */
		scaleMili,				            /*!<时延分布 */
		0					            /*!<表示成功的状态码，多个返回码用逗号分隔 */
	);
	hawkeye_metrics::MetricsSetting::set_not_skip_initial_period(*factory);
}

void MetricsExAddCounter(int64_t val, const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = std::string(name);
	COUNTER_MAP_ITERATOR it = counterM.find(sKey);
	if(it == counterM.end()){
		counterM[sKey] = factory->async_counter_model(name); 
	}

	hawkeye_metrics::AsyncCounterMetrics c = counterM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);

	c.add(key, val);
}

void MetricsExSetGauge(int64_t val, const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = std::string(name);
	GAUGE_MAP_ITERATOR it = gaugeM.find(sKey);
	if(it == gaugeM.end()){
		gaugeM[sKey] = factory->async_gauge_model(name); 
	}

	hawkeye_metrics::AsyncGaugeMetrics c = gaugeM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);

	c.set(key, val);
}

void MetricsExAddGauge(int64_t val, const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = std::string(name);
	GAUGE_MAP_ITERATOR it = gaugeM.find(sKey);
	if(it == gaugeM.end()){
		gaugeM[sKey] = factory->async_gauge_model(name); 
	}

	hawkeye_metrics::AsyncGaugeMetrics c = gaugeM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);
	
	c.add(key, val);
}

void MetricsExServerSetDuration(int64_t val, const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = std::string(name);
	DEFAULT_MAP_ITERATOR it = defaultM.find(sKey);
	if(it == defaultM.end()){
		defaultM[sKey] = factory->async_default_model(hawkeye_metrics::URI_TAG_SERVER); 
	}

	hawkeye_metrics::AsyncDefaultMetrics c = defaultM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);

	c.mark_microsecond(key, val, true);
}

void MetricsExServerSetDurationAndStatus(int64_t duration, uint32_t status,  const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = "server-" + std::string(name);
	DEFAULT_MAP_ITERATOR it = defaultM.find(sKey);
	if(it == defaultM.end()){
		defaultM[sKey] = factory->async_default_model(hawkeye_metrics::URI_TAG_SERVER); 
	}

	hawkeye_metrics::AsyncDefaultMetrics c = defaultM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);

	c.mark_microsecond_and_code(key, duration, status);
}


void MetricsExClientSetDuration(int64_t val, const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = std::string(name);
	DEFAULT_MAP_ITERATOR it = defaultM.find(sKey);
	if(it == defaultM.end()){
		defaultM[sKey] = factory->async_default_model(hawkeye_metrics::URI_TAG_CLIENT); 
	}

	hawkeye_metrics::AsyncDefaultMetrics c = defaultM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);

	c.mark_microsecond(key, val, true);
}

void MetricsExInnerSetDuration(int64_t val, const char *name, const char *keyFmt, ...)
{
	if(!factory)
		return;

	std::string sKey = std::string(name);
	DEFAULT_MAP_ITERATOR it = defaultM.find(sKey);
	if(it == defaultM.end()){
		defaultM[sKey] = factory->async_default_model(hawkeye_metrics::URI_TAG_INNER); 
	}

	hawkeye_metrics::AsyncDefaultMetrics c = defaultM[sKey];

	char key[512];
	va_list ap;
	va_start(ap, keyFmt);
	vsnprintf(key, sizeof(key), keyFmt, ap);
	va_end(ap);

	c.mark_microsecond(key, val, true);
}

void MetricsExSetDurationAndStatus(const char *name, hawkeye_metrics::uri_tag uriTag, const char* uri, int64_t duration, int status)
{
	if (!factory)
		return;

	std::string preName = "";
	switch (uriTag)
	{
	case hawkeye_metrics::URI_TAG_INNER:
		preName = "inner-";
		break;
	case hawkeye_metrics::URI_TAG_CLIENT:
		preName = "client-";
		break;
	case hawkeye_metrics::URI_TAG_SERVER:
		preName = "server-";
		break;

	default:
		return;
	}
	
	std::string sKey = preName + std::string(name);
	DEFAULT_MAP_ITERATOR it = defaultM.find(sKey);
	if (it == defaultM.end()) {
		defaultM[sKey] = factory->async_default_model(uriTag);
	}

	hawkeye_metrics::AsyncDefaultMetrics c = defaultM[sKey];
	c.mark_microsecond_and_code(uri, duration, status);
}

#ifdef __cplusplus    
}
#endif


MetricsBuffer::MetricsBuffer()
{
}

MetricsBuffer::~MetricsBuffer()
{
}

void* MetricsBuffer::thread_func(void *arg)
{
	MetricsBuffer* instance = (MetricsBuffer*)arg;
	while(true)
	{
		if(0 == instance->flushMsg())
			usleep(100000);
	}
}

bool MetricsBuffer::init(const char* service_name)
{
	MetricsExInit("aladdin", service_name);
	pthread_mutex_init(&m_mutex, NULL);

	// start a thread
	//
	if(pthread_create(&m_threadId, NULL, thread_func, this) != 0)
	{
		FUNLOG(Err, "start thread fail!");
		return false;
	}

	FUNLOG(Info, "MetricsBuffer init finish");
	return true;
}

int MetricsBuffer::flushMsg()
{
	//lock
	pthread_mutex_lock(&m_mutex);

	int doCount = 0;
	for (size_t i = 0; i < m_messages.size() && doCount < 10000; i++)
	{
		doCount++;

		Message msg = m_messages.front();
		m_messages.pop_front();
		MetricsExSetDurationAndStatus("", msg.uriTag, msg.uri.c_str(), msg.duration, msg.code);
	}
	pthread_mutex_unlock(&m_mutex);
	if(doCount > 0)
	{
		FUNLOG(Debug, "has push %d metrics msg", doCount);
	}
	return doCount;
}

bool MetricsBuffer::pushMsg(hawkeye_metrics::uri_tag tag, const char* uri, int64_t duration, int code)
{
	//lock
	pthread_mutex_lock(&m_mutex);
	if (m_messages.size() > 100000)
	{
		pthread_mutex_unlock(&m_mutex);
		FUNLOG(Err, "pushMsg buffer list is full...");
		return false;
	}

	Message msg;
	msg.uriTag = tag;
	msg.uri = std::string(uri);
	msg.duration = duration;
	msg.code = code;
	m_messages.push_back(msg);

	pthread_mutex_unlock(&m_mutex);
	return true;
}

MetricsAutoReporter::MetricsAutoReporter(const char* uriTag, int* status, const char* uriFmt, ...)
{
	if (0 == strcmp(uriTag, "i"))
	{
		m_uriTag = hawkeye_metrics::URI_TAG_INNER;
	}
	else if (0 == strcmp(uriTag, "s"))
	{
		m_uriTag = hawkeye_metrics::URI_TAG_SERVER;
	}
	else if (0 == strcmp(uriTag, "c"))
	{
		m_uriTag = hawkeye_metrics::URI_TAG_CLIENT;
	}

	char uri[256] = { 0 };
	va_list ap;
	va_start(ap, uriFmt);
	vsnprintf(uri, sizeof(uri), uriFmt, ap);
	va_end(ap);

	m_uri = std::string(uri);

	m_status = status;
	m_startTime = getCurrentTimeMicro1();
}

MetricsAutoReporter::~MetricsAutoReporter()
{
	int64_t useTime = getCurrentTimeMicro1() - m_startTime;
	MetricsBuffer::getInstance()->pushMsg(m_uriTag, m_uri.c_str(), useTime, *m_status);
}

bool MetricsAutoReporter::pushMsgNow(const char* uriTag, int status, const char* uri)
{
	hawkeye_metrics::uri_tag tag = hawkeye_metrics::URI_TAG_INNER;
	if (0 == strcmp(uriTag, "i"))
	{
		tag = hawkeye_metrics::URI_TAG_INNER;
	}
	else if (0 == strcmp(uriTag, "s"))
	{
		tag = hawkeye_metrics::URI_TAG_SERVER;
	}
	else if (0 == strcmp(uriTag, "c"))
	{
		tag = hawkeye_metrics::URI_TAG_CLIENT;
	}

	return MetricsBuffer::getInstance()->pushMsg(tag, uri, 0, status);
}
