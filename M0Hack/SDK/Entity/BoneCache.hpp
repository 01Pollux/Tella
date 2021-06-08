#pragma once

#include <array>
#include <vector>

#include "../MathLib/Vector3D.hpp"
#include "../MathLib/Matrix3x4.hpp"
#include "Studio.hpp"


class IBoneCache
{
public:
	const Matrix3x4*	GetCachedBone(int studioIndex) const;

	void				ReadCachedBones(std::vector<Matrix3x4>* bones) const;
	void				ReadCachedBonePointers(std::vector<const Matrix3x4*>* bones, size_t numbones) const;

	bool				IsValid(float curtime, float dt = 0.1f) const noexcept
	{
		return curtime - TimeValid <= dt;
	}

	float			TimeValid;
	int				BoneMask;

	unsigned int	Size;
	unsigned short	CachedBoneCount;
	unsigned short	MatrixOffset;
	unsigned short	CachedToStudioOffset;
	unsigned short	BoneOutOffset;

private:
	const Matrix3x4* BoneArray() const
	{
		return reinterpret_cast<const Matrix3x4*>(reinterpret_cast<const char*>(this + 1) + MatrixOffset);
	}
	const short* StudioToCached() const
	{
		return reinterpret_cast<const short*>(this + 1);
	}
	const short* CachedToStudio() const
	{
		return reinterpret_cast<const short*>(this + 1) + CachedToStudioOffset;
	}
};

struct BoneResult
{
	Vector Position;
	Vector Min;
	Vector Max;
};

class IBoneInfo 
{
public:
	IBoneInfo() = default;

	IBoneInfo(const IBoneCache* bonecache, const mstudiohitboxset_t* hitbox_set, const studiohdr_t* studio_hdr) :
						bonecache(bonecache), hitbox_set(hitbox_set), studio_hdr(studio_hdr) { }

	operator bool() const noexcept
	{
		return bonecache != nullptr;
	}

	BoneResult GetBonePosition(int bone) const;

	void GetBonePosition(const int* bone, size_t size, std::vector<BoneResult>* positions) const;

	void GetBonePosition(std::vector<BoneResult>* positions) const;

	const IBoneCache* GetBoneCache() const noexcept
	{
		return bonecache;
	}

	const mstudiohitboxset_t* GetStudioHitbox() const noexcept
	{
		return hitbox_set;
	}

	const studiohdr_t* GetStudioHDR() const noexcept
	{
		return studio_hdr;
	}

private:
	const IBoneCache* bonecache{ };

	const mstudiohitboxset_t* hitbox_set;
	const studiohdr_t* studio_hdr;
};



namespace IStudioHelper
{
	virtualmdl_t* GetVirtualModel(const studiohdr_t* shdr);
	int FindBone(const studiohdr_t* shdr, const char* name);
}