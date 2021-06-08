#pragma once

#include "MathLib/Vector3D.hpp"

class IVModelInfo;
namespace Interfaces
{
	extern IVModelInfo* MdlInfo;
	constexpr const char* MdlInfoName = "VModelInfoClient006";
}

struct ModelInfo;
struct studiohdr_t;
struct vcollide_t;
struct virtualmdl_t;
struct cplane_t;
struct mleaf_t;
struct mleafwaterdata_t;
struct mvertex_t;

class IMaterial;
class UtlBuffer;
class IClientRenderable;
class IPhysCollide;
class EngTrace;
class IModelLoadCallback;

class IVModelInfo_Internal
{
public:
	virtual							~IVModelInfo_Internal() = default;

	// Returns ModelInfo* pointer for a model given a precached or dynamic model index.
	virtual const ModelInfo* GetModel(int modelindex) = 0;

	// Returns index of model by name for precached or known dynamic models.
	// Does not adjust reference count for dynamic models.
	virtual int						GetModelIndex(const char* name) const = 0;

	// Returns name of model
	virtual const char* GetModelName(const ModelInfo* model) const = 0;
	virtual vcollide_t* GetVCollide(const ModelInfo* model) = 0;
	virtual vcollide_t* GetVCollide(int modelindex) = 0;
	virtual void					GetModelBounds(const ModelInfo* model, Vector& mins, Vector& maxs) const = 0;
	virtual	void					GetModelRenderBounds(const ModelInfo* model, Vector& mins, Vector& maxs) const = 0;
	virtual int						GetModelFrameCount(const ModelInfo* model) const = 0;
	virtual int						GetModelType(const ModelInfo* model) const = 0;
	virtual void* GetModelExtraData(const ModelInfo* model) = 0;
	virtual bool					ModelHasMaterialProxy(const ModelInfo* model) const = 0;
	virtual bool					IsTranslucent(ModelInfo const* model) const = 0;
	virtual bool					IsTranslucentTwoPass(const ModelInfo* model) const = 0;
	virtual void					RecomputeTranslucency(const ModelInfo* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate = 1.0f) = 0;
	virtual int						GetModelMaterialCount(const ModelInfo* model) const = 0;
	virtual void					GetModelMaterials(const ModelInfo* model, int count, IMaterial** ppMaterial) = 0;
	virtual bool					IsModelVertexLit(const ModelInfo* model) const = 0;
	virtual const char* GetModelKeyValueText(const ModelInfo* model) = 0;
	virtual bool					GetModelKeyValue(const ModelInfo* model, UtlBuffer& buf) = 0; // supports keyvalue blocks in submodels
	virtual float					GetModelRadius(const ModelInfo* model) = 0;

	virtual const studiohdr_t* FindModel(const studiohdr_t* pStudioHdr, void** cache, const char* modelname) const = 0;
	virtual const studiohdr_t* FindModel(void* cache) const = 0;
	virtual	virtualmdl_t* GetVirtualModel(const studiohdr_t* pStudioHdr) const = 0;
	virtual unsigned char* GetAnimBlock(const studiohdr_t* pStudioHdr, int iBlock) const = 0;

	// Available on client only!!!
	virtual void					GetModelMaterialColorAndLighting(const ModelInfo* model, Vector const& origin,
		QAngle const& angles, EngTrace* pTrace,
		Vector& lighting, Vector& matColor) = 0;
	virtual void					GetIlluminationPoint(const ModelInfo* model, IClientRenderable* pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter) = 0;

	virtual int						GetModelContents(int modelIndex) = 0;
	virtual studiohdr_t* GetStudiomodel(const ModelInfo* mod) = 0;
	virtual int						GetModelSpriteWidth(const ModelInfo* model) const = 0;
	virtual int						GetModelSpriteHeight(const ModelInfo* model) const = 0;

	// Sets/gets a map-specified fade range (client only)
	virtual void					SetLevelScreenFadeRange(float flMinSize, float flMaxSize) = 0;
	virtual void					GetLevelScreenFadeRange(float* pMinArea, float* pMaxArea) const = 0;

	// Sets/gets a map-specified per-view fade range (client only)
	virtual void					SetViewScreenFadeRange(float flMinSize, float flMaxSize) = 0;

