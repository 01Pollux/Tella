#pragma once 

enum class EResult
{
	OK = 1,								// success
	Fail = 2,							// generic failure 
	NoConnection = 3,					// no/failed network connection
//	NoConnectionRetry = 4,				// OBSOLETE - removed
	InvalidPassword = 5,				// password/ticket is invalid
	LoggedInElsewhere = 6,				// same user logged in elsewhere
	InvalidProtocolVer = 7,				// protocol version is incorrect
	InvalidParam = 8,					// a parameter is incorrect
	FileNotFound = 9,					// file was not found
	Busy = 10,							// called method busy - action not taken
	InvalidState = 11,					// called object was in an invalid state
	InvalidName = 12,					// name is invalid
	InvalidEmail = 13,					// email is invalid
	DuplicateName = 14,					// name is not unique
	AccessDenied = 15,					// access is denied
	Timeout = 16,						// operation timed out
	Banned = 17,						// VAC2 banned
	AccountNotFound = 18,				// account not found
	InvalidSteamID = 19,				// steamID is invalid
	ServiceUnavailable = 20,			// The requested service is currently unavailable
	NotLoggedOn = 21,					// The user is not logged on
	Pending = 22,						// Request is pending (may be in process, or waiting on third party)
	EncryptionFailure = 23,				// Encryption or Decryption failed
	InsufficientPrivilege = 24,			// Insufficient privilege
	LimitExceeded = 25,					// Too much of a good thing
	Revoked = 26,						// Access has been revoked (used for revoked guest passes)
	Expired = 27,						// License/Guest pass the user is trying to access is expired
	AlreadyRedeemed = 28,				// Guest pass has already been redeemed by account, cannot be acked again
	DuplicateRequest = 29,				// The request is a duplicate and the action has already occurred in the past, ignored this time
	AlreadyOwned = 30,					// All the games in this guest pass redemption request are already owned by the user
	IPNotFound = 31,					// IP address not found
	PersistFailed = 32,					// failed to write change to the data store
	LockingFailed = 33,					// failed to acquire access lock for this operation
	LogonSessionReplaced = 34,
	ConnectFailed = 35,
	HandshakeFailed = 36,
	IOFailure = 37,
	RemoteDisconnect = 38,
	ShoppingCartNotFound = 39,			// failed to find the shopping cart requested
	Blocked = 40,						// a user didn't allow it
	Ignored = 41,						// target is ignoring sender
	NoMatch = 42,						// nothing matching the request found
	AccountDisabled = 43,
	ServiceReadOnly = 44,				// this service is not accepting content changes right now
	AccountNotFeatured = 45,			// account doesn't have value, so this feature isn't available
	AdministratorOK = 46,				// allowed to take this action, but only because requester is admin
	ContentVersion = 47,				// A Version mismatch in content transmitted within the Steam protocol.
	TryAnotherCM = 48,					// The current CM can't service the user making a request, user should try another.
	PasswordRequiredToKickSession = 49,	// You are already logged in elsewhere, this cached credential login has failed.
	AlreadyLoggedInElsewhere = 50,		// You are already logged in elsewhere, you must wait
	Suspended = 51,						// Long running operation (content download) suspended/paused
	Cancelled = 52,						// Operation canceled (typically by user: content download)
	DataCorruption = 53,				// Operation canceled because data is ill formed or unrecoverable
	DiskFull = 54,						// Operation canceled - not enough disk space.
	RemoteCallFailed = 55,				// an remote call or IPC call failed
	PasswordUnset = 56,					// Password could not be verified as it's unset server side
	ExternalAccountUnlinked = 57,		// External account (PSN, Facebook...) is not linked to a Steam account
	PSNTicketInvalid = 58,				// PSN ticket was invalid
	ExternalAccountAlreadyLinked = 59,	// External account (PSN, Facebook...) is already linked to some other account, must explicitly request to replace/delete the link first
	RemoteFileConflict = 60,			// The sync cannot resume due to a conflict between the local and remote files
	IllegalPassword = 61,				// The requested new password is not legal
	SameAsPreviousValue = 62,			// new value is the same as the old one ( secret question and answer )
	AccountLogonDenied = 63,			// account login denied due to 2nd factor authentication failure
	CannotUseOldPassword = 64,			// The requested new password is not legal
	InvalidLoginAuthCode = 65,			// account login denied due to auth code invalid
	AccountLogonDeniedNoMail = 66,		// account login denied due to 2nd factor auth failure - and no mail has been sent
	HardwareNotCapableOfIPT = 67,		// 
	IPTInitError = 68,					// 
	ParentalControlRestricted = 69,		// operation failed due to parental control restrictions for current user
	FacebookQueryError = 70,			// Facebook query returned an error
	ExpiredLoginAuthCode = 71,			// account login denied due to auth code expired
	IPLoginRestrictionFailed = 72,
	AccountLockedDown = 73,
	AccountLogonDeniedVerifiedEmailRequired = 74,
	NoMatchingURL = 75,
	BadResponse = 76,					// parse failure, missing field, etc.
	RequirePasswordReEntry = 77,		// The user cannot complete the action until they re-enter their password
	ValueOutOfRange = 78,				// the value entered is outside the acceptable range
	UnexpectedError = 79,				// something happened that we didn't expect to ever happen
	Disabled = 80,						// The requested service has been configured to be unavailable
	InvalidCEGSubmission = 81,			// The set of files submitted to the CEG server are not valid !
	RestrictedDevice = 82,				// The device being used is not allowed to perform this action
	RegionLocked = 83,					// The action could not be complete because it is region restricted
	RateLimitExceeded = 84,				// Temporary rate limit exceeded, try again later, different from LimitExceeded which may be permanent
	AccountLoginDeniedNeedTwoFactor = 85,// Need two-factor code to login
	ItemDeleted = 86,					// The thing we're trying to access has been deleted
	AccountLoginDeniedThrottle = 87,	// login attempt failed, try to throttle response to possible attacker
	TwoFactorCodeMismatch = 88,			// two factor code mismatch
	TwoFactorActivationCodeMismatch = 89,// activation code for two-factor didn't match
	AccountAssociatedToMultiplePartners = 90,// account has been associated with multiple partners
	NotModified = 91,					// data not modified
	NoMobileDevice = 92,				// the account does not have a mobile device associated with it
	TimeNotSynced = 93,					// the time presented is out of range or tolerance
	SmsCodeFailed = 94,					// SMS code failure (no match, none pending, etc.)
	AccountLimitExceeded = 95,			// Too many accounts access this resource
	AccountActivityLimitExceeded = 96,	// Too many changes to this account
	PhoneActivityLimitExceeded = 97,	// Too many changes to this phone
	RefundToWallet = 98,				// Cannot refund to payment method, must use wallet
	EmailSendFailure = 99,				// Cannot send an email
	NotSettled = 100,					// Can't perform operation till payment has settled
	NeedCaptcha = 101,					// Needs to provide a valid captcha
};

