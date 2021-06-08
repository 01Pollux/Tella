#pragma once

#include <cstdint>
#include "MathLib/Vector3D.hpp"
#include "MathLib/VMatrix.hpp"
#include "Const.hpp"

class IVEngineClient;
class IBaseClientDLL;

class IGamestatsData;
class IAchievementMgr;
struct ModelInfo;
class SurfInfo;
class IMaterial;
class client_textmessage_t;
class ISentence;
class IAudioSource;
class ISpatialQuery;
class KeyValues;
class IMaterialSystem;
struct AudioState_t;
class INetChannel;
class IPhysCollide;
class IValveGlobalVars;
class ClientClass;
class bf_write;
class bf_read;
struct vrect_t;
class IViewSetup;
struct ScreenFade_t;
class IEngineSprite;
class ISaveRestoreData;
class EntDataMap;
class EntTypeDescription;
class IStandardRecvProxies;

template<typename DataType>
class ValveUtlVector;

struct PlayerInfo
{
	char			Name[MAX_PLAYER_NAME_LENGTH];
	int				UserId;
	// global unique player identifer
	char			GlobalUId[SIGNED_GUID_LEN + 1];
	// friends identification number
	uint32_t		FriendsID;
	// friends name
	char			FriendsName[32];
	// true, if player is a bot controlled by game.dll
	bool			IsFakeplayer;
	// true if player is the HLTV proxy
	bool			Isshltv;
	// true if player is the Replay proxy
	bool			IsReplay;
	// custom files CRC for this player
	uint32_t		CustomFiles[MAX_CUSTOM_FILES];
	// this counter increases each time the server downloaded a new file
	unsigned char	FilesDownloaded;
};


namespace Interfaces
{
	extern IVEngineClient* EngineClient;
	constexpr const char* EngineClientName = "VEngineClient014";

	extern IBaseClientDLL* ClientDLL;
	constexpr const char* ClientDLLName = "VClient017";

	void ForceClientFullUpdate();
}

class IVEngineClient
{
public:
	// Find the model's surfaces that intersect the given sphere.
	// Returns the number of surfaces filled in.
	virtual int					GetIntersectingSurfaces(
		const ModelInfo* model,
		const Vector& vCenter,
		const float radius,
		const bool bOnlyVisibleSurfaces,	// Only return surfaces visible to vCenter.
		SurfInfo* pInfos,
		const int nMaxInfos) abstract;

	// Get the lighting intensivty for a specified point
	// If bClamp is specified, the resulting Vector is restricted to the 0.0 to 1.0 for each element
	virtual Vector				GetLightForPoint(const Vector& pos, bool bClamp) abstract;

	// Traces the line and reports the material impacted as well as the lighting information for the impact point
	virtual IMaterial* TraceLineMaterialAndLighting(const Vector& start, const Vector& end,
		Vector& diffuseLightColor, Vector& baseColor) abstract;

	// Given an input text buffer data pointer, parses a single token into the variable token and returns the new
	//  reading position
	virtual const char* ParseFile(const char* data, char* token, int maxlen) abstract;
	virtual bool				CopyLocalFile(const char* source, const char* destination) abstract;

	// Gets the dimensions of the game window
	virtual void				GetScreenSize(int& width, int& height) abstract;

	// Forwards szCmdString to the server, sent reliably if bReliable is set
	virtual void				ServerCmd(const char* szCmdString, bool bReliable = true) abstract;
	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// Note: Calls to this are checked against FCVAR_CLIENTCMD_CAN_EXECUTE (if that bit is not set, then this function can't change it).
	//       Call ClientCmd_Unrestricted to have access to FCVAR_CLIENTCMD_CAN_EXECUTE vars.
	virtual void				ClientCmd(const char* szCmdString) abstract;

	// Fill in the player info structure for the specified player index (name, model, etc.)
	virtual bool				GetPlayerInfo(int ent_num, PlayerInfo* pinfo) abstract;

	// Retrieve the player entity number for a specified userID
	virtual int					GetPlayerForUserID(int userID) abstract;

