#pragma once
#include "..\Engine\Surface.h"
#include "..\Engine\SurfaceSet.h"
#include "..\Engine\Script.h"
#include "..\Savegame\SavedBattleGame.h"
// #include "..\Mod\RuleInventory.h"
// #include "..\Savegame\BattleItem.h"

namespace OpenXcom
{

class BattleItem;
class RuleItem;
// class SaveBattleGame;

class InventoryItemSprite
{
private:
	/// the item this sprite represents
	const BattleItem* _battleItem;
	/// rules for the item.
	const RuleItem* _itemRules;
	/// Set that contains the sprite.
	inline static SurfaceSet* _bigObsSurfaceSet;
	/// Surface that contains the sprite.
	Surface _overlay;
	/// Surface used as a hand slot overlay for scripting.
	Surface _handOverlay;
	/// Worker for pixel level script blitting.
	ScriptWorkerBlit _scriptWorker {};

	/// get the appropriate sprite for drawing, including scripting.
	const Surface* getSprite(const SavedBattleGame* save, int animFrame) const;

public:
	InventoryItemSprite(const BattleItem* battleItem);

	void draw(Surface& target, const SavedBattleGame* saveBattleGame, int animationFrame, int groundOffset = 0);
};

}
