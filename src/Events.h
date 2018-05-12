#pragma once
#include "Common.h"


class EventLog final : public ILogEvent
{
	std::vector<ILogEventSubscriber *> _subscribers;

public:
	
	void Fire(const char *pStr, LOG_TYPE type);

	API Subscribe(ILogEventSubscriber* pSubscriber) override;
	API Unsubscribe(ILogEventSubscriber* pSubscriber) override;
};