	// Retrieves text message system information for the specified message by name
	virtual client_textmessage_t* TextMessageGet(const char* pName) abstract;

	// Returns true if the console is visible
	virtual bool				Con_IsVisible() abstract;

	// Get the entity index of the local player
	virtual int					GetLocalPlayer() abstract;

	// Client DLL is hooking a model, loads the model into memory and returns  pointer to the ModelInfo
	virtual const ModelInfo* LoadModel(const char* pName, bool bProp = false) abstract;

	// Get accurate, sub-frame clock ( profiling use )
	virtual float				Time() abstract;

	// Get the exact server timesstamp ( server time ) from the last message received from the server
	virtual float				GetLastTimeStamp() abstract;

	// Given a CAudioSource (opaque pointer), retrieve the underlying CSentence object ( stores the words, phonemes, and close
	//  captioning data )
	virtual ISentence* GetSentence(IAudioSource* pAudioSource) abstract;
	// Given a CAudioSource, determines the length of the underlying audio file (.wav, .mp3, etc.)
	virtual float				GetSentenceLength(IAudioSource* pAudioSource) abstract;
	// Returns true if the sound is streaming off of the hard disk (instead of being memory resident)
	virtual bool				IsStreaming(IAudioSource* pAudioSource) const = 0;

	// Copy current view orientation into va
	virtual void				GetViewAngles(QAngle& va) abstract;
	// Set current view orientation from va
	virtual void				SetViewAngles(QAngle& va) abstract;

	// Retrieve the current game's maxclients setting
	virtual int					GetMaxClients() abstract;

	// Given the string pBinding which may be bound to a key, 
	//  returns the string name of the key to which this string is bound. Returns NULL if no such binding exists
	virtual	const char*			Key_LookupBinding(const char* pBinding) abstract;

	// Given the name of the key "mouse1", "e", "tab", etc., return the string it is bound to "+jump", "impulse 50", etc.
	virtual const char*			Key_BindingForKey(int code) abstract;

	// key trapping (for binding keys)
	virtual void				StartKeyTrapMode() abstract;
	virtual bool				CheckDoneKeyTrapping(int& code) abstract;

	// Returns true if the player is fully connected and active in game (i.e, not still loading)
	virtual bool				IsInGame() abstract;
	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
	virtual bool				IsConnected() abstract;
	// Returns true if the loading plaque should be drawn
	virtual bool				IsDrawingLoadingImage() abstract;

	// Prints the formatted string to the notification area of the screen ( down the right hand edge
	//  numbered lines starting at position 0
	virtual void				Con_NPrintf(int pos, const char* fmt, ...) abstract;
	// Similar to Con_NPrintf, but allows specifying custom text color and duration information
	virtual void				Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...) abstract;

	// Is the specified world-space bounding box inside the view frustum?
	virtual int					IsBoxVisible(const Vector& mins, const Vector& maxs) abstract;

	// Is the specified world-space boudning box in the same PVS cluster as the view origin?
	virtual int					IsBoxInViewCluster(const Vector& mins, const Vector& maxs) abstract;

	// Returns true if the specified box is outside of the view frustum and should be culled
	virtual bool				CullBox(const Vector& mins, const Vector& maxs) abstract;

	// Allow the sound system to paint additional data (during lengthy rendering operations) to prevent stuttering sound.
	virtual void				Sound_ExtraUpdate() abstract;

	// Get the current game directory ( e.g., hl2, tf2, cstrike, hl1 )
	virtual const char* GetGameDirectory() abstract;

	// Get access to the world to screen transformation matrix
	virtual const VMatrix& WorldToScreenMatrix() abstract;

	// Get the matrix to move a point from world space into view space
	// (translate and rotate so the camera is at the origin looking down X).
	virtual const VMatrix& WorldToViewMatrix() abstract;

	// The .bsp file can have mod-specified data lumps. These APIs are for working with such game lumps.

	// Get mod-specified lump version id for the specified game data lump
	virtual int					GameLumpVersion(int lumpId) const = 0;
	// Get the raw size of the specified game data lump.
	virtual int					GameLumpSize(int lumpId) const = 0;
	// Loads a game lump off disk, writing the data into the buffer pointed to bye pBuffer
	// Returns false if the data can't be read or the destination buffer is too small
	virtual bool				LoadGameLump(int lumpId, void* pBuffer, int size) abstract;

	// Returns the number of leaves in the level
	virtual int					LevelLeafCount() const = 0;

	// Gets a way to perform spatial queries on the BSP tree
	virtual ISpatialQuery* GetBSPTreeQuery() abstract;

	// Convert texlight to gamma...
	virtual void		LinearToGamma(float* linear, float* gamma) abstract;

	// Get the lightstyle value
	virtual float		LightStyleValue(int style) abstract;

	// Computes light due to dynamic lighting at a point
	// If the normal isn't specified, then it'll return the maximum lighting
	virtual void		ComputeDynamicLighting(const Vector& pt, const Vector* pNormal, Vector& color) abstract;

	// Returns the color of the ambient light
	virtual void		GetAmbientLightColor(Vector& color) abstract;

	// Returns the dx support level
	virtual int			GetDXSupportLevel() abstract;

	// GR - returns the HDR support status
	virtual bool        SupportsHDR() abstract;

	// Replace the engine's material system pointer.
	virtual void		Mat_Stub(IMaterialSystem* pMatSys) abstract;

	// Get the name of the current map
	virtual void GetChapterName(char* pchBuff, int iMaxLength) abstract;
	virtual char const* GetLevelName() abstract;
	virtual int	GetLevelVersion() abstract;
