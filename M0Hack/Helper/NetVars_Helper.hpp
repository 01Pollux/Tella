#pragma once

#include <stdint.h>

class RecvProp;
class RecvTable;
class IRecvProxyData;
class IClientNetworkable;
enum class EntClassID;

using ArrayLengthRecvProxyFn	= void	(*)	(void*, int, int);
using RecvVarProxyFn			= void	(*)	(const IRecvProxyData* pData, void* pThis, void* pOut);
using DataTableRecvVarProxyFn	= void	(*)	(const RecvProp* pProp, void** pOut, void* pData, int objectID);

enum class SendPropType
{
	Int,
	Float,
	Vector,
	VectorXY,
	String,
	Array,
	DataTable,

	Count,
};

class DVariant
{
public:
	union
	{
		float			FloatVar;
		int				IntVar;
		const char*		StringVar;
		void*			RawVar;
		float			VectorVar[3];
	};
	SendPropType		Type;
};

class IRecvProxyData
{
public:
	const RecvProp* RecvProp;
	DVariant		Value;
	int				Element;
	int				ObjectID;
};

class RecvProp
{
public:
	const char* VarName;
	SendPropType			RecvType;
	int						Flags;
	int						StringBufferSize;

	bool 					IsInsideArray;

	const void*				ExtraData;

	RecvProp*				ArrayProp;
	ArrayLengthRecvProxyFn	ArrayLengthProxy;

	RecvVarProxyFn			ProxyFn;
	DataTableRecvVarProxyFn	DataTableProxyFn;

	RecvTable*				DataTable;
	int						Offset;

	int						ElementStride;
	int						NumElements;

	const char*				ParentArrayPropName;
};

class RecvTable
{
public:

	RecvProp*		Props;
	int				NumProps;

	void*			Decoder;
	const char*		NetTableName;

	bool 			IsInitialized;
	bool 			IsInMainList;
};

class ClientClass
{
public:
	IClientNetworkable* (*CreateCallFunc)(int, int);
	IClientNetworkable* (*CreateEventFunc)();

	const char*			NetworkName;
	RecvTable*			RecvTable;

	ClientClass*		NextClass;
	EntClassID			ClassID;
};


struct CachedRecvInfo
{
	RecvProp*	Prop{ };
	uint32_t	Offset{ };
};



class NetVarHook
{
public:
	explicit NetVarHook(const char* class_name, const char* prop_name, RecvVarProxyFn callback)
	{
		this->init(class_name, prop_name, callback);

	}
	explicit NetVarHook(RecvProp* prop, RecvVarProxyFn callback) noexcept
	{
		this->init(prop, callback);
	}

	void execute(const IRecvProxyData* pData, void* pStruct, void* pOut) const noexcept
	{
		(this->ActualProxy)(pData, pStruct, pOut);
	}

	void init(const char* class_name, const char* prop_name, RecvVarProxyFn callback);

	void init(RecvProp* pProp, RecvVarProxyFn pCallback) noexcept
	{
		this->ActualProxy = pProp->ProxyFn;
		this->Prop = pProp;
		pProp->ProxyFn = *pCallback;
	}

	void shutdown() noexcept
	{
		this->Prop->ProxyFn = ActualProxy;
	}

	RecvProp* get() noexcept
	{
		return Prop;
	}
	
	const RecvProp* get() const noexcept
	{
		return Prop;
	}

public:
	NetVarHook() = default;					~NetVarHook() = default;
	NetVarHook(const NetVarHook&) = delete;	NetVarHook& operator=(const NetVarHook&) = delete;
	NetVarHook(NetVarHook&&) = default;		NetVarHook& operator=(NetVarHook&&) = default;

private:
	RecvVarProxyFn	ActualProxy;
	RecvProp*		Prop;
};



#define SPROP_UNSIGNED			(1<<0)					// Unsigned integer data.
#define SPROP_COORD				(1<<1)					// If this is set, the float/vector is treated like a world coordinate.
// Note that the bit count is ignored in this case.
#define SPROP_NOSCALE			(1<<2)					// For floating point, don't scale into range, just take value as is.
#define SPROP_ROUNDDOWN			(1<<3)					// For floating point, limit high value to range minus one bit unit
#define SPROP_ROUNDUP			(1<<4)					// For floating point, limit low value to range minus one bit unit
#define SPROP_NORMAL			(1<<5)					// If this is set, the vector is treated like a normal (only valid for vectors)
#define SPROP_EXCLUDE			(1<<6)					// This is an exclude prop (not excludED, but it points at another prop to be excluded).
#define SPROP_XYZE				(1<<7)					// Use XYZ/Exponent encoding for vectors.
#define SPROP_INSIDEARRAY		(1<<8)					// This tells us that the property is inside an array, so it shouldn't be put into the
														// flattened property list. Its array will point at it when it needs to.
