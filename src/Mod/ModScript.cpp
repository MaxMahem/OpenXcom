#include "..\SaveGame\BattleUnit.h"
#include "..\Savegame\SavedBattleGame.h"
#include "..\Engine\ScriptBind.h"
#include "ModScript.h"
#include "Mod.h"

namespace OpenXcom
{

namespace
{

void commonOverlayImpl(BindBase &b, Mod *mod)
{
	b.addCustomPtr<const Mod>("rules", mod);
}

}

/**
 * Constructor of inventory sprite overlay script parser.
 */
ModScript::InventorySpriteOverlayParser::InventorySpriteOverlayParser(ScriptGlobal *shared, const std::string &name, Mod *mod) : ScriptParserEvents{ shared, name,
	"item", "battle_game", "overlay", "anim_frame" }
{
	BindBase b { this };

	commonOverlayImpl(b, mod);
}

/**
 * Constructor of hand overlay script parser.
 */
ModScript::HandOverlayParser::HandOverlayParser(ScriptGlobal* shared, const std::string& name, Mod* mod) : ScriptParserEvents{shared, name,
	"item", "battle_game", "overlay", "anim_frame"}
{
	BindBase b{this};

	commonOverlayImpl(b, mod);
}

}