#if !defined( NO_VOICE )
	// Obtain access to the voice tweaking API
	virtual struct IVoiceTweak_s* GetVoiceTweakAPI() abstract;
#endif
	// Tell engine stats gathering system that the rendering frame is beginning/ending
	virtual void		EngineStats_BeginFrame() abstract;
	virtual void		EngineStats_EndFrame() abstract;

	// This tells the engine to fire any events (temp entity messages) that it has queued up this frame. 
	// It should only be called once per frame.
	virtual void		FireEvents() abstract;

	// Returns an area index if all the leaves are in the same area. If they span multple areas, then it returns -1.
	virtual int			GetLeavesArea(int* pLeaves, int nLeaves) abstract;

	// Returns true if the box touches the specified area's frustum.
	virtual bool		DoesBoxTouchAreaFrustum(const Vector& mins, const Vector& maxs, int iArea) abstract;

	// Sets the hearing origin (i.e., the origin and orientation of the listener so that the sound system can spatialize 
	//  sound appropriately ).
	virtual void		SetAudioState(const AudioState_t& state) abstract;

	// Sentences / sentence groups
	virtual int			SentenceGroupPick(int groupIndex, char* name, int nameBufLen) abstract;
	virtual int			SentenceGroupPickSequential(int groupIndex, char* name, int nameBufLen, int sentenceIndex, int reset) abstract;
	virtual int			SentenceIndexFromName(const char* pSentenceName) abstract;
	virtual const char* SentenceNameFromIndex(int sentenceIndex) abstract;
	virtual int			SentenceGroupIndexFromName(const char* pGroupName) abstract;
	virtual const char* SentenceGroupNameFromIndex(int groupIndex) abstract;
	virtual float		SentenceLength(int sentenceIndex) abstract;

	// Computes light due to dynamic lighting at a point
	// If the normal isn't specified, then it'll return the maximum lighting
	// If pBoxColors is specified (it's an array of 6), then it'll copy the light contribution at each box side.
	virtual void		ComputeLighting(const Vector& pt, const Vector* pNormal, bool bClamp, Vector& color, Vector* pBoxColors = NULL) abstract;

	// Activates/deactivates an occluder...
	virtual void		ActivateOccluder(int nOccluderIndex, bool bActive) abstract;
	virtual bool		IsOccluded(const Vector& vecAbsMins, const Vector& vecAbsMaxs) abstract;

	// The save restore system allocates memory from a shared memory pool, use this allocator to allocate/free saverestore 
	//  memory.
	virtual void* SaveAllocMemory(size_t num, size_t size) abstract;
	virtual void		SaveFreeMemory(void* pSaveMem) abstract;

	// returns info interface for client netchannel
	virtual INetChannel* GetNetChannelInfo() abstract;

	// Debugging functionality:
	// Very slow routine to draw a physics model
	virtual void		DebugDrawPhysCollide(const IPhysCollide* pCollide, IMaterial* pMaterial, Matrix3x4& transform, const VColor32& color) abstract;
	// This can be used to notify test scripts that we're at a particular spot in the code.
	virtual void		CheckPoint(const char* pName) abstract;
	// Draw portals if r_DrawPortals is set (Debugging only)
	virtual void		DrawPortals() abstract;
	// Determine whether the client is playing back or recording a demo
	virtual bool		IsPlayingDemo() abstract;
	virtual bool		IsRecordingDemo() abstract;
	virtual bool		IsPlayingTimeDemo() abstract;
	virtual int			GetDemoRecordingTick() abstract;
	virtual int			GetDemoPlaybackTick() abstract;
	virtual int			GetDemoPlaybackStartTick() abstract;
	virtual float		GetDemoPlaybackTimeScale() abstract;
	virtual int			GetDemoPlaybackTotalTicks() abstract;
	// Is the game paused?
	virtual bool		IsPaused() abstract;
	// Is the game currently taking a screenshot?
	virtual bool		IsTakingScreenshot() abstract;
	// Is this a HLTV broadcast ?
	virtual bool		IsHLTV() abstract;
	// is this level loaded as just the background to the main menu? (active, but unplayable)
	virtual bool		IsLevelMainMenuBackground() abstract;
	// returns the name of the background level
	virtual void		GetMainMenuBackgroundName(char* dest, int destlen) abstract;

	// Get video modes
	virtual void		GetVideoModes(int& nCount, VModeS*& pModes) abstract;

	// Occlusion system control
	virtual void		SetOcclusionParameters(const float(&)[2]) abstract;

	// What language is the user expecting to hear .wavs in, "english" or another...
	virtual void		GetUILanguage(char* dest, int destlen) abstract;

	// Can skybox be seen from a particular point?
	virtual SkyboxVisibility	IsSkyboxVisibleFromPoint(const Vector& vecPoint) abstract;

	// Get the pristine map entity lump string.  (e.g., used by CS to reload the map entities when restarting a round.)
	virtual const char* GetMapEntitiesString() abstract;

	// Is the engine in map edit mode ?
	virtual bool		IsInEditMode() abstract;

	// current screen aspect ratio (eg. 4.0f/3.0f, 16.0f/9.0f)
	virtual float		GetScreenAspectRatio() abstract;

	// allow the game UI to login a user
	virtual bool		REMOVED_SteamRefreshLogin(const char* password, bool isSecure) abstract;
	virtual bool		REMOVED_SteamProcessCall(bool& finished) abstract;

	// allow other modules to know about engine versioning (one use is a proxy for network compatability)
	virtual unsigned int	GetEngineBuildNumber() abstract; // engines build
	virtual const char* GetProductVersionString() abstract; // mods version number (steam.inf)

	// Communicates to the color correction editor that it's time to grab the pre-color corrected frame
	// Passes in the actual size of the viewport
	virtual void			GrabPreColorCorrectedFrame(int x, int y, int width, int height) abstract;

	virtual bool			IsHammerRunning() const = 0;

	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// And then executes the command string immediately (vs ClientCmd() which executes in the next frame)
	//
	// Note: this is NOT checked against the FCVAR_CLIENTCMD_CAN_EXECUTE vars.
	virtual void			ExecuteClientCmd(const char* szCmdString) abstract;

	// returns if the loaded map was processed with HDR info. This will be set regardless
	// of what HDR mode the player is in.
	virtual bool MapHasHDRLighting() abstract;

	virtual int	GetAppID() abstract;

	// Just get the leaf ambient light - no caching, no samples
	virtual Vector			GetLightForPointFast(const Vector& pos, bool bClamp) abstract;

	// This version does NOT check against FCVAR_CLIENTCMD_CAN_EXECUTE.
	virtual void			ClientCmd_Unrestricted(const char* szCmdString) abstract;

	// This used to be accessible through the cl_restrict_server_commands cvar.
	// By default, Valve games restrict the server to only being able to execute commands marked with FCVAR_SERVER_CAN_EXECUTE.
	// By default, mods are allowed to execute any server commands, and they can restrict the server's ability to execute client
	// commands with this function.
	virtual void			SetRestrictServerCommands(bool bRestrict) abstract;

	// If set to true (defaults to true for Valve games and false for others), then IVEngineClient::ClientCmd
	// can only execute things marked with FCVAR_CLIENTCMD_CAN_EXECUTE.
	virtual void			SetRestrictClientCommands(bool bRestrict) abstract;

	// Sets the client renderable for an overlay's material proxy to bind to
	virtual void			SetOverlayBindProxy(int iOverlayID, void* pBindProxy) abstract;

	virtual bool			CopyFrameBufferToMaterial(const char* pMaterialName) abstract;

	// Matchmaking
	virtual void			ChangeTeam(const char* pTeamName) abstract;

	// Causes the engine to read in the user's configuration on disk
	virtual void			ReadConfiguration(const bool readDefault = false) abstract;

	virtual void			SetAchievementMgr(IAchievementMgr* pAchievementMgr) abstract;
	virtual IAchievementMgr* GetAchievementMgr() abstract;

	virtual bool			MapLoadFailed() abstract;
	virtual void			SetMapLoadFailed(bool bState) abstract;

	virtual bool			IsLowViolence() abstract;
	virtual const char* GetMostRecentSaveGame() abstract;
	virtual void			SetMostRecentSaveGame(const char* lpszFilename) abstract;

	virtual void			StartXboxExitingProcess() abstract;
	virtual bool			IsSaveInProgress() abstract;
	virtual uint32_t		OnStorageDeviceAttached() abstract;
	virtual void			OnStorageDeviceDetached() abstract;

	virtual void			ResetDemoInterpolation() abstract;

	// Methods to set/get a gamestats data container so client & server running in same process can send combined data
	virtual void SetGamestatsData(IGamestatsData* pGamestatsData) abstract;
	virtual IGamestatsData* GetGamestatsData() abstract;

