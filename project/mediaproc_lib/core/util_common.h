#pragma once

#ifdef NO_UTILITY_NAMESPACE
#define UTILITY_NAMESPACE_BEGIN
#define UTILITY_NAMESPACE_END
#define UTILITY_NAMESPACE_USE
#else
#define UTILITY_NAMESPACE_BEGIN namespace utility {
#define UTILITY_NAMESPACE_END }
#define UTILITY_NAMESPACE_USE using namespace utility;
#endif
