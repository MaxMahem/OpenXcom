#pragma once
#include "..\Engine\Surface.h"
#include "..\Engine\SurfaceSet.h"
#include "..\Engine\Script.h"
#include "..\Savegame\SavedBattleGame.h"
#include "..\Savegame\BattleItem.h"

namespace OpenXcom
{

class RuleItem;

class InventoryItemSprite
{
private:
	/// The item this sprite represents.
	const BattleItem& _battleItem;
	/// Rules for the item.
	const RuleItem& _itemRules;
	/// Worker for pixel level script blitting.
	ScriptWorkerBlit _scriptWorker {};

public:
	InventoryItemSprite(const BattleItem& battleItem, Surface& target, SDL_Rect& spriteBounds)
	  : _battleItem(battleItem), _itemRules(*battleItem.getRules()), target(target), spriteBounds(spriteBounds) {}

	// no copy or move.
	InventoryItemSprite(InventoryItemSprite&) = delete;
	InventoryItemSprite& operator=(InventoryItemSprite&) = delete;

	/// Surface this item should be draw to.
	Surface& target;
	/// Bounding box for the sprite.
	const SDL_Rect spriteBounds;

	/// Draw the sprite, including scripting.
	void draw(const SurfaceSet& surfaceSet, const SavedBattleGame& saveBattleGame, int animationFrame);
};

class InventoryItemOverlay
{
private:
	/// The item this overlay covers represents.
	const BattleItem& _battleItem;
	/// The rules for the item.
	const RuleItem& _itemRules;

public:
	/// surface this item should be draw to.
	Surface& target;
	/// bounding box for the overlay.
	const SDL_Rect bounds;
	/// The options to use when displaying this overlay.
	enum OverlayOptions : int
	{
		DRAW_NONE = 0,
		DRAW_GRENADE = 1 << 0,
		DRAW_CORPSE_STATE = 1 << 1,
		DRAW_FATAL_WOUNDS = 1 << 2,
		DRAW_AMMO = 1 << 3,
		DRAW_MEDKIT = 1 << 4,
		DRAW_TWOHAND = 1 << 5,
		DRAW_INV_SLOT = DRAW_GRENADE | DRAW_CORPSE_STATE | DRAW_FATAL_WOUNDS,
		DRAW_ALL = INT_MAX,
	} options;

	/// Creates a new overlay.
	InventoryItemOverlay(const BattleItem& battleItem, Surface& target, SDL_Rect& spriteBounds, OverlayOptions options)
	  : _battleItem(battleItem), _itemRules(*battleItem.getRules()), target(target), bounds(spriteBounds), options(options) {}

	// no copy or move.
	InventoryItemOverlay(InventoryItemSprite&) = delete;
	InventoryItemOverlay& operator=(InventoryItemSprite&) = delete;

	/// Draws the sprite overlays and executes related scripts.
	void draw(const SavedBattleGame& saveBattleGame, int animationFrame);
	/// Draws the hand-slot overlays and executes related scripts.
	void drawHandOverlay(const SavedBattleGame& saveBattleGame, int animationFrame);

	/// Gets the width of this overlay.
	int getWidth() const  { return bounds.w; }
	/// Gets the height of this overlay.
	int getHeight() const { return bounds.h; }

	/// Name of class used in script.
	static constexpr const char* ScriptName = "InvItemOverlay";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

	static const OverlayOptions INVENTORY_HAND = OverlayOptions::DRAW_ALL;
	static const OverlayOptions INVENTORY_SLOT = OverlayOptions::DRAW_INV_SLOT;
	static const OverlayOptions INVENTORY_GRND = OverlayOptions::DRAW_INV_SLOT;
	static const OverlayOptions ALIEN_INV_HAND = OverlayOptions::DRAW_NONE;
	static const OverlayOptions BATTSCAPE_HAND = OverlayOptions::DRAW_ALL;
	static const OverlayOptions INVENTORY_CRSR = OverlayOptions::DRAW_NONE;
	static const OverlayOptions INVENTORY_AMMO = OverlayOptions::DRAW_NONE;

private:
	/// Draw the grande primed indicator.
	void drawGrenadePrimedIndicator(const SavedBattleGame& saveBattleGame, int animFrame);
	/// Draw the ammo indicators (ammo quantity).
	void drawAmmoIndicator(const SavedBattleGame& saveBattleGame);
	/// Draw the Medkit indicators (medicine doses).
	void drawMedkitIndicator(const SavedBattleGame& saveBattleGame);
	/// Draw the two-handed indicator.
	void drawTwoHandIndicator(const SavedBattleGame& saveBattleGame);
	/// Draw the corpse wound indicator.
	void drawCorpseIndicator(const SavedBattleGame& saveBattleGame, int animationFrame);
	/// Draw the fatal wounds indicator.
	void drawFatalWoundIndicator(const SavedBattleGame& saveBattleGame);

	typedef std::pair<int, int> (*cornerFunc)(const SDL_Rect&, int, int, int);

	/// Draws a number in a given corner.
	void drawNumberCorner(cornerFunc getChildCoord, int spacing, int number, int color, int yRowOffset = 0, bool bordered = false);
};

}