#if defined( USE_SDL )
	// we need to pull delta's from the cocoa mgr, the engine vectors this for us
	virtual void GetMouseDelta(int& x, int& y, bool bIgnoreNextMouseDelta = false) abstract;
#endif

	// Sends a key values server command, not allowed from scripts execution
	// Params:
	//	pKeyValues	- key values to be serialized and sent to server
	//				  the pointer is deleted inside the function: pKeyValues->deleteThis()
	virtual void			ServerCmdKeyValues(KeyValues* pKeyValues) abstract;

	virtual bool			IsSkippingPlayback() abstract;
	virtual bool			IsLoadingDemo() abstract;

	// Returns true if the engine is playing back a "locally recorded" demo, which includes
	// both SourceTV and replay demos, since they're recorded locally (on servers), as opposed
	// to a client recording a demo while connected to a remote server.
	virtual bool			IsPlayingDemoALocallyRecordedDemo() abstract;

	// Given the string pBinding which may be bound to a key, 
	//  returns the string name of the key to which this string is bound. Returns NULL if no such binding exists
	// Unlike Key_LookupBinding, leading '+' characters are not stripped from bindings.
	virtual	const char* Key_LookupBindingExact(const char* pBinding) abstract;

	virtual void			AddPhonemeFile(const char* pszPhonemeFile) abstract;
	virtual float			GetPausedExpireTime() abstract;

	virtual bool			StartDemoRecording(const char* pszFilename, const char* pszFolder = NULL) abstract;
	virtual void			StopDemoRecording() abstract;
	virtual void			TakeScreenshot(const char* pszFilename, const char* pszFolder = NULL) abstract;
};

