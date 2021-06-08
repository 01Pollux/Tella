#pragma once

#include <stdint.h>
#include <compare>
#include "Enums.hpp"

class SteamID
{
public:
	SteamID(uint32_t accid = 0, SteamUniverse accuni = SteamUniverse::Invalid, SteamAccountType acctype = SteamAccountType::Invalid, uint32_t accinst = 1) noexcept :
		AccountID(accid), AccountType(acctype), AccountUniverse(accuni), AccountInstance(accinst)
	{ }
	
	SteamID(uint64_t info) noexcept : SteamInfo64(info) { }

	/// <summary> Sets parameters for steam ID </summary>
	/// 
	/// <param name="accid"> 32-bit account ID </param>
	/// <param name="accuni"> Universe this account belongs to </param>
	/// <param name="acctype"> Type of account </param>
	void set(uint32_t accid, SteamUniverse accuni, SteamAccountType acctype) noexcept
	{
		AccountID = accid;
		AccountUniverse = accuni;
		AccountType = acctype;

		if (acctype == SteamAccountType::Clan || acctype == SteamAccountType::GameServer)
			AccountInstance = 0;
		else
			AccountInstance = 1;
	}

	void clear() noexcept
	{
		set(0, SteamUniverse::Invalid, SteamAccountType::Invalid);
	}

	uint64_t to_uint64() const noexcept
	{
		return SteamInfo64;
	}

	/// <summary>
	/// Is this an anonymous game server login that will be filled in?
	/// </summary>
	bool BBlankAnonAccount() const noexcept
	{
		return AccountID == 0 && BAnonAccount() && AccountInstance == 0;
	}

	/// <summary>
	/// Is this a game server account id?  (Either persistent or anonymous)
	/// </summary>
	bool BGameServerAccount() const noexcept
	{
		return BPersistentGameServerAccount() || BAnonGameServerAccount();
	}

	/// <summary>
	/// Is this a persistent (not anonymous) game server account id?
	/// </summary>
	bool BPersistentGameServerAccount() const noexcept
	{
		return AccountType == SteamAccountType::GameServer;
	}

	/// <summary>
	/// Purpose: Is this an anonymous game server account id?
	/// </summary>
	bool BAnonGameServerAccount() const noexcept
	{
		return AccountType == SteamAccountType::AnonGameServer;
	}

	/// <summary>
	/// Is this a content server account id?
	/// </summary>
	bool BContentServerAccount() const noexcept
	{
		return AccountType == SteamAccountType::ContentServer;
	}

	/// <summary>
	/// Is this a clan account id?
	/// </summary>
	bool BClanAccount() const noexcept
	{
		return AccountType == SteamAccountType::Clan;
	}

	/// <summary>
	/// Is this a chat account id?
	/// </summary>
	bool BChatAccount() const noexcept
	{
		return AccountType == SteamAccountType::Chat;
	}

	/// <summary>
	///  Is this a chat account id?
	/// </summary>
	bool IsLobby() const noexcept
	{
		return BChatAccount() && AccountInstance & 0x40000U;
	}

	/// <summary>
	/// Is this an individual user account id?
	/// </summary>
	bool BIndividualAccount() const noexcept
	{
		return AccountType == SteamAccountType::Individual || AccountType == SteamAccountType::ConsoleUser;
	}

	/// <summary>
	/// Is this an anonymous account?
	/// </summary>
	bool BAnonAccount() const noexcept
	{
		return AccountType == SteamAccountType::AnonUser || AccountType == SteamAccountType::AnonGameServer;
	}

	/// <summary>
	/// Is this an anonymous user account? (used to create an account or reset a password)
	/// </summary>
	bool BAnonUserAccount() const noexcept
	{
		return AccountType == SteamAccountType::AnonUser;
	}

	/// <summary>
	/// Is this a faked up Steam ID for a PSN friend account?
	/// </summary>
	bool BConsoleUserAccount() const noexcept
	{
		return AccountType == SteamAccountType::ConsoleUser;
	}

	bool Valid() const noexcept
	{
		if (AccountType <= SteamAccountType::Invalid || AccountType >= SteamAccountType::Count)
			return false;

		if (AccountUniverse <= SteamUniverse::Invalid || AccountUniverse >= SteamUniverse::Count)
			return false;

		switch (AccountType)
		{
		case SteamAccountType::Individual:
			return AccountID != 0 && AccountInstance <= 4;
			[[fallthrough]];
		case SteamAccountType::Clan:
			return AccountID != 0 && !AccountInstance;
			[[fallthrough]];
		case SteamAccountType::GameServer:
			return AccountID != 0;
			[[fallthrough]];
		default:
			return true;
		}

	}
	constexpr auto operator<=>(const SteamID& other) const noexcept { return SteamInfo64 <=> other.SteamInfo64; }

public:
	union
	{
		struct
		{
			uint32_t			AccountID : 32;			// unique account identifier
			uint32_t			AccountInstance : 20;	// dynamic instance ID
			SteamAccountType	AccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			SteamUniverse		AccountUniverse : 8;	// universe this account belongs to
		};
		uint64_t SteamInfo64;
	};
};