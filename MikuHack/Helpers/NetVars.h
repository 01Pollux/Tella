#pragma once


#include <unordered_map>
#include <string>
#include <stdexcept>

class RecvProp;
class RecvTable;
class CRecvProxyData;

using ArrayLengthRecvProxyFn	= void	(*)	(void*, int, int);
using RecvVarProxyFn			= void	(*)	(const CRecvProxyData* pData, void* pStruct, void* pOut);
using DataTableRecvVarProxyFn	= void	(*)	(const RecvProp* pProp, void** pOut, void* pData, int objectID);

enum class SendPropType
{
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_VectorXY,
	DPT_String,
	DPT_Array,
	DPT_DataTable,

	DPT_NUMSendPropTypes
};

class DVariant
{
public:
	union
	{
		float			m_Float;
		int				m_Int;
		const char*		m_pString;
		void*			m_pData;
		float			m_Vector[3];
	};
	SendPropType		m_Type;
};

class CRecvProxyData
{
public:
	const RecvProp* m_pRecvProp;
	DVariant		m_Value;
	int				m_iElement;
	int				m_ObjectID;
};

class RecvProp
{
public:
	const char*				m_pVarName;
	SendPropType			m_RecvType;
	int						m_Flags;
	int						m_StringBufferSize;

	bool 					m_bInsideArray;	

	const void*				m_pExtraData;

	RecvProp*				m_pArrayProp;
	ArrayLengthRecvProxyFn	m_ArrayLengthProxy;

	RecvVarProxyFn			m_ProxyFn;
	DataTableRecvVarProxyFn	m_DataTableProxyFn;

	RecvTable*				m_pDataTable;
	int						m_Offset;

	int						m_ElementStride;
	int						m_nElements;

	const char*				m_pParentArrayPropName;
};

class RecvTable
{
public:

	RecvProp*		m_pProps;
	int				m_nProps;
	void*			m_pDecoder;
	const char*			m_pNetTableName;

	bool 			m_bInitialized;
	bool 			m_bInMainList;
};

class ClientClass
{
public:
	char			m_Unk[8];
	const char*		m_pNetworkName;
	RecvTable*		m_pRecvTable;
	ClientClass*	m_pNext;
	int				m_ClassID;
};


struct recvprop_info_t
{
	RecvProp*	pProp{ };
	uint32_t	offset{ };
	bool		valid{ false };
};


bool LookupRecvPropC(ClientClass* pClass, const char* offset, recvprop_info_t* info);


class NetVarHook
{
	RecvVarProxyFn pActual;
	RecvProp* pProp;

	void Init(RecvProp* pProp, RecvVarProxyFn pCallback)
	{
		this->pActual = pProp->m_ProxyFn;
		this->pProp = pProp;
		pProp->m_ProxyFn = *pCallback;
	}

public:
	explicit NetVarHook(const char* class_name, const char* prop_name, RecvVarProxyFn callback);
	explicit NetVarHook(RecvProp* prop, RecvVarProxyFn callback) noexcept
	{
		this->Init(prop, callback);
	}

	void Execute(const CRecvProxyData* pData, void* pStruct, void* pOut) const noexcept
	{
		(this->pActual)(pData, pStruct, pOut);
	}

