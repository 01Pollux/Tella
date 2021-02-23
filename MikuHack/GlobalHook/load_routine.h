#pragma once

#include "../Helpers/AutoList.h"

class IMainRoutine: public IAutoList<IMainRoutine>
{
public:
	virtual void OnLoadDLL() { };
	virtual void OnUnloadDLL() { };
};