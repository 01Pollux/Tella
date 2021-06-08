#pragma once

#include "Interfaces.hpp"

class bf_write;
class bf_read;
class IGameEventManager;

#define GAMEEVENT_CLIENT_SIDE false
#define GAMEEVENT_SERVER_SIDE true

namespace Interfaces
{
	extern IGameEventManager* GameEventMgr;
	constexpr const char* GameEventMgrName = "GAMEEVENTSMANAGER002";
}


class IGameEvent
{
public:
	virtual ~IGameEvent() = default;
	virtual const char* GetName() const abstract;

	virtual bool  IsReliable() const abstract; 
	virtual bool  IsLocal() const abstract;
	virtual bool  IsEmpty(const char* keyName = nullptr) abstract;

	virtual bool  GetBool(const char* keyName = nullptr, bool defaultValue = false) abstract;
	virtual int   GetInt(const char* keyName = nullptr, int defaultValue = 0) abstract;
	virtual float GetFloat(const char* keyName = nullptr, float defaultValue = 0.0f) abstract;
	virtual const char* GetString(const char* keyName = nullptr, const char* defaultValue = "") abstract;

	virtual void SetBool(const char* keyName, bool value) abstract;
	virtual void SetInt(const char* keyName, int value) abstract;
	virtual void SetFloat(const char* keyName, float value) abstract;
	virtual void SetString(const char* keyName, const char* value) abstract;
};


class IGameEventListener
{
public:
	virtual	~IGameEventListener() = default;

	// FireEvent is called by EventManager if event just occurred
	// KeyValue memory will be freed by manager if not needed anymore
	virtual void FireGameEvent(IGameEvent* event) abstract;
}; 


class IGameEventManager : public IBaseInterface
{
public:
	virtual	~IGameEventManager() = default;

	// load game event descriptions from a file eg "resource\gameevents.res"
	virtual int LoadEventsFromFile(const char* filename) abstract;

	// removes all and anything
	virtual void  Reset() abstract;

	// adds a listener for a particular event
	virtual bool AddListener(IGameEventListener* listener, const char* name, bool bServerSide) abstract;

	// returns true if this listener is listens to given event
	virtual bool FindListener(IGameEventListener* listener, const char* name) abstract;

	// removes a listener 
	virtual void RemoveListener(IGameEventListener* listener) abstract;

	// create an event by name, but doesn't fire it. returns NULL is event is not
	// known or no listener is registered for it. bForce forces the creation even if no listener is active
	virtual IGameEvent* CreateGameEvent(const char* name, bool bForce = false) abstract;

	// fires a server event created earlier, if bDontBroadcast is set, event is not send to clients
	virtual bool FireEvent(IGameEvent* event, bool bDontBroadcast = false) abstract;

	// fires an event for the local client only, should be used only by client code
	virtual bool FireEventClientSide(IGameEvent* event) abstract;

	// create a new copy of this event, must be free later
	virtual IGameEvent* DuplicateEvent(IGameEvent* event) abstract;

	// if an event was created but not fired for some reason, it has to bee freed, same UnserializeEvent
	virtual void FreeEvent(IGameEvent* event) abstract;

	// write/read event to/from bitbuffer
	virtual bool SerializeEvent(IGameEvent* event, bf_write* buf) abstract;
	virtual IGameEvent* UnserializeEvent(bf_read* buf) abstract; // create new KeyValues, must be deleted
};