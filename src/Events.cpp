#include "pch.h"
#include "Events.h"
#include <algorithm>


//#define DEFINE_EVENT_CLASS_IMPLEMENTATION2(NAME, ARG1, ARG2) \
//void NAME ## ::Fire(ARG1, ARG2) \
//{ \
//	if (!_subscribers.empty()) \
//		for (auto it = _subscribers.begin(); it != _subscribers.end(); it++) \
//			(*it)->Call(pStr, type); \
//} \
//\
//API NAME ## ::Subscribe(I ## NAME ## Subscriber* pSubscriber) \
//{ \
//	auto it = std::find_if(_subscribers.begin(), _subscribers.end(), \
//		[pSubscriber](I ## NAME ## tSubscriber *sbr) -> bool \
//	{ \
//		return \
//			pSubscriber == sbr; \
//	}); \
// \
//	if (it == _subscribers.end()) \
//		_subscribers.push_back(pSubscriber); \
// \
//	return S_OK; \
//} \
// \
//API NAME ## ::Unsubscribe(I ## NAME ## Subscriber* pSubscriber) \
//{ \
//	auto it = std::find_if(_subscribers.begin(), _subscribers.end(), \
//		[pSubscriber](I ## NAME ## Subscriber *sbr) -> bool \
//	{ \
//		return \
//			pSubscriber == sbr; \
//	}); \
// \
//	if (it != _subscribers.end()) \
//		_subscribers.erase(it); \
// \
//	return S_OK; \
//}
//
//DEFINE_EVENT_CLASS_IMPLEMENTATION2(LogEvent, const char *pMessage, LOG_TYPE type)
