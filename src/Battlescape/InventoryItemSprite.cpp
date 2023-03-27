#include "InventoryItemSprite.h"
#include "..\Engine\Surface.h"
#include "..\Engine\SurfaceSet.h"
#include "..\Engine\Script.h"
#include "..\Mod\Mod.h"
#include "..\Mod\ModScript.h"
#include "..\Mod\RuleItem.h"
#include "..\Mod\RuleInventory.h"
#include "..\Savegame\BattleItem.h"
#include "..\Savegame\BattleUnit.h"
#include "..\Savegame\SavedBattleGame.h"

namespace OpenXcom
{

InventoryItemSprite::InventoryItemSprite(const BattleItem* battleItem)
	: _battleItem(battleItem), _itemRules(battleItem->getRules()),
	_handOverlay(RuleInventory::HAND_W * RuleInventory::SLOT_W, RuleInventory::HAND_H * RuleInventory::SLOT_H)
{
	int width = _itemRules->getInventoryWidth() * RuleInventory::SLOT_W;
	int height = _itemRules->getInventoryHeight() * RuleInventory::SLOT_H;

	_overlay = Surface(width, height);
}

const Surface* InventoryItemSprite::getSprite(const SavedBattleGame* save, int animFrame) const
{
	if (!_bigObsSurfaceSet)
	{
		_bigObsSurfaceSet = save->getMod()->getSurfaceSet("BIGOBS.PCK");
	}

	int index = _itemRules->getBigSprite();
	if (index == -1)
	{
		return nullptr;
	}

	const Surface* surf = _bigObsSurfaceSet->getFrame(index);
	// enforce compatibility with basic version
	if (surf == nullptr)
	{
		throw Exception("Image missing in 'BIGOBS.PCK' for item '" + _itemRules->getType() + "'");
	}

	index = ModScript::scriptFunc2<ModScript::SelectItemSprite>(_itemRules, index, 0, _battleItem, save, BODYPART_ITEM_INVENTORY, animFrame, 0);

	const auto* newSurf = _bigObsSurfaceSet->getFrame(index);
	if (newSurf == nullptr)
	{
		newSurf = surf;
	}
	return newSurf;
}

void InventoryItemSprite::draw(Surface& target, const SavedBattleGame* saveBattleGame, int animationFrame, int groundOffset)
{
	const Surface* sprite = getSprite(saveBattleGame, animationFrame);

	if (!sprite)
	{
		auto id = std::to_string(_battleItem->getId());
		LogOnce(LOG_WARNING, id) << "Attempted to draw item without sprite. item id: " << id;
		return;
	}

	int invSlotW = _itemRules->getInventoryWidth();
	int invSlotH = _itemRules->getInventoryHeight();
	if (invSlotW == 0 || invSlotH == 0)
	{
		auto id = std::to_string(_battleItem->getId());
		LogOnce(LOG_WARNING, id) << "Attempted to draw item with width: " << invSlotW << " or height: " << invSlotH << " of 0. item id: " << id;
		return;
	}

	auto invetorySlot = _battleItem->getSlot();
	auto slotType = invetorySlot->getType();
	int slotX = invetorySlot->getX();
	int slotY = invetorySlot->getY();
	int itemInvPosOffsetX, itemInvPosOffsetY;

	switch (slotType)
	{
	case INV_SLOT:
		// offset by place in inventory container * slot size.
		itemInvPosOffsetX = _battleItem->getSlotX() * RuleInventory::SLOT_W;
		itemInvPosOffsetY = _battleItem->getSlotY() * RuleInventory::SLOT_H;
		break;
	case INV_HAND:
		// offset by half the difference in item size and hand slot size in order to center.
		itemInvPosOffsetX = (RuleInventory::HAND_W - invSlotW) * RuleInventory::SLOT_W / 2;
		itemInvPosOffsetY = (RuleInventory::HAND_H - invSlotH) * RuleInventory::SLOT_H / 2;
		break;
	case INV_GROUND:
		// offset by place in the ground container, after taking into account the ground offset (in inventory units)
		itemInvPosOffsetX = (_battleItem->getSlotX() - groundOffset) * RuleInventory::SLOT_W;
		itemInvPosOffsetY =  _battleItem->getSlotY() * RuleInventory::SLOT_H;
		break;
	default:
		Log(LOG_ERROR) << "Item in unit inventory with bad enum value: " << slotType << " for slotType.";
		return;
	}

	int itemX = slotX + itemInvPosOffsetX;
	int itemY = slotY + itemInvPosOffsetY;

	_overlay.clear();

	BattleItem::ScriptFill(&_scriptWorker, _battleItem, saveBattleGame, BODYPART_ITEM_INVENTORY, animationFrame, 0);
	_scriptWorker.executeBlit(sprite, &target, itemX, itemY, 0);

	// execute the overlay script and blit.
	_overlay.setPalette(target.getPalette());
	ModScript::scriptCallback<ModScript::InventorySpriteOverlay>(_itemRules, _battleItem, saveBattleGame, &_overlay, animationFrame);

	_overlay.blitNShade(&target, itemX, itemY);

	if (slotType == INV_HAND)
	{
		_handOverlay.setPalette(target.getPalette());
		ModScript::scriptCallback<ModScript::HandOverlay>(_itemRules, _battleItem, saveBattleGame, &_handOverlay, animationFrame);
		_handOverlay.blitNShade(&target, slotX, slotY);
	}
}

}
