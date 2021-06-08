#include "aimbot.hpp"

#include "Trace.hpp"
#include "UserCmd.hpp"
#include "GlobalVars.hpp"

constexpr double bad_fov = 999.9 * 999.9;

[[nodiscard]] bool GetBestHitbox(
	const IBaseEntity pEnt,
	const QAngle& angle,
	Vector& out_pos,
	double& out_fov
)
{
	const IBoneInfo info = pEnt->QueryBoneInfo();
	if (!info)
		return false;

	PROFILE_USECTION("Aimbot::GetBestHitbox", M0PROFILER_GROUP::CHEAT_PROFILE);

	double best_fov = bad_fov;

	ILocalPlayer pMe;
	const Vector start = pMe->EyePosition();
	Vector best_pos;

	{
		GameTrace tr;
		Trace::FilterIgnoreAllExceptOne fitler_ignore(pEnt);

		std::vector<BoneResult> positions;
		info.GetBonePosition(&positions);

		for (const BoneResult& bresult : positions)
		{
			Trace::TraceHull(start, bresult.Position, bresult.Min, bresult.Max, MASK_SHOT_HULL, &tr, &fitler_ignore);

			if (!tr.DidHit(pEnt.get()))
				continue;

			const double cur = tr.EndPos.DistToSqr(start);
			if (cur < best_fov)
			{
				best_fov = cur;
				best_pos = tr.EndPos;
			}
		}
	}

	out_pos = best_pos;
	out_fov = best_fov;

	return best_fov != bad_fov;
}
	
[[nodiscard]] static std::optional<Vector>
Trace_GenericHull(const ITFPlayer pMe, const ITFPlayer pPlayer)
{
	BoneResult br;
	if (pPlayer->GetBonePosition(PlayerHitboxIdx::Head, &br))
	{
		GameTrace res;
		Trace::FilterSimple filter(pMe);
		Trace::TraceHull(pMe->EyePosition(), br.Position, br.Min, br.Max, MASK_SHOT_HULL, &res, &filter);

		if (res.DidHit(pPlayer.get()))
			return res.EndPos;
	}

	return std::nullopt;
}

