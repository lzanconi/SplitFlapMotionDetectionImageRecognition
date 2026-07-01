#pragma once
#include "customtypes.h"


class IApp
{
public:
	virtual ~IApp() = default;

	virtual Config& GetConfig() = 0;

};