#define SPROP_PROXY_ALWAYS_YES	(1<<9)					// Set for datatable props using one of the default datatable proxies like
														// SendProxy_DataTableToDataTable that always send the data to all clients.
#define SPROP_CHANGES_OFTEN		(1<<10)					// this is an often changed field, moved to head of sendtable so it gets a small index
#define SPROP_IS_A_VECTOR_ELEM	(1<<11)					// Set automatically if SPROP_VECTORELEM is used.
#define SPROP_COLLAPSIBLE		(1<<12)					// Set automatically if it's a datatable with an offset of 0 that doesn't change the pointer
														// (ie: for all automatically-chained base classes).
														// In this case, it can get rid of this SendPropDataTable altogether and spare the
														// trouble of walking the hierarchy more than necessary.
#define SPROP_COORD_MP					(1<<13)			// Like SPROP_COORD, but special handling for multiplayer games
#define SPROP_COORD_MP_LOWPRECISION 	(1<<14)			// Like SPROP_COORD, but special handling for multiplayer games where the fractional component only gets a 3 bits instead of 5
#define SPROP_COORD_MP_intEGRAL			(1<<15)			// SPROP_COORD_MP, but coordinates are rounded to integral boundaries
#define SPROP_VARint					SPROP_NORMAL	// reuse existing flag so we don't break demo. note you want to include SPROP_UNSIGNED if needed, its more efficient


