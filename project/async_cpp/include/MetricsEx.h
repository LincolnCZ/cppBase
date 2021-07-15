#ifndef _METRICSEX_H_
#define _METRICSEX_H_

#include "MetricsApi.h"

#include <list>
#include <string>
#include <pthread.h>
#include "ISingleton.h"

#ifdef __cplusplus
extern "C" {
#endif
void MetricsExInit(const char* app_name, const char* service_name);
void MetricsExAddCounter(int64_t val, const char *name, const char *keyFmt, ...);
void MetricsExSetGauge(int64_t val, const char *name, const char *keyFmt, ...);
void MetricsExAddGauge(int64_t val, const char *name, const char *keyFmt, ...);
void MetricsExClientSetDuration(int64_t val, const char *name, const char *keyFmt, ...);
void MetricsExServerSetDuration(int64_t val, const char *name, const char *keyFmt, ...);
void MetricsExServerSetDurationAndStatus(int64_t duration, uint32_t status,  const char *name, const char *keyFmt, ...);
void MetricsExInnerSetDuration(int64_t val, const char *name, const char *keyFmt, ...);

void MetricsExSetDurationAndStatus(const char *name, hawkeye_metrics::uri_tag uriTag, const char* uri, int64_t duration, int status);
#ifdef __cplusplus
}
#endif

class MetricsBuffer: public ISingleton<MetricsBuffer>
{
	friend class ISingleton<MetricsBuffer>;

private:
	MetricsBuffer();
	virtual ~MetricsBuffer();

public:
	bool init(const char* service_name);
	int flushMsg();
	bool pushMsg(hawkeye_metrics::uri_tag tag, const char* uri, int64_t duration, int code);
	void Timer();

private:
	struct Message
	{
		hawkeye_metrics::uri_tag uriTag;
		std::string uri;
		int64_t duration;
		int code;
	};
	
	std::list<Message> m_messages;
	pthread_mutex_t m_mutex;
	pthread_t       m_threadId;

	static void* thread_func(void *arg);
};

class MetricsAutoReporter
{
public:
	MetricsAutoReporter(const char* uriTag, int* status, const char* uriFmt, ...);
	~MetricsAutoReporter();

	static bool pushMsgNow(const char* uriTag, int status, const char* uri);

private:
	int* m_status;
	int64_t m_startTime;
	hawkeye_metrics::uri_tag m_uriTag;
	std::string m_key;
	std::string m_uri;
};

enum METRICS_ERROR
{
	ERROR_SUCCESS = 0,
};

#endif /* METRICS_H_ */
