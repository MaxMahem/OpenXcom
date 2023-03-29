#include <SDL.h>
#include <assert.h>
#include "InventoryItemSprite.h"
#include "..\Engine\Surface.h"
#include "..\Engine\SurfaceSet.h"
#include "..\Engine\Script.h"
#include "..\Engine\ScriptBind.h"
#include "..\Mod\Mod.h"
#include "..\Mod\ModScript.h"
#include "..\Mod\RuleItem.h"
#include "..\Mod\RuleInventory.h"
#include "..\Mod\RuleInterface.h"
#include "..\Mod\RuleEnviroEffects.h"
#include "..\Savegame\BattleItem.h"
#include "..\Savegame\BattleUnit.h"
#include "..\Savegame\SavedBattleGame.h"
#include "..\Interface\Text.h"
#include "..\Interface\NumberText.h"

namespace OpenXcom
{
/**
 * @brief Draws the inventory item sprite in the current context.
 * @param surfaceSet The set that contains this sprite (should be BIGOBS.PCK).
 * @param animationFrame The current animation frame.
*/
void InventoryItemSprite::draw(const SurfaceSet& surfaceSet, const SavedBattleGame& saveBattleGame, int animationFrame)
{
	const Surface* sprite = _battleItem.getBigSprite(&surfaceSet, &saveBattleGame, animationFrame);

	if (!sprite)
	{
		const auto id = std::to_string(_battleItem.getId());
		LogOnce("drawSprite" + id, LOG_WARNING) << "Attempted to draw item without sprite. item id: " << id;
		return;
	}

	int invSlotW = _itemRules.getInventoryWidth();
	int invSlotH = _itemRules.getInventoryHeight();
	if (invSlotW == 0 || invSlotH == 0)
	{
		auto id = std::to_string(_battleItem.getId());
		LogOnce("drawSprite" + id, LOG_WARNING) << "Attempted to draw item with width: " << invSlotW << " or height: " << invSlotH << " of 0. item id: " << id;
		return;
	}

	BattleItem::ScriptFill(&_scriptWorker, &_battleItem, &saveBattleGame, BODYPART_ITEM_INVENTORY, animationFrame, 0);
	_scriptWorker.executeBlit(sprite, &target, spriteBounds.x, spriteBounds.y, 0);
}

/**
 * @brief Draws the sprite overlays, including related scripting.
 * @param animationFrame The current animation frame.
*/
void InventoryItemOverlay::draw(const SavedBattleGame& saveBattleGame, int animationFrame)
{
	ModScript::scriptCallback<ModScript::InventorySpriteOverlay>(&_itemRules, &_battleItem, &saveBattleGame, this, animationFrame);

	if (options & OverlayOptions::DRAW_GRENADE)		 { drawGrenadePrimedIndicator(saveBattleGame, animationFrame); }
	if (options & OverlayOptions::DRAW_FATAL_WOUNDS) { drawFatalWoundIndicator(saveBattleGame); }
	if (options & OverlayOptions::DRAW_CORPSE_STATE) { drawCorpseIndicator(saveBattleGame, animationFrame); }
}

/**
 * @brief Draws the hand-slot overelays, including related scripting.
 * @param animationFrame Current animation frame.
*/
void InventoryItemOverlay::drawHandOverlay(const SavedBattleGame& saveBattleGame, int animationFrame)
{
	assert(_battleItem->getSlot()->getType() == INV_HAND); // check the item is indeed in hand.

	ModScript::scriptCallback<ModScript::HandOverlay>(&_itemRules, &_battleItem, &saveBattleGame, this, animationFrame);
	if (options & OverlayOptions::DRAW_AMMO)	{ drawAmmoIndicator(saveBattleGame); }
	if (options & OverlayOptions::DRAW_MEDKIT)  { drawMedkitIndicator(saveBattleGame); }
	if (options & OverlayOptions::DRAW_TWOHAND) { drawTwoHandIndicator(saveBattleGame); }
}

namespace // some short functions to help with rendering
{
/**
 * @brief Transforms a number into its position in a triangle wave.
 * For example, triangleWave(x, 8, 4) => (0, 1, 2, 3, 4, 3, 2, 1)
*/
int triangleWave(int number, int period, int amplitude){ return abs((number % period) - amplitude); }

/// Gets the number of digits in a positive number less than 1000.
int getDigits(int number) { return number < 10 ? 1 : number < 100 ? 2 : 3; }

std::pair<int, int> topLeft(const SDL_Rect& bounds, int numW, int numH, int spacing)
{
	return std::pair{bounds.x + spacing, bounds.y + spacing};
}

std::pair<int, int> topRight(const SDL_Rect& bounds, int numW, int numH, int spacing)
{
	return std::pair{bounds.x + bounds.w - numW - spacing, bounds.y + spacing};
}

std::pair<int, int> bottomLeft(const SDL_Rect& bounds, int numW, int numH, int spacing)
{
	return std::pair{bounds.x + spacing, bounds.y + bounds.h - numH - spacing};
}

std::pair<int, int> bottomRight(const SDL_Rect& bounds, int numW, int numH, int spacing)
{
	return std::pair(bounds.x + bounds.w - numW - spacing, bounds.y + bounds.h - numH - spacing);
}

} // namespace

/**
 * @brief Draws a numberText relative to a corner of this overlays bounding box.
 * @param getChildCoord Function to get the approrpiate corner offset.
 * @param spacing Amount of additional space to move from the corner.
 * @param number The number to draw (must be positive and less than 1000).
 * @param color The color to draw the number in.
 * @param yRowOffset an optional additional y-offset.
 * @param bordered if the number should be bordered or not.
*/
void InventoryItemOverlay::drawNumberCorner(const cornerFunc getChildCoord, int spacing, int number, int color, int yRowOffset, bool bordered)
{
	int numW = getDigits(number) * (bordered ? 5 : 4); /// width of number.
	int numH = bordered ? 6 : 5;                       /// height of number.

	auto [x, y] = getChildCoord(bounds, numW, numH, spacing);
	auto numText = NumberText(numW, numH, x, y + yRowOffset * (numH + 1));

	numText.setColor(color);
	numText.setBordered(bordered);
	numText.setValue(number);
	numText.setPalette(target.getPalette());

	numText.blit(target.getSurface());
}

void InventoryItemOverlay::drawGrenadePrimedIndicator(const SavedBattleGame& saveBattleGame, int animFrame)
{
	if (_battleItem.getFuseTimer() < 0) { return; }

	const Surface* primedIndicator = saveBattleGame.getMod()->getSurfaceSet("SCANG.DAT")->getFrame(6);
	int newColor = _battleItem.isFuseEnabled() ? 0 : 32;

	primedIndicator->blitNShade(&target, bounds.x, bounds.y, triangleWave(animFrame, 8, 4), false, newColor);
}

void InventoryItemOverlay::drawCorpseIndicator(const SavedBattleGame& saveBattleGame, int animFrame)
{
	const auto unit = _battleItem.getUnit();
	if (!unit || unit->getStatus() != STATUS_UNCONSCIOUS) { return; }

	const Surface* corpseStateIndicator = unit->getFire()        > 0     ? saveBattleGame.getMod()->getSurface("BigBurnIndicator") :
										  unit->getFatalWounds() > 0     ? saveBattleGame.getMod()->getSurface("BigWoundIndicator") :
										  unit->hasNegativeHealthRegen() ? nullptr /// TODO get shock here, it's complicated.
																		 : saveBattleGame.getMod()->getSurface("BigStunIndicator");
	
	corpseStateIndicator->blitNShade(&target, bounds.x, bounds.y, triangleWave(animFrame, 8, 4));
}

void InventoryItemOverlay::drawFatalWoundIndicator(const SavedBattleGame& saveBattleGame)
{
	const auto unit = _battleItem.getUnit();
	if (!unit || unit->getStatus() != STATUS_UNCONSCIOUS) { return; }

	int woundCount = unit->getFatalWounds();

	const auto battlescapeInterface = saveBattleGame.getMod()->getInterface("inventory");
	const auto woundElement = battlescapeInterface->getElement("fatalWounds");
	int woundColor = woundElement ? woundElement->color : battlescapeInterface->getElement("numStack")->color2;

	drawNumberCorner(bottomRight, 0, woundCount, woundColor, 0, true);
}

void InventoryItemOverlay::drawAmmoIndicator(const SavedBattleGame& saveBattleGame)
{
	const auto battlescapeInterface = saveBattleGame.getMod()->getInterface("battlescape");
	int ammoColor = _battleItem.getSlot()->isRightHand() ? battlescapeInterface->getElement("numAmmoRight")->color :
					_battleItem.getSlot()->isLeftHand()  ? battlescapeInterface->getElement("numAmmoLeft")->color
														 : throw std::logic_error("item in hand with bad hand value.");

	for (int slot = 0; slot < RuleItem::AmmoSlotMax; ++slot)
	{
		if (_battleItem.isAmmoVisibleForSlot(slot))
		{
			const BattleItem* ammo = _battleItem.getAmmoForSlot(slot);
			int ammoQuant = ammo ? ammo->getAmmoQuantity() : 0;

			drawNumberCorner(topLeft, 0, ammoQuant, ammoColor, slot);
		}
	}
}

void InventoryItemOverlay::drawMedkitIndicator(const SavedBattleGame& saveBattleGame)
{
	if (_itemRules.getBattleType() != BT_MEDIKIT) { return; }

	const int defaultColor = 0;
	const auto battlescapeInterface = saveBattleGame.getMod()->getInterface("battlescape");
	const auto getColor = [&](std::string elementName) -> int
	{
		const auto element = battlescapeInterface->getElement(elementName);
		return element ? element->color : defaultColor;
	};

	drawNumberCorner(bottomLeft, 1, _battleItem.getPainKillerQuantity(), getColor("painKillerDoses"), -2);
	drawNumberCorner(bottomLeft, 1, _battleItem.getStimulantQuantity(),  getColor("stimulantDoses"), -1);
	drawNumberCorner(bottomLeft, 1, _battleItem.getHealQuantity(),       getColor("healDoses"), 0);
}

void InventoryItemOverlay::drawTwoHandIndicator(const SavedBattleGame& saveBattleGame)
{
	if (!_itemRules.isTwoHanded()) { return; }

	const auto battlescapeInterface = saveBattleGame.getMod()->getInterface("battlescape");
	int color = _itemRules.isBlockingBothHands() ? battlescapeInterface->getElement("twoHandedRed")->color
			  									 : battlescapeInterface->getElement("twoHandedGreen")->color;

	drawNumberCorner(bottomRight, 1, 2, color);
}

//// Script binding
namespace
{
void blitScript(InventoryItemOverlay* dest, const Surface* source, int x, int y)
{
	source->blitNShade(&dest->target, dest->bounds.x + x, dest->bounds.y + y, 0, false, 0);
}
void blitCropScript(InventoryItemOverlay* dest, const Surface* source, int x1, int y1, int x2, int y2)
{
	GraphSubset crop = GraphSubset({dest->bounds.x + x1, dest->bounds.x + x2}, {dest->bounds.y + y1, dest->bounds.y + y2});
	source->blitNShade(&dest->target, dest->bounds.x, dest->bounds.y, 0, crop);
}
void blitShadeCropScript(InventoryItemOverlay* dest, const Surface* source, int shade, int x, int y, int x1, int y1, int x2, int y2)
{
	GraphSubset crop = GraphSubset({x1, x2}, {y1, y2});
	source->blitNShade(&dest->target, dest->bounds.x + x, dest->bounds.y + y, shade, crop);
}
void blitNShadeScript(InventoryItemOverlay* dest, const Surface* source, int x, int y, int shade)
{
	source->blitNShade(&dest->target, dest->bounds.x + x, dest->bounds.y + y, shade, false, 0);
}
void blitNShadeNRecolorScript(InventoryItemOverlay* dest, const Surface* source, int x, int y, int shade, int newColor)
{
	source->blitNShade(&dest->target, dest->bounds.x, dest->bounds.y, shade, false, newColor);
}

/// Todo: finish this script for gods sake!
void drawTextScript(InventoryItemOverlay* surf, const std::string& text, int width, int height, int x, int y, int color)
{
	auto surfaceText = Text(width, height, x, y);
	surfaceText.setPalette(surf->target.getPalette());
	surfaceText.setColor(color);
	// surfaceText.blitNShade(surf, x, y);
	// surf->drawString(x, y, text.c_str(), color);
}

void drawNumberScript(InventoryItemOverlay* dest, int value, int width, int height, int x, int y, int color)
{
	NumberText text = NumberText(width, height, dest->bounds.x + x, dest->bounds.y + y);
	text.setPalette(dest->target.getPalette());
	text.setColor(color);
	text.setBordered(false);
	text.setValue(value);
	text.blit(dest->target.getSurface());
}

void getOptionsScript(InventoryItemOverlay* overlay, int& options)
{
	options = overlay->options;
}

void setOptionsScript(InventoryItemOverlay* overlay, int options)
{
	overlay->options = static_cast<InventoryItemOverlay::OverlayOptions>(overlay->options | options);
}

void unsetOptionsScript(InventoryItemOverlay* overlay, int options)
{
	overlay->options = static_cast<InventoryItemOverlay::OverlayOptions>(overlay->options & ~options);
}

void drawLineScript(InventoryItemOverlay* dest, int x1, int y1, int x2, int y2, int color)
{
	dest->target.drawLine(dest->bounds.x + x1, dest->bounds.y + y1, dest->bounds.x + x2, dest->bounds.y + y2, color);
}
void drawRectScript(InventoryItemOverlay* dest, int x1, int y1, int x2, int y2, int color)
{
	dest->target.drawRect(dest->bounds.x + x1, dest->bounds.y + y1, dest->bounds.x + x2, dest->bounds.y + y2, color);
}
void drawCircScript(InventoryItemOverlay* dest, int x, int y, int radius, int color)
{
	dest->target.drawCircle(dest->bounds.x + x, dest->bounds.y + y, radius, color);
}

std::string debugDisplayScript(const InventoryItemOverlay* overlay)
{
	if (overlay)
	{
		std::ostringstream output;
		output << " (x:" << overlay->bounds.x << " y:" << overlay->bounds.y << " w:" << overlay->bounds.w << " h:" << overlay->bounds.x << ")";
		return output.str();
	}
	else
	{
		return "null";
	}
}

} // namespace.

void InventoryItemOverlay::ScriptRegister(ScriptParserBase* parser)
{
	Bind<InventoryItemOverlay> invItemSpriteBinder = {parser};

	invItemSpriteBinder.addCustomConst("INV_SLOT_W", RuleInventory::SLOT_W);
	invItemSpriteBinder.addCustomConst("INV_SLOT_H", RuleInventory::SLOT_H);
	invItemSpriteBinder.addCustomConst("INV_HAND_SLOT_COUNT_W", RuleInventory::HAND_W);
	invItemSpriteBinder.addCustomConst("INV_HAND_SLOT_COUNT_H", RuleInventory::HAND_H);
	invItemSpriteBinder.addCustomConst("INV_HAND_OVERLAY_W", RuleInventory::HAND_W * RuleInventory::SLOT_W);
	invItemSpriteBinder.addCustomConst("INV_HAND_OVERLAY_H", RuleInventory::HAND_H * RuleInventory::SLOT_H);

	invItemSpriteBinder.addCustomConst("OVERLAY_DRAW_GRENADE_INDICATOR", InventoryItemOverlay::DRAW_GRENADE);
	invItemSpriteBinder.addCustomConst("OVERLAY_DRAW_CORPSE_STATE", InventoryItemOverlay::DRAW_CORPSE_STATE);
	invItemSpriteBinder.addCustomConst("OVERLAY_DRAW_FATAL_WOUNDS", InventoryItemOverlay::DRAW_FATAL_WOUNDS);
	invItemSpriteBinder.addCustomConst("OVERLAY_DRAW_AMMO", InventoryItemOverlay::DRAW_AMMO);
	invItemSpriteBinder.addCustomConst("OVERLAY_DRAW_MEDKIT_AMMO", InventoryItemOverlay::DRAW_MEDKIT);
	invItemSpriteBinder.addCustomConst("OVERLAY_DRAW_TWOHAND_INDICATOR", InventoryItemOverlay::DRAW_TWOHAND);

	invItemSpriteBinder.add<&getOptionsScript>("getOptions", "Gets the current options.");
	invItemSpriteBinder.add<&setOptionsScript>("setOptions", "Sets (turns on) a given option or options.");
	invItemSpriteBinder.add<&unsetOptionsScript>("unsetOptions", "Unsets (turns off) a given option or options.");

	invItemSpriteBinder.add<&blitScript>("blit", "Blits a sprite onto the overlay.");
	invItemSpriteBinder.add<&blitCropScript>("blitCrop", "Blits a sprite onto the overlay with a crop. (sprite, x, y, cropX1, cropY1, cropX2, cropY2)");
	invItemSpriteBinder.add<&blitNShadeScript>("blitShade", "Blits and shades a sprite onto the overlay.");
	invItemSpriteBinder.add<&blitNShadeNRecolorScript>("blitShadeRecolor", "Blits, shades, and recolors a sprite onto the overlay.");

	invItemSpriteBinder.add<&drawTextScript>("drawText", "Draws text on a sprite.");
	invItemSpriteBinder.add<&drawNumberScript>("drawNumber", "Draws number on a sprite. (number width height x y color)");

	invItemSpriteBinder.add<&drawLineScript>("drawLine", "Draws a line on a sprite. (x1 y1 x2 y2 color)");
	invItemSpriteBinder.add<&drawRectScript>("drawRect", "Draws a rectange on a sprite. (x1 y1 x2 y2 color)");
	invItemSpriteBinder.add<&drawCircScript>("drawCirc", "Draws a circle on a sprite. (x y radius color)");

	invItemSpriteBinder.add<&InventoryItemOverlay::getWidth>("getWidth", "Gets the width of this overlay.");
	invItemSpriteBinder.add<&InventoryItemOverlay::getHeight>("getHeight", "Gets the height of this overlay.");

	invItemSpriteBinder.addDebugDisplay<&debugDisplayScript>();
}

namespace
{

void commonOverlayImpl(BindBase& b, Mod* mod)
{
	b.addCustomPtr<const Mod>("rules", mod);
}

}

/**
 * Constructor of inventory sprite overlay script parser.
 */
ModScript::InventorySpriteOverlayParser::InventorySpriteOverlayParser(ScriptGlobal* shared, const std::string& name, Mod* mod)
	: ScriptParserEvents{shared, name, "item", "battle_game", "overlay", "anim_frame"}
{
	BindBase b{this};

	commonOverlayImpl(b, mod);
}

/**
 * Constructor of hand overlay script parser.
 */
ModScript::HandOverlayParser::HandOverlayParser(ScriptGlobal* shared, const std::string& name, Mod* mod)
	: ScriptParserEvents{shared, name, "item", "battle_game", "overlay", "anim_frame"}
{
	BindBase b{this};

	commonOverlayImpl(b, mod);
}

}
