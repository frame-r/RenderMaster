#include "EvLog.h"
#include <algorithm>


void EvLog::Fire(const char* pStr, LOG_TYPE type)
{
	if (!_subscribers.empty())
		for (auto it = _subscribers.begin(); it != _subscribers.end(); it++)
			(*it)->Call(pStr, type);
}

API EvLog::Subscribe(ILogEventSubscriber* pSubscriber)
{
	auto it = std::find_if(_subscribers.begin(), _subscribers.end(),
		[pSubscriber](ILogEventSubscriber *sbr) -> bool
	{
		return
			pSubscriber == sbr;
	});

	if (it == _subscribers.end())
		_subscribers.push_back(pSubscriber);

	return S_OK;
}

API EvLog::Unsubscribe(ILogEventSubscriber* pSubscriber)
{
	auto it = std::find_if(_subscribers.begin(), _subscribers.end(),
		[pSubscriber](ILogEventSubscriber *sbr) -> bool
	{
		return
			pSubscriber == sbr;
	});

	if (it != _subscribers.end())
		_subscribers.erase(it);

	return S_OK;
}
