#pragma once

class IDualSenseInputMessageReceiver
{
public:
	virtual ~IDualSenseInputMessageReceiver() {};

	virtual void OnDualSenseConnected() = 0;
	virtual void OnDualSenseDisconnected() = 0;

	virtual int32 GetDualSenseControllerIndex() = 0;
};