class IBaseClientDLL
{
public:
	using CreateInterfaceFunction = void* (*)(const char*, int*);
	// Called once when the client DLL is loaded
	virtual int				Init(CreateInterfaceFunction appSystemFactory,
								 CreateInterfaceFunction physicsFactory,
								 IValveGlobalVars* pGlobals) abstract;

	virtual void			PostInit() abstract;

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown() abstract;

	// Called once the client is initialized to setup client-side replay interface pointers
	virtual bool			ReplayInit(CreateInterfaceFunction replayFactory) abstract;
	virtual bool			ReplayPostInit() abstract;

	// Called at the start of each level change
	virtual void			LevelInitPreEntity(char const* pMapName) abstract;
	// Called at the start of a new level, after the entities have been received and created
	virtual void			LevelInitPostEntity() abstract;
	// Called at the end of a level
	virtual void			LevelShutdown() abstract;

	// Request a pointer to the list of client datatable classes
	virtual ClientClass* GetAllClasses() abstract;

	// Called once per level to re-initialize any hud element drawing stuff
	virtual int				HudVidInit() abstract;
	// Called by the engine when gathering user input
	virtual void			HudProcessInput(bool bActive) abstract;
	// Called oncer per frame to allow the hud elements to think
	virtual void			HudUpdate(bool bActive) abstract;
	// Reset the hud elements to their initial states
	virtual void			HudReset() abstract;
	// Display a hud text message
	virtual void			HudText(const char* message) abstract;

