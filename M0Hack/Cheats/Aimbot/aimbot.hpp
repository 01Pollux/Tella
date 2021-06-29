#pragma once

#include "../Main.hpp"
#include "aimbot_particles.hpp"
#include "aimbot_var.hpp"

#include "EntityList.hpp"
#include "Helper/DrawTools.hpp"

#include "Helper/Timer.hpp"
#include "Helper/Config.hpp"

#include "Hooks/CreateMove.hpp"
#include "Profiler/mprofiler.hpp"

#include <optional>

class IAimbotHack 
{
public:
	IAimbotHack();

	void OnLoadDLL();
	void OnUnloadDLL();

	void OnRender();

	MHookRes OnCreateMovePre(UserCmd*);
	MHookRes OnCreateMovePost(UserCmd*);

	void ToggleAutoAim() noexcept
	{
		Vars.AutoAim = !Vars.AutoAim;
	}

	void Invalidate();

private:
	void DrawDebug(IBaseEntity pEnt, const Vector& aim_pos);

	void GetBestPlayer(const UserCmd*);
	_NODISCARD bool IsValidPlayer(const ITFPlayer);
	void GetBestEntity(const UserCmd*);
	_NODISCARD bool IsValidEntity(const IBaseEntity, const TFTeam);

	_NODISCARD bool CheckInputs(const UserCmd*);

	_NODISCARD int GetShootRange(EntClassID clsID)
	{
		switch (clsID)
		{
		case EntClassID::CTFFlameThrower:
			return 312;
		case EntClassID::CTFWeaponFlameBall:
			return 512;
		default:
			return Vars.MaxRange;
		}
	}

	_NODISCARD std::optional<Vector> Filter_IsEntityVisible(const IBaseEntity pEnt, bool is_player, const QAngle& angle);

private:
	M0HookManager::Policy::CreateMove CreateMoveHandler{ };
	bool ShouldProcess = false;

	struct IAimbotVars
	{
	public:
		M0Config::Bool Enable { 
			AIMBOT_SECTION("Enable"), 
			true, 
			"Enable aimbot"
		};

		M0Config::Bool PlayerOnly {
			AIMBOT_SECTION("Player only"), 
			true,
			"Only target players"
		};

		M0Config::Bool AimForBuildings { 
			AIMBOT_SECTION("Aim for buildings"),
			true, 
			"Target buildings"
		};

		M0Config::Bool AimForProjectiles {
			AIMBOT_SECTION("Aim for projectiles"),
			false,
			"Target projectiles"
		};

		M0Config::Bool OnlyZoom { 
			AIMBOT_SECTION("Only zoom"), 
			true,
			"Only aim if player is scoped as a sniper"
		};

		struct
		{
			M0Config::Bool Enable {
				AIMBOT_SECTION("Target lock"),
				true,
				"Keep track of old target"
			};
			bool Invalid_{ true };

			bool should_retarget() noexcept
			{
				if (!Enable)
					return false;
				return !Invalid_;
			}
		} TargetLock;

		M0Config::Bool KeyLock {
			AIMBOT_SECTION("Input.Key lock"),
			false,
			"Only aim if key is down"
		};

		M0Config::Bool AutoAim {
			AIMBOT_SECTION("Input.Auto aim"),
			false,
			"Aim without pressing L-Mouse"
		};

		M0Config::Bool AutoShoot { 
			AIMBOT_SECTION("Input.Auto shoot"),
			false, 
			"Automatically shoot if the target is in range" 
		};

		M0Config::Bool CanToggle {
			AIMBOT_SECTION("Input.Can toggle"),
			true,
			"allow user to set on/off aim state" 
		};

		M0Config::Int AimKey {
			AIMBOT_SECTION("Input.Aim key"),
			VK_DELETE, 
			"Enable aimbot by pressing key (in hexadecimal)" 
		};

		M0Config::Int MaxRange {
			AIMBOT_SECTION("Max range"),
			1024, 
			"Max valid range to target" 
		};

		M0Config::Int MissChance {
			AIMBOT_SECTION("Miss Chance"),
			14,
			"Chance to invalidate/miss the target"
		};

		M0Config::Float FOV {
			AIMBOT_SECTION("FOV"),
			45.f,
			"Target must be in this fov to be a valid target"
		};

		struct
		{
			M0Config::Float Standard {
				AIMBOT_SECTION("Smooth.Default"),
				35.0f,
				"Default FOV decrease rate"
			};

			M0Config::Float Sniper {
				AIMBOT_SECTION("Smooth.Sniper"),
				0.19f,
				M0CONFIG_NULL_DESCRIPTION
			};

			M0Config::Float Spy {
				AIMBOT_SECTION("Smooth.Spy"),
				0.09f,
				M0CONFIG_NULL_DESCRIPTION
			};

