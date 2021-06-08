#pragma once

#define M0_INTERFACE \
namespace Interfaces \
{

#define M0_END }


M0_INTERFACE;

void InitAllInterfaces();

M0_END;

class IBaseInterface
{
public:
	virtual	~IBaseInterface() = default;
};