enum class EntClassID
{
	CAI_BaseNPC,
	CBaseAnimating,
	CBaseAnimatingOverlay,
	CBaseAttributableItem,
	CBaseCombatCharacter,
	CBaseCombatWeapon,
	CBaseDoor,
	CBaseEntity,
	CBaseFlex,
	CBaseGrenade,
	CBaseObject,
	CBaseObjectUpgrade,
	CBaseParticleEntity,
	CBasePlayer,
	CBaseProjectile,
	CBasePropDoor,
	CBaseTeamObjectiveResource,
	CBaseTempEntity,
	CBaseViewModel,
	CBeam,
	CBoneFollower,
	CBonusDuckPickup,
	CBonusPack,
	CBonusRoundLogic,
	CBreakableProp,
	CBreakableSurface,
	CCaptureFlag,
	CCaptureFlagReturnIcon,
	CCaptureZone,
	CColorCorrection,
	CColorCorrectionVolume,
	CCurrencyPack,
	CDynamicLight,
	CDynamicProp,
	CEconEntity,
	CEconWearable,
	CEmbers,
	CEntityDissolve,
	CEntityFlame,
	CEntityParticleTrail,
	CEnvDetailController,
	CEnvParticleScript,
	CEnvProjectedTexture,
	CEnvQuadraticBeam,
	CEnvScreenEffect,
	CEnvScreenOverlay,
	CEnvTonemapController,
	CEnvWind,
	CEyeballBoss,
	CFireSmoke,
	CFireTrail,
	CFish,
	CFogController,
	CFunc_Dust,
	CFunc_LOD,
	CFuncAreaPortalWindow,
	CFuncConveyor,
	CFuncForceField,
	CFuncLadder,
	CFuncMonitor,
	CFuncOccluder,
	CFuncPasstimeGoal,
	CFuncReflectiveGlass,
	CFuncRespawnRoom,
	CFuncRespawnRoomVisualizer,
	CFuncRotating,
	CFuncSmokeVolume,
	CFuncTrackTrain,
	CGameRulesProxy,
	CHalloweenGiftPickup,
	CHalloweenPickup,
	CHalloweenSoulPack,
	CHandleTest,
	CHeadlessHatman,
	CHightower_TeleportVortex,
	CInfoLadderDismount,
	CInfoLightingRelative,
	CInfoOverlayAccessor,
	CLaserDot,
	CLightGlow,
	CMannVsMachineStats,
	CMaterialModifyControl,
	CMerasmus,
	CMerasmusDancer,
	CMonsterResource,
	CObjectCartDispenser,
	CObjectDispenser,
	CObjectSapper,
	CObjectSentrygun,
	CObjectTeleporter,
	CParticleFire,
	CParticlePerformanceMonitor,
	CParticleSystem,
	CPasstimeBall,
	CPasstimeGun,
	CPhysBox,
	CPhysBoxMultiplayer,
	CPhysicsProp,
	CPhysicsPropMultiplayer,
	CPhysMagnet,
	CPlasma,
	CPlayerDestructionDispenser,
	CPlayerResource,
	CPointCamera,
	CPointCommentaryNode,
	CPoseController,
	CPrecipitation,
	CPropVehicleDriveable,
	CRagdollManager,
	CRagdollProp,
	CRagdollPropAttached,
	CRobotDispenser,
	CRopeKeyframe,
	CSceneEntity,
	CShadowControl,
	CSlideshowDisplay,
	CSmokeStack,
	CSniperDot,
	CSpotlightEnd,
	CSprite,
	CSpriteOriented,
	CSpriteTrail,
	CSteamJet,
	CSun,
	CTeam,
	CTeamplayRoundBasedRulesProxy,
	CTeamRoundTimer,
	CTeamTrainWatcher,
	CTEArmorRicochet,
	CTEBaseBeam,
	CTEBeamEntPoint,
	CTEBeamEnts,
	CTEBeamFollow,
	CTEBeamLaser,
	CTEBeamPoints,
	CTEBeamRing,
	CTEBeamRingPoint,
	CTEBeamSpline,
	CTEBloodSprite,
	CTEBloodStream,
	CTEBreakModel,
	CTEBSPDecal,
	CTEBubbles,
	CTEBubbleTrail,
	CTEClientProjectile,
	CTEDecal,
	CTEDust,
	CTEDynamicLight,
	CTEEffectDispatch,
	CTEEnergySplash,
	CTEExplosion,
	CTEFireBullets,
	CTEFizz,
	CTEFootprintDecal,
	CTEGaussExplosion,
	CTEGlowSprite,
	CTEImpact,
	CTEKillPlayerAttachments,
	CTELargeFunnel,
	CTeleportVortex,
	CTEMetalSparks,
	CTEMuzzleFlash,
	CTEParticleSystem,
	CTEPhysicsProp,
	CTEPlayerAnimEvent,
	CTEPlayerDecal,
	CTEProjectedDecal,
	CTEShatterSurface,
	CTEShowLine,
	CTesla,
	CTESmoke,
	CTESparks,
	CTESprite,
	CTESpriteSpray,
	CTest_ProxyToggle_Networkable,
	CTestTraceline,
	CTETFBlood,
	CTETFExplosion,
	CTETFParticleEffect,
	CTEWorldDecal,
	CTFAmmoPack,
	CTFBall_Ornament,
	CTFBaseBoss,
	CTFBaseProjectile,
	CTFBaseRocket,
	CTFBat,
	CTFBat_Fish,
	CTFBat_Giftwrap,
	CTFBat_Wood,
	CTFBonesaw,
	CTFBotHintEngineerNest,
	CTFBottle,
	CTFBreakableMelee,
	CTFBreakableSign,
	CTFBuffItem,
	CTFCannon,
	CTFChargedSMG,
	CTFCleaver,
	CTFClub,
	CTFCompoundBow,
	CTFCrossbow,
	CTFDRGPomson,
	CTFDroppedWeapon,
	CTFFireAxe,
	CTFFists,
	CTFFlameManager,
	CTFFlameRocket,
	CTFFlameThrower,
	CTFFlareGun,
	CTFFlareGun_Revenge,
	CTFGameRulesProxy,
	CTFGasManager,
	CTFGenericBomb,
	CTFGlow,
	CTFGrapplingHook,
	CTFGrenadeLauncher,
	CTFGrenadePipebombProjectile,
	CTFHalloweenMinigame,
	CTFHalloweenMinigame_FallingPlatforms,
	CTFHellZap,
	CTFItem,
	CTFJar,
	CTFJarGas,
	CTFJarMilk,
	CTFKatana,
	CTFKnife,
	CTFLaserPointer,
	CTFLunchBox,
	CTFLunchBox_Drink,
	CTFMechanicalArm,
	CTFMedigunShield,
	CTFMiniGame,
	CTFMinigameLogic,
	CTFMinigun,
	CTFObjectiveResource,
	CTFParachute,
	CTFParachute_Primary,
	CTFParachute_Secondary,
	CTFParticleCannon,
	CTFPasstimeLogic,
	CTFPEPBrawlerBlaster,
	CTFPipebombLauncher,
	CTFPistol,
	CTFPistol_Scout,
	CTFPistol_ScoutPrimary,
	CTFPistol_ScoutSecondary,
	CTFPlayer,
	CTFPlayerDestructionLogic,
	CTFPlayerResource,
	CTFPointManager,
	CTFPowerupBottle,
	CTFProjectile_Arrow,
	CTFProjectile_BallOfFire,
	CTFProjectile_Cleaver,
	CTFProjectile_EnergyBall,
	CTFProjectile_EnergyRing,
	CTFProjectile_Flare,
	CTFProjectile_GrapplingHook,
	CTFProjectile_HealingBolt,
	CTFProjectile_Jar,
	CTFProjectile_JarGas,
	CTFProjectile_JarMilk,
	CTFProjectile_MechanicalArmOrb,
	CTFProjectile_Rocket,
	CTFProjectile_SentryRocket,
	CTFProjectile_SpellBats,
	CTFProjectile_SpellFireball,
	CTFProjectile_SpellKartBats,
	CTFProjectile_SpellKartOrb,
	CTFProjectile_SpellLightningOrb,
	CTFProjectile_SpellMeteorShower,
	CTFProjectile_SpellMirv,
	CTFProjectile_SpellPumpkin,
	CTFProjectile_SpellSpawnBoss,
	CTFProjectile_SpellSpawnHorde,
	CTFProjectile_SpellSpawnZombie,
	CTFProjectile_SpellTransposeTeleport,
	CTFProjectile_Throwable,
	CTFProjectile_ThrowableBreadMonster,
	CTFProjectile_ThrowableBrick,
	CTFProjectile_ThrowableRepel,
	CTFPumpkinBomb,
	CTFRagdoll,
	CTFRaygun,
	CTFReviveMarker,
	CTFRevolver,
	CTFRobotArm,
	CTFRobotDestruction_Robot,
	CTFRobotDestruction_RobotGroup,
	CTFRobotDestruction_RobotSpawn,
	CTFRobotDestructionLogic,
	CTFRocketLauncher,
	CTFRocketLauncher_AirStrike,
	CTFRocketLauncher_DirectHit,
	CTFRocketLauncher_Mortar,
	CTFRocketPack,
	CTFScatterGun,
	CTFShotgun,
	CTFShotgun_HWG,
	CTFShotgun_Pyro,
	CTFShotgun_Revenge,
	CTFShotgun_Soldier,
	CTFShotgunBuildingRescue,
	CTFShovel,
	CTFSlap,
	CTFSMG,
	CTFSniperRifle,
	CTFSniperRifleClassic,
	CTFSniperRifleDecap,
	CTFSodaPopper,
	CTFSpellBook,
	CTFStickBomb,
	CTFStunBall,
	CTFSword,
	CTFSyringeGun,
	CTFTankBoss,
	CTFTauntProp,
	CTFTeam,
	CTFThrowable,
	CTFViewModel,
	CTFWeaponBase,
	CTFWeaponBaseGrenadeProj,
	CTFWeaponBaseGun,
	CTFWeaponBaseMelee,
	CTFWeaponBaseMerasmusGrenade,
	CTFWeaponBuilder,
	CTFWeaponFlameBall,
	CTFWeaponInvis,
	CTFWeaponPDA,
	CTFWeaponPDA_Engineer_Build,
	CTFWeaponPDA_Engineer_Destroy,
	CTFWeaponPDA_Spy,
	CTFWeaponPDAExpansion_Dispenser,
	CTFWeaponPDAExpansion_Teleporter,
	CTFWeaponSapper,
	CTFWearable,
	CTFWearableCampaignItem,
	CTFWearableDemoShield,
	CTFWearableItem,
	CTFWearableLevelableItem,
	CTFWearableRazorback,
	CTFWearableRobotArm,
	CTFWearableVM,
	CTFWrench,
	CVGuiScreen,
	CVoteController,
	CWaterBullet,
	CWaterLODControl,
	CWeaponIFMBase,
	CWeaponIFMBaseCamera,
	CWeaponIFMSteadyCam,
	CWeaponMedigun,
	CWorld,
	CZombie,
	DustTrail,
	MovieExplosion,
	NextBotCombatCharacter,
	ParticleSmokeGrenade,
	RocketTrail,
	SmokeTrail,
	SporeExplosion,
	SporeTrail,

	Count,
	Reserved_HealthPack
};