	~NetVarHook()
	{
		this->pProp->m_ProxyFn = pActual;
	}

public:
	NetVarHook(const NetVarHook&)			= delete;
	NetVarHook& operator=(const NetVarHook&)= delete;
	NetVarHook(NetVarHook&&)				= delete;
	NetVarHook& operator=(NetVarHook&&)		= delete;
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


enum ClassID
{
	ClassID_CAI_BaseNPC,
	ClassID_CBaseAnimating,
	ClassID_CBaseAnimatingOverlay,
	ClassID_CBaseAttributableItem,
	ClassID_CBaseCombatCharacter,
	ClassID_CBaseCombatWeapon,
	ClassID_CBaseDoor,
	ClassID_CBaseEntity,
	ClassID_CBaseFlex,
	ClassID_CBaseGrenade,
	ClassID_CBaseObject,
	ClassID_CBaseObjectUpgrade,
	ClassID_CBaseParticleEntity,
	ClassID_CBasePlayer,
	ClassID_CBaseProjectile,
	ClassID_CBasePropDoor,
	ClassID_CBaseTeamObjectiveResource,
	ClassID_CBaseTempEntity,
	ClassID_CBaseViewModel,
	ClassID_CBeam,
	ClassID_CBoneFollower,
	ClassID_CBonusDuckPickup,
	ClassID_CBonusPack,
	ClassID_CBonusRoundLogic,
	ClassID_CBreakableProp,
	ClassID_CBreakableSurface,
	ClassID_CCaptureFlag,
	ClassID_CCaptureFlagReturnIcon,
	ClassID_CCaptureZone,
	ClassID_CColorCorrection,
	ClassID_CColorCorrectionVolume,
	ClassID_CCurrencyPack,
	ClassID_CDynamicLight,
	ClassID_CDynamicProp,
	ClassID_CEconEntity,
	ClassID_CEconWearable,
	ClassID_CEmbers,
	ClassID_CEntityDissolve,
	ClassID_CEntityFlame,
	ClassID_CEntityParticleTrail,
	ClassID_CEnvDetailController,
	ClassID_CEnvParticleScript,
	ClassID_CEnvProjectedTexture,
	ClassID_CEnvQuadraticBeam,
	ClassID_CEnvScreenEffect,
	ClassID_CEnvScreenOverlay,
	ClassID_CEnvTonemapController,
	ClassID_CEnvWind,
	ClassID_CEyeballBoss,
	ClassID_CFireSmoke,
	ClassID_CFireTrail,
	ClassID_CFish,
	ClassID_CFogController,
	ClassID_CFunc_Dust,
	ClassID_CFunc_LOD,
	ClassID_CFuncAreaPortalWindow,
	ClassID_CFuncConveyor,
	ClassID_CFuncForceField,
	ClassID_CFuncLadder,
	ClassID_CFuncMonitor,
	ClassID_CFuncOccluder,
	ClassID_CFuncPasstimeGoal,
	ClassID_CFuncReflectiveGlass,
	ClassID_CFuncRespawnRoom,
	ClassID_CFuncRespawnRoomVisualizer,
	ClassID_CFuncRotating,
	ClassID_CFuncSmokeVolume,
	ClassID_CFuncTrackTrain,
	ClassID_CGameRulesProxy,
	ClassID_CHalloweenGiftPickup,
	ClassID_CHalloweenPickup,
	ClassID_CHalloweenSoulPack,
	ClassID_CHandleTest,
	ClassID_CHeadlessHatman,
	ClassID_CHightower_TeleportVortex,
	ClassID_CInfoLadderDismount,
	ClassID_CInfoLightingRelative,
	ClassID_CInfoOverlayAccessor,
	ClassID_CLaserDot,
	ClassID_CLightGlow,
	ClassID_CMannVsMachineStats,
	ClassID_CMaterialModifyControl,
	ClassID_CMerasmus,
	ClassID_CMerasmusDancer,
	ClassID_CMonsterResource,
	ClassID_CObjectCartDispenser,
	ClassID_CObjectDispenser,
	ClassID_CObjectSapper,
	ClassID_CObjectSentrygun,
	ClassID_CObjectTeleporter,
	ClassID_CParticleFire,
	ClassID_CParticlePerformanceMonitor,
	ClassID_CParticleSystem,
	ClassID_CPasstimeBall,
	ClassID_CPasstimeGun,
	ClassID_CPhysBox,
	ClassID_CPhysBoxMultiplayer,
	ClassID_CPhysicsProp,
	ClassID_CPhysicsPropMultiplayer,
	ClassID_CPhysMagnet,
	ClassID_CPlasma,
	ClassID_CPlayerDestructionDispenser,
	ClassID_CPlayerResource,
	ClassID_CPointCamera,
	ClassID_CPointCommentaryNode,
	ClassID_CPoseController,
	ClassID_CPrecipitation,
	ClassID_CPropVehicleDriveable,
	ClassID_CRagdollManager,
	ClassID_CRagdollProp,
	ClassID_CRagdollPropAttached,
	ClassID_CRobotDispenser,
	ClassID_CRopeKeyframe,
	ClassID_CSceneEntity,
	ClassID_CShadowControl,
	ClassID_CSlideshowDisplay,
	ClassID_CSmokeStack,
	ClassID_CSniperDot,
	ClassID_CSpotlightEnd,
	ClassID_CSprite,
	ClassID_CSpriteOriented,
	ClassID_CSpriteTrail,
	ClassID_CSteamJet,
	ClassID_CSun,
	ClassID_CTeam,
	ClassID_CTeamplayRoundBasedRulesProxy,
	ClassID_CTeamRoundTimer,
	ClassID_CTeamTrainWatcher,
	ClassID_CTEArmorRicochet,
	ClassID_CTEBaseBeam,
	ClassID_CTEBeamEntPoint,
	ClassID_CTEBeamEnts,
	ClassID_CTEBeamFollow,
	ClassID_CTEBeamLaser,
	ClassID_CTEBeamPoints,
	ClassID_CTEBeamRing,
	ClassID_CTEBeamRingPoint,
	ClassID_CTEBeamSpline,
	ClassID_CTEBloodSprite,
	ClassID_CTEBloodStream,
	ClassID_CTEBreakModel,
	ClassID_CTEBSPDecal,
	ClassID_CTEBubbles,
	ClassID_CTEBubbleTrail,
	ClassID_CTEClientProjectile,
	ClassID_CTEDecal,
	ClassID_CTEDust,
	ClassID_CTEDynamicLight,
	ClassID_CTEEffectDispatch,
	ClassID_CTEEnergySplash,
	ClassID_CTEExplosion,
	ClassID_CTEFireBullets,
	ClassID_CTEFizz,
	ClassID_CTEFootprintDecal,
	ClassID_CTEGaussExplosion,
	ClassID_CTEGlowSprite,
	ClassID_CTEImpact,
	ClassID_CTEKillPlayerAttachments,
	ClassID_CTELargeFunnel,
	ClassID_CTeleportVortex,
	ClassID_CTEMetalSparks,
	ClassID_CTEMuzzleFlash,
	ClassID_CTEParticleSystem,
	ClassID_CTEPhysicsProp,
	ClassID_CTEPlayerAnimEvent,
	ClassID_CTEPlayerDecal,
	ClassID_CTEProjectedDecal,
	ClassID_CTEShatterSurface,
	ClassID_CTEShowLine,
	ClassID_CTesla,
	ClassID_CTESmoke,
	ClassID_CTESparks,
	ClassID_CTESprite,
	ClassID_CTESpriteSpray,
	ClassID_CTest_ProxyToggle_Networkable,
	ClassID_CTestTraceline,
	ClassID_CTETFBlood,
	ClassID_CTETFExplosion,
	ClassID_CTETFParticleEffect,
	ClassID_CTEWorldDecal,
	ClassID_CTFAmmoPack,
	ClassID_CTFBall_Ornament,
	ClassID_CTFBaseBoss,
	ClassID_CTFBaseProjectile,
	ClassID_CTFBaseRocket,
	ClassID_CTFBat,
	ClassID_CTFBat_Fish,
	ClassID_CTFBat_Giftwrap,
	ClassID_CTFBat_Wood,
	ClassID_CTFBonesaw,
	ClassID_CTFBotHintEngineerNest,
	ClassID_CTFBottle,
	ClassID_CTFBreakableMelee,
	ClassID_CTFBreakableSign,
	ClassID_CTFBuffItem,
	ClassID_CTFCannon,
	ClassID_CTFChargedSMG,
	ClassID_CTFCleaver,
	ClassID_CTFClub,
	ClassID_CTFCompoundBow,
	ClassID_CTFCrossbow,
	ClassID_CTFDRGPomson,
	ClassID_CTFDroppedWeapon,
	ClassID_CTFFireAxe,
	ClassID_CTFFists,
	ClassID_CTFFlameManager,
	ClassID_CTFFlameRocket,
	ClassID_CTFFlameThrower,
	ClassID_CTFFlareGun,
	ClassID_CTFFlareGun_Revenge,
	ClassID_CTFGameRulesProxy,
	ClassID_CTFGasManager,
	ClassID_CTFGenericBomb,
	ClassID_CTFGlow,
	ClassID_CTFGrapplingHook,
	ClassID_CTFGrenadeLauncher,
	ClassID_CTFGrenadePipebombProjectile,
	ClassID_CTFHalloweenMinigame,
	ClassID_CTFHalloweenMinigame_FallingPlatforms,
	ClassID_CTFHellZap,
	ClassID_CTFItem,
	ClassID_CTFJar,
	ClassID_CTFJarGas,
	ClassID_CTFJarMilk,
	ClassID_CTFKatana,
	ClassID_CTFKnife,
	ClassID_CTFLaserPointer,
	ClassID_CTFLunchBox,
	ClassID_CTFLunchBox_Drink,
	ClassID_CTFMechanicalArm,
	ClassID_CTFMedigunShield,
	ClassID_CTFMiniGame,
	ClassID_CTFMinigameLogic,
	ClassID_CTFMinigun,
	ClassID_CTFObjectiveResource,
	ClassID_CTFParachute,
	ClassID_CTFParachute_Primary,
	ClassID_CTFParachute_Secondary,
	ClassID_CTFParticleCannon,
	ClassID_CTFPasstimeLogic,
	ClassID_CTFPEPBrawlerBlaster,
	ClassID_CTFPipebombLauncher,
	ClassID_CTFPistol,
	ClassID_CTFPistol_Scout,
	ClassID_CTFPistol_ScoutPrimary,
	ClassID_CTFPistol_ScoutSecondary,
	ClassID_CTFPlayer,
	ClassID_CTFPlayerDestructionLogic,
	ClassID_CTFPlayerResource,
	ClassID_CTFPointManager,
	ClassID_CTFPowerupBottle,
	ClassID_CTFProjectile_Arrow,
	ClassID_CTFProjectile_BallOfFire,
	ClassID_CTFProjectile_Cleaver,
	ClassID_CTFProjectile_EnergyBall,
	ClassID_CTFProjectile_EnergyRing,
	ClassID_CTFProjectile_Flare,
	ClassID_CTFProjectile_GrapplingHook,
	ClassID_CTFProjectile_HealingBolt,
	ClassID_CTFProjectile_Jar,
	ClassID_CTFProjectile_JarGas,
	ClassID_CTFProjectile_JarMilk,
	ClassID_CTFProjectile_MechanicalArmOrb,
	ClassID_CTFProjectile_Rocket,
	ClassID_CTFProjectile_SentryRocket,
	ClassID_CTFProjectile_SpellBats,
	ClassID_CTFProjectile_SpellFireball,
	ClassID_CTFProjectile_SpellKartBats,
	ClassID_CTFProjectile_SpellKartOrb,
	ClassID_CTFProjectile_SpellLightningOrb,
	ClassID_CTFProjectile_SpellMeteorShower,
	ClassID_CTFProjectile_SpellMirv,
	ClassID_CTFProjectile_SpellPumpkin,
	ClassID_CTFProjectile_SpellSpawnBoss,
	ClassID_CTFProjectile_SpellSpawnHorde,
	ClassID_CTFProjectile_SpellSpawnZombie,
	ClassID_CTFProjectile_SpellTransposeTeleport,
	ClassID_CTFProjectile_Throwable,
	ClassID_CTFProjectile_ThrowableBreadMonster,
	ClassID_CTFProjectile_ThrowableBrick,
	ClassID_CTFProjectile_ThrowableRepel,
	ClassID_CTFPumpkinBomb,
	ClassID_CTFRagdoll,
	ClassID_CTFRaygun,
	ClassID_CTFReviveMarker,
	ClassID_CTFRevolver,
	ClassID_CTFRobotArm,
	ClassID_CTFRobotDestruction_Robot,
	ClassID_CTFRobotDestruction_RobotGroup,
	ClassID_CTFRobotDestruction_RobotSpawn,
	ClassID_CTFRobotDestructionLogic,
	ClassID_CTFRocketLauncher,
	ClassID_CTFRocketLauncher_AirStrike,
	ClassID_CTFRocketLauncher_DirectHit,
	ClassID_CTFRocketLauncher_Mortar,
	ClassID_CTFRocketPack,
	ClassID_CTFScatterGun,
	ClassID_CTFShotgun,
	ClassID_CTFShotgun_HWG,
	ClassID_CTFShotgun_Pyro,
	ClassID_CTFShotgun_Revenge,
	ClassID_CTFShotgun_Soldier,
	ClassID_CTFShotgunBuildingRescue,
	ClassID_CTFShovel,
	ClassID_CTFSlap,
	ClassID_CTFSMG,
	ClassID_CTFSniperRifle,
	ClassID_CTFSniperRifleClassic,
	ClassID_CTFSniperRifleDecap,
	ClassID_CTFSodaPopper,
	ClassID_CTFSpellBook,
	ClassID_CTFStickBomb,
	ClassID_CTFStunBall,
	ClassID_CTFSword,
	ClassID_CTFSyringeGun,
	ClassID_CTFTankBoss,
	ClassID_CTFTauntProp,
	ClassID_CTFTeam,
	ClassID_CTFThrowable,
	ClassID_CTFViewModel,
	ClassID_CTFWeaponBase,
	ClassID_CTFWeaponBaseGrenadeProj,
	ClassID_CTFWeaponBaseGun,
	ClassID_CTFWeaponBaseMelee,
	ClassID_CTFWeaponBaseMerasmusGrenade,
	ClassID_CTFWeaponBuilder,
	ClassID_CTFWeaponFlameBall,
	ClassID_CTFWeaponInvis,
	ClassID_CTFWeaponPDA,
	ClassID_CTFWeaponPDA_Engineer_Build,
	ClassID_CTFWeaponPDA_Engineer_Destroy,
	ClassID_CTFWeaponPDA_Spy,
	ClassID_CTFWeaponPDAExpansion_Dispenser,
	ClassID_CTFWeaponPDAExpansion_Teleporter,
	ClassID_CTFWeaponSapper,
	ClassID_CTFWearable,
	ClassID_CTFWearableCampaignItem,
	ClassID_CTFWearableDemoShield,
	ClassID_CTFWearableItem,
	ClassID_CTFWearableLevelableItem,
	ClassID_CTFWearableRazorback,
	ClassID_CTFWearableRobotArm,
	ClassID_CTFWearableVM,
	ClassID_CTFWrench,
	ClassID_CVGuiScreen,
	ClassID_CVoteController,
	ClassID_CWaterBullet,
	ClassID_CWaterLODControl,
	ClassID_CWeaponIFMBase,
	ClassID_CWeaponIFMBaseCamera,
	ClassID_CWeaponIFMSteadyCam,
	ClassID_CWeaponMedigun,
	ClassID_CWorld,
	ClassID_CZombie,
	ClassID_DustTrail,
	ClassID_MovieExplosion,
	ClassID_NextBotCombatCharacter,
	ClassID_ParticleSmokeGrenade,
	ClassID_RocketTrail,
	ClassID_SmokeTrail,
	ClassID_SporeExplosion,
	ClassID_SporeTrail,
};