			M0Config::Float Soldier {
				AIMBOT_SECTION("Smooth.Soldier"),
				0.71f,
				M0CONFIG_NULL_DESCRIPTION
			};

			AutoCTimer<0.73f> Override_;

			float get() const noexcept { return Intern_; }
			void reset() noexcept { Intern_ = Standard; }
			void mult(const float val) noexcept { Intern_ *= val; }

		private:
			float Intern_;
		} Smooth;

		M0Config::Bool BypassCloak { 
			AIMBOT_SECTION("Bypass cloak"),
			false,
			"Don't ignore invisible spies"
		};

		M0Config::Bool BypassBadConds { 
			AIMBOT_SECTION("Bypass conds"),
			true, 
			"Don't ignore 'invisible' conds"
		};

		M0Config::Bool MedicOnly {
			AIMBOT_SECTION("Only medics"),
			false, 
			"Only target medics"
		};

		struct
		{
			M0Config::Bool DrawHitbox {
				AIMBOT_SECTION("Debug.Hitbox"),
				false,
				M0CONFIG_NULL_DESCRIPTION
			};
			M0Config::Bool DrawBox {
				AIMBOT_SECTION("Debug.Box"),
				false,
				M0CONFIG_NULL_DESCRIPTION
			};
			M0Config::Bool DrawLine {
				AIMBOT_SECTION("Debug.Line"),
				false,
				M0CONFIG_NULL_DESCRIPTION
			};

			M0Config::RTTimer RepeatTimer {
				AIMBOT_SECTION("Debug.Delay"),
				1.5f,
				"Repeat Delay"
			};

			bool should_draw() noexcept
			{
				if (DrawHitbox || DrawBox || DrawLine)
					return RepeatTimer.trigger_if_elapsed();
				return false;
			}
		} Debug;

		struct
		{
			M0Config::Bool Enable {
				AIMBOT_SECTION("Particle.Enable"),
				true,
				"Draw particles for target"
			};

			M0Config::String String {
				AIMBOT_SECTION("Particle.Name"),
				"soldierbuff_blue_buffed", 
				"Particle name string"
			};

			IAimbotParticleHelper Handler_;
		} ParticleInfo;

		struct
		{
			M0Config::Bool Enable {
				AIMBOT_SECTION("Glow.Enable"),
				false,
				"Display glow effect for target (requires mat_dxlevel >= 9)"
			};
			M0Config::U8Color Color {
				AIMBOT_SECTION("Glow.Color"),
				color::names::cyan,
				"Glow Color"
			};

			IAimbotGlowHelper Handler_;
		} GlowInfo;

		M0Config::AimPriority TargetPriority{ 
			AIMBOT_SECTION("Aim Priority"),
			AimTargetPriority::FovAndDist, 
			"Filter Entity by given priority"
		};

		M0Config::HBPriority HitboxPriority{
			AIMBOT_SECTION("Hitbox Priority"),
			HitboxPriority::Smart,
			"Process best aim position, Smart = Handles localplayer weapon's then switch to 'Nearest' if it fails"
		};

	public:
		constexpr static size_t size_of_particle() { return 64U; }

		IAimbotVars()
		{
			ParticleInfo.String.get().reserve(size_of_particle());
		}
	};
	IAimbotVars Vars;

	class IAimbotState
	{
	public:
		struct IAimData
		{
			mutable Vector results;
			IBaseHandle target_hndl;
			IBaseEntity target;
			bool is_player;

			/// return true if the current data is valid + output the new results
			/// to IAimbotState::IAimData::results
			/// else if it's false, you must use IAimbotState::IAimData::invalidate()
			_NODISCARD bool validate(IAimbotHack* aim, const UserCmd* cmd) const
			{
				if (!target)
					return false;

				return (IBaseEntity(target_hndl) == target && pass_checks(aim, cmd));
			}

			void invalidate() noexcept
			{
				target = nullptr;
			}

		private:
			_NODISCARD bool pass_checks(IAimbotHack*, const UserCmd*) const;
		};

		_NODISCARD IAimData& get() noexcept
		{
			return last_data;
		}

		void set(IBaseEntity pEnt, bool is_player)
		{
			last_data.target = pEnt;
			last_data.target_hndl = pEnt ? pEnt->GetRefEHandle() : IBaseHandle{ };
			last_data.is_player = is_player;
		}

	private:
		IAimData last_data;
	};
	IAimbotState AimbotState;

	/// @out : reference to IAimbotState::last_data 
	///	should invalidate if it fails IAimbotState::validate(const CUserCmd*) check
	[[nodiscard]] IAimbotState::IAimData& GetBestTarget(const UserCmd*);

	[[nodiscard]] IAimbotState::IAimData& GetTargetNoReset();
};