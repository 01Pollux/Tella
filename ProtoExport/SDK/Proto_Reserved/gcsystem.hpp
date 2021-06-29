#pragma once

#include "GCSys.hpp"

namespace gcsdk
{
#pragma pack( push, 8 ) // this is a 8 instead of a 1 to maintain backward compatibility with Steam

	// generic zero-length message struct
	struct MsgGCEmpty
	{

	};

	// k_EGCMsgAchievementAwarded 
	struct MsgGCAchievementAwarded
	{
		uint16_t StatID;
		uint8_t	 Bit;
		// var data:
		//    string data: name of achievement earned
	};

	// k_EGCMsgConCommand
	struct MsgGCConCommand
	{
		// var data:
		//		string: the command as typed into the console
	};


	// k_EGCMsgStartPlaying
	struct MsgGCStartPlaying
	{
		SteamID steamID;
		SteamID SteamIDGS;
		uint32_t ServerAddr;
		uint16_t ServerPort;
	};


	// k_EGCMsgStartPlaying
	// k_EGCMsgStopGameserver
	struct MsgGCStopSession
	{
		SteamID m_steamID;
	};


	// k_EGCMsgStartGameserver
	struct MsgGCStartGameserver
	{
		SteamID steamID;
		uint32_t ServerAddr;
		uint16_t ServerPort;
	};

	// k_EGCMsgWGRequest
	struct MsgGCWGRequest
	{
		uint64_t steamID;		//SteamID of auth'd WG user
		uint32_t Privilege;	// The EGCWebApiPrivilege value that the request was made with
		uint32_t cubKeyValues;	// length of the key values data blob in message (starts after string request name data)
		// var data - 
		//		request name
		//		binary key values of web request
	};

	// k_EGCMsgWGResponse
	struct MsgGCWGResponse
	{
		bool	 Result;			// True if the request was successful
		uint32_t cubKeyValues;	// length of the key values data blob in message
		// var data - 
		//		binary key values of web response
	};


	// k_EGCMsgGetUserGameStatsSchemaResponse
	struct MsgGetUserGameStatsSchemaResponse
	{
		bool Success;		// True is the request was successful
		// var data -
		//		binary key values containing the User Game Stats schema
	};


	// k_EGCMsgGetUserStats
	struct MsgGetUserStats
	{
		SteamID steamID;	// SteamID the stats are requested for
		uint16_t StatIDs;		// A count of the number of statIDs requested
		// var data -
		//		Array of m_cStatIDs 16-bit StatIDs
	};


	// k_EGCMsgGetUserStatsResponse
	struct MsgGetUserStatsResponse
	{
		SteamID  steamID;	// SteamID the stats were requested for
		bool	 Success;		// True is the request was successful
		uint16_t Stats;		// Number of stats returned in the message
		// var data -
		//		m_cStats instances of:
		//			uint16 usStatID - Stat ID
		//			uint32 unData   - Stat value
	};

	// k_EGCMsgValidateSession
	struct MsgGCValidateSession
	{
		SteamID steamID;	// SteamID to validate
	};

	// k_EGCMsgValidateSessionResponse
	struct MsgGCValidateSessionResponse
	{
		SteamID	 steamID;
		uint64_t SteamIDGS;
		uint32_t ServerAddr;
		uint16_t ServerPort;
		bool	 Online;
	};

	// response to k_EGCMsgLookupAccountFromInput
	struct MsgGCLookupAccountResponse
	{
		SteamID	steamID;
	};

	// k_EGCMsgSendHTTPRequest
	struct MsgGCSendHTTPRequest
	{
		// Variable data:
		//	- Serialized CHTTPRequest
	};

	// k_EGCMsgSendHTTPRequestResponse
	struct MsgGCSendHTTPRequestResponse
	{
		bool m_bCompleted;
		// Variable data:
		//	- if m_bCompleted is true, Serialized CHTTPResponse
	};


	// k_EGCMsgRecordSupportAction
	struct MsgGCRecordSupportAction
	{
		uint32_t AccountID;		// which  account is affected (object)
		uint32_t ActorID;		// who made the change (subject)
		// Variable data:
		//	- string - Custom data for the event
		//  - string - A note with the reason for the change
	};


	// k_EGCMsgWebAPIRegisterInterfaces
	struct MsgGCWebAPIRegisterInterfaces
	{
		uint32_t Interfaces;
		// Variable data:
		// - KeyValues for interface - one per interface
	};

	// k_EGCMsgGetAccountDetails
	struct MsgGCGetAccountDetails
	{
		SteamID steamID;	// SteamID to validate
	};


	// Used by k_EGCMsgFindAccounts
	enum class AccountFindType
	{
		Invalid = 0,
		AccountName = 1,
		Email,
		PersonaName,
		URL,
		AllOnline,
		All,
		ClanName,
		ClanURL,
		OfficialURL,
		AppID,
	};


#pragma pack(pop)
}