	// Mouse Input Interfaces
	// Activate the mouse (hides the cursor and locks it to the center of the screen)
	virtual void			IN_ActivateMouse() abstract;
	// Deactivates the mouse (shows the cursor and unlocks it)
	virtual void			IN_DeactivateMouse() abstract;
	// This is only called during extra sound updates and just accumulates mouse x, y offets and recenters the mouse.
	//  This call is used to try to prevent the mouse from appearing out of the side of a windowed version of the engine if 
	//  rendering or other processing is taking too long
	virtual void			IN_Accumulate() abstract;
	// Reset all key and mouse states to their initial, unpressed state
	virtual void			IN_ClearStates() abstract;
	// If key is found by name, returns whether it's being held down in isdown, otherwise function returns false
	virtual bool			IN_IsKeyDown(const char* name, bool& isdown) abstract;
	// Raw keyboard signal, if the client .dll returns 1, the engine processes the key as usual, otherwise,
	//  if the client .dll returns 0, the key is swallowed.
	virtual int				IN_KeyEvent(int eventcode, int keynum, const char* pszCurrentBinding) abstract;

	// This function is called once per tick to create the player CUserCmd (used for prediction/physics simulation of the player)
	// Because the mouse can be sampled at greater than the tick interval, there is a separate input_sample_frametime, which
	//  specifies how much additional mouse / keyboard simulation to perform.
	virtual void			CreateMove(
								int sequence_number,			// sequence_number of this cmd
								float input_sample_frametime,	// Frametime for mouse input sampling
								bool active) abstract;				// True if the player is active (not paused)

	// If the game is running faster than the tick_interval framerate, then we do extra mouse sampling to avoid jittery input
	//  This code path is much like the normal move creation code, except no move is created
	virtual void			ExtraMouseSample(float frametime, bool active) abstract;