std::optional<Vector>
IAimbotHack::Filter_IsEntityVisible(const IBaseEntity pEnt, bool is_player, const QAngle& angle)
{
	PROFILE_USECTION("IAimbotHack::Filter_IsEntityVisible", M0PROFILER_GROUP::CHEAT_PROFILE);

	constexpr double min_threshhold = 8.9;
	ILocalPlayer pMe;
	
	switch (Vars.HitboxPriority)
	{
	case HitboxPriority::Head:
	{
		return is_player ? Trace_GenericHull(pMe, ITFPlayer(pEnt)) : std::nullopt;
	}

	case HitboxPriority::Nearest:
	{
		Vector res; double dummy;
		if (GetBestHitbox(pEnt, angle, res, dummy))
			return res;
		else return std::nullopt;
	}

	case HitboxPriority::Smart:
	{
		Vector cur_pos;
		double cur_fov = bad_fov;
		Vars.Smooth.reset();
		
		if (!Vars.Smooth.Override_.has_elapsed())
				return std::nullopt;

		if (is_player)
		{
			switch (pMe->Class)
			{
			case TFClass::Soldier:
			case TFClass::Sniper:
			case TFClass::Spy:
			{
				IBaseWeapon pWpn(pMe->ActiveWeapon.get());

				if (pWpn)
				{
					ITFPlayer pPlayer(pEnt);

					switch (pWpn->GetClientClass()->ClassID)
					{
					case EntClassID::CTFCompoundBow:
					case EntClassID::CTFSniperRifle:
					case EntClassID::CTFSniperRifleClassic:
					case EntClassID::CTFSniperRifleDecap:
					{

						// Shot < Health, not enough to kill him
						// might aswell auto aim for the head
						if (pWpn->ChargedDamage < pPlayer->CurrentHealth && 
							 pMe->InCond(ETFCond::TF_COND_ZOOMED))
						{
							if (auto res = Trace_GenericHull(pMe, ITFPlayer(pEnt)); res.has_value())
							{
								cur_pos = res.value();
								cur_fov = pMe->EyePosition().DistToSqr(cur_pos);

								Vars.Smooth.mult(Vars.Smooth.Sniper);
								Vars.Smooth.Override_.update();
							}
						}

						break;
					}

					case EntClassID::CTFRevolver:
					{
						constexpr float perfect_time = 1.25f;
						static bool should_headshot = false;
						static ITimer timer_headshot;
						
						if (timer_headshot.has_elapsed(1.1f))
						{
							should_headshot = false;
							int idef = pWpn->ItemDefinitionIndex;
							
							if ((idef == 61 || idef == 1006) && Interfaces::GlobalVars->CurTime - pWpn->LastFireTime >= perfect_time)
							{
									const Vector pos = pPlayer->EyePosition(), my_eyepos = pMe->EyePosition();
								if (Trace::VecIsVisible(my_eyepos, pos, pPlayer, pMe))
								{
									should_headshot = true;
									timer_headshot.update();
								}
							}
						}

						if (should_headshot && timer_headshot.has_elapsed(0.15f))
						{
							if (auto res = Trace_GenericHull(pMe, ITFPlayer(pEnt)); res.has_value())
							{
								cur_pos = res.value();
								cur_fov = pMe->EyePosition().DistToSqr(cur_pos);

								Vars.Smooth.mult(Vars.Smooth.Spy);
								Vars.Smooth.Override_.update();
							}
						}

						break;
					}

					case EntClassID::CTFBaseRocket:
					case EntClassID::CTFRocketLauncher:
					case EntClassID::CTFRocketLauncher_AirStrike:
					case EntClassID::CTFRocketLauncher_DirectHit:
					case EntClassID::CTFRocketLauncher_Mortar:
					{
						constexpr PlayerHitboxIdx best_hitboxes[]
						{
							PlayerHitboxIdx::Hip_L,
							PlayerHitboxIdx::Foot_L,
							PlayerHitboxIdx::Hip_R,
							PlayerHitboxIdx::Foot_R,
						};

						const IBoneInfo info = pPlayer->QueryBoneInfo();
						if (info)
						{
							std::vector<BoneResult> positions;
							info.GetBonePosition(reinterpret_cast<const int*>(best_hitboxes), SizeOfArray(best_hitboxes), &positions);
							const Vector start = pMe->EyePosition();

							GameTrace tr;
							Trace::FilterSimple filter(pMe);
							const Vector eye_pos = pMe->EyePosition();

							double in_cur_fov = bad_fov;

							for (const BoneResult& br : positions)
							{
								Trace::TraceHull(eye_pos, br.Position, br.Min, br.Max, MASK_SHOT, &tr, &filter);
								in_cur_fov = start.DistToSqr(tr.EndPos);

								if (tr.DidHit(pEnt.get()) && cur_fov > in_cur_fov)
								{
									cur_pos = tr.EndPos;
									cur_fov = in_cur_fov;

									Vars.Smooth.mult(Vars.Smooth.Soldier);
								}
							}
						}
						break;
					}
					default: break;
					}

				}
				break;
				}
			}
		}

		Vector best_pos;
		double best_fov;
		if (!GetBestHitbox(pEnt, angle, best_pos, best_fov))
			return std::nullopt;
		
		if (cur_fov != bad_fov && (best_fov - cur_fov <= min_threshhold))
			return cur_pos;
		else
			return best_pos;
		}
	}

	return std::nullopt;
}

bool IAimbotHack::IAimbotState::IAimData::pass_checks(IAimbotHack* aim, const UserCmd* cmd) const
{
	if (auto res = aim->Filter_IsEntityVisible(target, is_player, cmd->ViewAngles); res.has_value())
	{
		results = res.value();
		if (this->is_player)
			results += ITFPlayer(target)->Velocity.get() * Interfaces::GlobalVars->FrameTime;
		return true;
	}
	return false;
}

bool IAimbotHack::CheckInputs(const UserCmd* cmd)
{
	static AutoCTimer<0.5f> timer_update_state;
	static bool key_state = true;

	if (!Vars.AutoAim && !(cmd->Buttons & IN_ATTACK))
	{
		Invalidate();
		return false;
	}

	if (Vars.KeyLock)
		key_state = GetAsyncKeyState(Vars.AimKey);
	else if (
		Vars.CanToggle &&
		timer_update_state.trigger_if_elapsed() &&
		GetAsyncKeyState(Vars.AimKey)
		) {
		key_state = !key_state;
	}

	return key_state;
}
	