// Error codes for use with the voice functions
enum class EVoiceResult
{
	OK = 0,
	NotInitialized = 1,
	NotRecording = 2,
	NoData = 3,
	BufferTooSmall = 4,
	DataCorrupted = 5,
	Restricted = 6,
	UnsupportedCodec = 7,
	ReceiverOutOfDate = 8,
	ReceiverDidNotAnswer = 9,

};

// Result codes to GSHandleClientDeny/Kick
enum class EDenyReason
{
	Invalid = 0,
	InvalidVersion = 1,
	Generic = 2,
	NotLoggedOn = 3,
	NoLicense = 4,
	Cheater = 5,
	LoggedInElseWhere = 6,
	UnknownText = 7,
	IncompatibleAnticheat = 8,
	MemoryCorruption = 9,
	IncompatibleSoftware = 10,
	SteamConnectionLost = 11,
	SteamConnectionError = 12,
	SteamResponseTimedOut = 13,
	SteamValidationStalled = 14,
	SteamOwnerLeftGuestUser = 15,
};


// results from BeginAuthSession
enum class EBeginAuthSessionResult
{
	OK = 0,						// Ticket is valid for this game and this steamID.
	InvalidTicket = 1,			// Ticket is not valid.
	DuplicateRequest = 2,		// A ticket has already been submitted for this steamID
	InvalidVersion = 3,			// Ticket is from an incompatible interface version
	GameMismatch = 4,			// Ticket is not for this game
	ExpiredTicket = 5,			// Ticket has expired
};

// Callback values for callback ValidateAuthTicketResponse_t which is a response to BeginAuthSession
enum class EAuthSessionResponse
{
	OK = 0,							// Steam has verified the user is online, the ticket is valid and ticket has not been reused.
	UserNotConnectedToSteam = 1,		// The user in question is not connected to steam
	NoLicenseOrExpired = 2,			// The license has expired.
	VACBanned = 3,					// The user is VAC banned for this game.
	LoggedInElseWhere = 4,			// The user account has logged in elsewhere and the session containing the game instance has been disconnected.
	VACCheckTimedOut = 5,				// VAC has been unable to perform anti-cheat checks on this user
	AuthTicketCanceled = 6,			// The ticket has been canceled by the issuer
	AuthTicketInvalidAlreadyUsed = 7,	// This ticket has already been used, it is not valid.
	AuthTicketInvalid = 8,			// This ticket is not from a user instance currently connected to steam.
	PublisherIssuedBan = 9,			// The user is banned for this game. The ban came via the web api and not VAC
};

// results from UserHasLicenseForApp
enum class EUserHasLicenseForAppResult
{
	HasLicense = 0,					// User has a license for specified app
	DoesNotHaveLicense = 1,			// User does not have a license for the specified app
	NoAuth = 2,						// User has not been authenticated
};

enum class SteamUniverse
{
	Invalid = 0,
	Public = 1,
	Beta = 2,
	Internal = 3,
	Dev = 4,

	Count,
};

enum class SteamAccountType : unsigned int
{
	Invalid = 0,
	Individual,		// single user account
	Multiseat,		// multiseat (e.g. cybercafe) account
	GameServer,		// game server account
	AnonGameServer,	// anonymous game server account
	Pending,		// pending
	ContentServer,	// content server
	Clan,
	Chat,
	ConsoleUser,	// Fake SteamID for local PSN account on PS3 or Live account on 360, etc.
	AnonUser,

	Count,
};


using HAuthTicket = unsigned int;
constexpr HAuthTicket HAuthTicketInvalid = 0U;
using SteamAppId = unsigned int;
