#include "muduo/base/FileUtil.h"

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace muduo;

int main()
{
  string result;
  int64_t size = 0;
  int err = FileUtil::readFile("/proc/self", 1024, &result, &size);
  printf("/proc/self %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/proc/self", 1024, &result, NULL);
  printf("/proc/self %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/proc/self/cmdline", 1024, &result, &size);
  printf("/proc/self/cmdline %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/dev/null", 1024, &result, &size);
  printf("/dev/null %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/dev/zero", 1024, &result, &size);
  printf("/dev/zero %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/notexist", 1024, &result, &size);
  printf("/notexist %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/dev/zero", 102400, &result, &size);
  printf("/dev/zero %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
  err = FileUtil::readFile("/dev/zero", 102400, &result, NULL);
  printf("/dev/zero %d %zd %ld content:%s" PRIu64 "\n", err, result.size(), size, result.c_str());
}

