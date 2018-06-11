#pragma once
#include "Common.h"
#include <algorithm>


//#define DEFINE_EVENT_CLASS2(NAME, ARG1, ARG2) \
//class NAME final : public I ## NAME \
//{ \
//	std::vector<I ## NAME ## Subscriber *> _subscribers; \
//\
//public: \
//\
//	void Fire(ARG1, ARG2); \
//\
//	API Subscribe(I ## NAME ## Subscriber* pSubscriber) override; \
//	API Unsubscribe(I ## NAME ## Subscriber* pSubscriber) override; \
//};
//
//DEFINE_EVENT_CLASS2(LogEvent, const char *pMessage, LOG_TYPE type)


template <typename IDerived, typename ISubscriber, typename... Arguments>
class EventTemplate : public IDerived
{
	std::vector<ISubscriber *> _subscribers;

public:

	void Fire(Arguments ... args)
	{
		if (!_subscribers.empty())
			for (auto it = _subscribers.begin(); it != _subscribers.end(); it++)
				(*it)->Call(args...);
	}

	API Subscribe(ISubscriber* pSubscriber) override
	{		
		auto it = std::find_if(_subscribers.begin(), _subscribers.end(), [pSubscriber](ISubscriber *sbr) -> bool { return pSubscriber == sbr; });
 
		if (it == _subscribers.end()) 
			_subscribers.push_back(pSubscriber); 
 
		return S_OK; 
	} 

	API Unsubscribe(ISubscriber* pSubscriber) override
	{
		auto it = std::find_if(_subscribers.begin(), _subscribers.end(), [pSubscriber](ISubscriber *sbr) -> bool { return pSubscriber == sbr; }); 
 
		if (it != _subscribers.end()) 
			_subscribers.erase(it); 
 
		return S_OK; 
	}
};


typedef EventTemplate<ILogEvent, ILogEventSubscriber, const char *, LOG_TYPE> LogEvent;
typedef EventTemplate<IPositionEvent, IPositionEventSubscriber, OUT vec3*> PositionEvent;