	// Computes fade alpha based on distance fade + screen fade (client only)
	virtual unsigned char			ComputeLevelScreenFade(const Vector& vecAbsOrigin, float flRadius, float flFadeScale) const = 0;
	virtual unsigned char			ComputeViewScreenFade(const Vector& vecAbsOrigin, float flRadius, float flFadeScale) const = 0;

	// both client and server
	virtual int						GetAutoplayList(const studiohdr_t* pStudioHdr, unsigned short** pAutoplayList) const = 0;

	// Gets a virtual terrain collision model (creates if necessary)
	// NOTE: This may return NULL if the terrain model cannot be virtualized
	virtual IPhysCollide* GetCollideForVirtualTerrain(int index) = 0;

	virtual bool					IsUsingFBTexture(const ModelInfo* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const = 0;

	// Obsolete methods. These are left in to maintain binary compatibility with clients using the IVModelInfo old version.
	virtual const ModelInfo* FindOrLoadModel(const char* name) { return NULL; }
	virtual void					InitDynamicModels() {  }
	virtual void					ShutdownDynamicModels() { ; }
	virtual void					AddDynamicModel(const char* name, int nModelIndex = -1) {  }
	virtual void					ReferenceModel(int modelindex) { }
	virtual void					UnreferenceModel(int modelindex) {  }
	virtual void					CleanupDynamicModels(bool bForce = false) {  }

	virtual unsigned short			GetCacheHandle(const ModelInfo* model) const = 0;

	// Returns planes of non-nodraw brush model surfaces
	virtual int						GetBrushModelPlaneCount(const ModelInfo* model) const = 0;
	virtual void					GetBrushModelPlane(const ModelInfo* model, int nIndex, cplane_t& plane, Vector* pOrigin) const = 0;
	virtual int						GetSurfacepropsForVirtualTerrain(int index) = 0;

	// Poked by engine host system
	virtual void					OnLevelChange() = 0;

	virtual int						GetModelClientSideIndex(const char* name) const = 0;

	// Returns index of model by name, dynamically registered if not already known.
	virtual int						RegisterDynamicModel(const char* name, bool bClientSide) = 0;

	virtual bool					IsDynamicModelLoading(int modelIndex) = 0;

	virtual void					AddRefDynamicModel(int modelIndex) = 0;
	virtual void					ReleaseDynamicModel(int modelIndex) = 0;

	// Registers callback for when dynamic model has finished loading.
	// Automatically adds reference, pair with ReleaseDynamicModel.
	virtual bool					RegisterModelLoadCallback(int modelindex, IModelLoadCallback* pCallback, bool bCallImmediatelyIfLoaded = true) = 0;
	virtual void					UnregisterModelLoadCallback(int modelindex, IModelLoadCallback* pCallback) = 0;
};

class IVModelInfo : public IVModelInfo_Internal
{
	virtual void OnDynamicModelsStringTableChange(int nStringIndex, const char* pString, const void* pData) = 0;

	// For tools only!
	virtual const ModelInfo* FindOrLoadModel(const char* name) = 0;
};


#define MODELFLAG_MATERIALPROXY					0x0001	// we've got a material proxy
#define MODELFLAG_TRANSLUCENT					0x0002	// we've got a translucent model
#define MODELFLAG_VERTEXLIT						0x0004	// we've got a vertex-lit model
#define MODELFLAG_TRANSLUCENT_TWOPASS			0x0008	// render opaque part in opaque pass
#define MODELFLAG_FRAMEBUFFER_TEXTURE			0x0010	// we need the framebuffer as a texture
#define MODELFLAG_HAS_DLIGHT					0x0020	// need to check dlights
#define MODELFLAG_STUDIOHDR_USES_FB_TEXTURE		0x0100	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_USES_BUMPMAPPING	0x0200	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_USES_ENV_CUBEMAP	0x0400	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_AMBIENT_BOOST		0x0800	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_DO_NOT_CAST_SHADOWS	0x1000	// persisted from studiohdr


struct ModelInfo
{
	void*				FileNameHndl;
	const char*			ModelName;

	int					LoadFlags;
	int					ServerCount;
	IMaterial**			pMarterials;

	int					ModelType;
	int					ModelFlags;

	Vector				Mins, Maxs;
	float				Radius;

	union
	{
		uint8_t			_PadBrush[16];
		uint16_t		StudioHndl;
		uint8_t			_PadSprite[16];
	};
};