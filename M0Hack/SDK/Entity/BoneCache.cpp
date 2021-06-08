
#include "BoneCache.hpp"
#include "../ModelInfo.hpp"

#include <cstdlib>

const Matrix3x4* IBoneCache::GetCachedBone(int studioIndex) const
{
	int cachedIndex = StudioToCached()[studioIndex];
	if (cachedIndex >= 0)
	{
		return BoneArray() + cachedIndex;
	}
	return NULL;
}

void IBoneCache::ReadCachedBones(std::vector<Matrix3x4>* bones) const
{
	const Matrix3x4* pBones = BoneArray();
	const short* cachedstudio = CachedToStudio();
	bones->resize(CachedBoneCount);
	Matrix3x4* data = bones->data();

	for (int i = 0; i < CachedBoneCount; i++)
		VectorHelper::MatrixCopy(pBones[i], data[cachedstudio[i]]);
}

void IBoneCache::ReadCachedBonePointers(std::vector<const Matrix3x4*>* bones, size_t numbones) const
{
	const Matrix3x4* pBones = BoneArray();
	const short* pCachedToStudio = CachedToStudio();

	bones->resize(numbones);
	const Matrix3x4** data = bones->data();

	for (int i = 0; i < CachedBoneCount; i++)
		data[pCachedToStudio[i]] = pBones + i;
}


namespace IStudioHelper
{
	virtualmdl_t* GetVirtualModel(const studiohdr_t* shdr)
	{
		return shdr->NumIncludeModels ? Interfaces::MdlInfo->GetVirtualModel(shdr) : nullptr;
	}

	int FindBone(const studiohdr_t* shdr, const char* name)
	{
		if (shdr)
		{
			int start = 0, end = shdr->NumBones - 1;
			const unsigned char* bone_table = shdr->GetBoneTableSortedByName();
			const mstudiobone_t* sb = shdr->GetBone(0);
			while (start <= end)
			{
				const int mid = (start + end) >> 1;
				const int cmp = _strcmpi(sb[bone_table[mid]].GetName(), name);

				if (cmp < 0)
				{
					start = mid + 1;
				}
				else if (cmp > 0)
				{
					end = mid - 1;
				}
				else
				{
					return bone_table[mid];
				}
			}
		}

		return -1;
	}

}

BoneResult IBoneInfo::GetBonePosition(const int hitbox) const
{
	const mstudiobbox_t* sbb = hitbox_set->GetHitbox(hitbox);
	const Matrix3x4* bone = bonecache->GetCachedBone(sbb->Bone);
	
	Vector pos;
	VectorHelper::MatrixPosition(*bone, pos);
	return { pos, sbb->Min, sbb->Max };
}

void IBoneInfo::GetBonePosition(const int* bone, size_t size, std::vector<BoneResult>* positions) const
{
	positions->reserve(size);

	for (size_t i = 0; i < size; i++)
		positions->emplace_back(GetBonePosition(bone[i]));
}

void IBoneInfo::GetBonePosition(std::vector<BoneResult>* positions) const
{
	const size_t num_of_bones = hitbox_set->NumHitboxes;
	positions->reserve(num_of_bones);

	for (size_t i = 0; i < num_of_bones; i++)
		positions->emplace_back(GetBonePosition(i));
}