	// Encode the delta (changes) between the CUserCmd in slot from vs the one in slot to.  The game code will have
	//  matching logic to read the delta.
	virtual bool			WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand) abstract;
	// Demos need to be able to encode/decode CUserCmds to memory buffers, so these functions wrap that
	virtual void			EncodeUserCmdToBuffer(bf_write& buf, int slot) abstract;
	virtual void			DecodeUserCmdFromBuffer(bf_read& buf, int slot) abstract;

	// Set up and render one or more views (e.g., rear view window, etc.).  This called into RenderView below
	virtual void			View_Render(vrect_t* rect) abstract;

	// Allow engine to expressly render a view (e.g., during timerefresh)
	// See IVRenderView.h, PushViewFlags_t for nFlags values
	virtual void			RenderView(const IViewSetup& view, int nClearFlags, int whatToDraw) abstract;

	// Apply screen fade directly from engine
	virtual void			View_Fade(ScreenFade_t* pSF) abstract;

	// The engine has parsed a crosshair angle message, this function is called to dispatch the new crosshair angle
	virtual void			SetCrosshairAngle(const QAngle& angle) abstract;

	// Sprite (.spr) model handling code
	// Load a .spr file by name
	virtual void			InitSprite(IEngineSprite* pSprite, const char* loadname) abstract;
	// Shutdown a .spr file
	virtual void			ShutdownSprite(IEngineSprite* pSprite) abstract;
	// Returns sizeof( CEngineSprite ) so the engine can allocate appropriate memory
	virtual int				GetSpriteSize() const = 0;

	// Called when a player starts or stops talking.
	// entindex is -1 to represent the local client talking (before the data comes back from the server). 
	// entindex is -2 to represent the local client's voice being acked by the server.
	// entindex is GetPlayer() when the server acknowledges that the local client is talking.
	virtual void			VoiceStatus(int entindex, int bTalking) abstract;

	// Networked string table definitions have arrived, allow client .dll to 
	//  hook string changes with a callback function ( see INetworkStringTableClient.h )
	virtual void			InstallStringTableCallback(char const* tableName) abstract;

	// Notification that we're moving into another stage during the frame.
	virtual void			FrameStageNotify(ClientFrameStage curStage) abstract;

	// The engine has received the specified user message, this code is used to dispatch the message handler
	virtual bool			DispatchUserMessage(int msg_type, bf_read& msg_data) abstract;

	// Save/restore system hooks
	virtual ISaveRestoreData* SaveInit(int size) abstract;
	virtual void			SaveWriteFields(ISaveRestoreData*, const char*, void*, EntDataMap*, EntTypeDescription*, int) abstract;
	virtual void			SaveReadFields(ISaveRestoreData*, const char*, void*, EntDataMap*, EntTypeDescription*, int) abstract;
	virtual void			PreSave(ISaveRestoreData*) abstract;
	virtual void			Save(ISaveRestoreData*) abstract;
	virtual void			WriteSaveHeaders(ISaveRestoreData*) abstract;
	virtual void			ReadRestoreHeaders(ISaveRestoreData*) abstract;
	virtual void			Restore(ISaveRestoreData*, bool) abstract;
	virtual void			DispatchOnRestore() abstract;

	// Hand over the StandardRecvProxies in the client DLL's module.
	virtual IStandardRecvProxies* GetStandardRecvProxies() abstract;

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshot(const char* pFilename) abstract;

	// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
	//  the appropriate close caption if running with closecaption = 1
	virtual void			EmitSentenceCloseCaption(char const* tokenstream) abstract;
	// Emits a regular close caption by token name
	virtual void			EmitCloseCaption(char const* captionname, float duration) abstract;

	// Returns true if the client can start recording a demo now.  If the client returns false,
	// an error message of up to length bytes should be returned in errorMsg.
	virtual bool			CanRecordDemo(char* errorMsg, int length) const = 0;

	// Added interface

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshotOfSize(const char* pFilename, int width, int height) abstract;

	// Gets the current view
	virtual bool			GetPlayerView(IViewSetup& playerView) abstract;

	// Matchmaking
	virtual void			SetupGameProperties(ValveUtlVector<char[8]>& contexts, ValveUtlVector<char[16]>& properties) abstract;
	virtual uint32_t		GetPresenceID(const char* pIDName) abstract;
	virtual const char*		GetPropertyIdString(const uint32_t id) abstract;
	virtual void			GetPropertyDisplayString(uint32_t id, uint32_t value, char* pOutput, int nBytes) abstract;
	virtual void			StartStatsReporting(void* handle, bool bArbitrated) abstract;

	virtual void			InvalidateMdlCache() abstract;

	virtual void			IN_SetSampleTime(float frametime) abstract;
};

