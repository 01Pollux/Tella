#include "Cheats/Main.hpp"

namespace Visuals
{
	class VisualsHandler
	{
	public:
		VisualsHandler();

	private:
		void OnRender();

		M0Config::BoolRef HideScope;
		M0Config::BoolRef DrawDeadPlayers;
		M0Config::ArrayXYRef<int> DrawDeadPlayersOffset;
	};
	static VisualsHandler visuals_handler;
}

void Visuals::VisualsHandler::OnRender()
{
	if (ImGui::CollapsingHeader("Visual"))
	{
		ImGui::Checkbox("No scope", HideScope.data());
		ImGui::SameLineHelp(HideScope);

		ImGui::Checkbox("Spectators", DrawDeadPlayers.data());
		ImGui::SameLineHelp(DrawDeadPlayers);

		ImGui::SliderInt2("Text Adjustor", DrawDeadPlayersOffset.data(), 0, 1980);
		ImGui::SameLineHelp(DrawDeadPlayersOffset);
	}
}

Visuals::VisualsHandler::VisualsHandler()
{
	M0EventManager::AddListener(
		EVENT_KEY_RENDER_EXTRA,
		std::bind(&VisualsHandler::OnRender, this),
		"Visuals::OnRender"
	);

	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			HideScope.init("Visual.Hide Scope");
			DrawDeadPlayers.init("Visual.Spectators.Enabled");
			DrawDeadPlayersOffset.init("Visual.Spectators.Position");
		},
		EVENT_NULL_NAME
	);
}