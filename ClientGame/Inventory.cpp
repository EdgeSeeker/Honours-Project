#include "stdafx.h"
#include "Inventory.h"

namespace
{
	const auto purseStartingLimitFallBack = 10000;
	const auto numPerLineFallBack = 2;
	const auto maxBagNumberFallBack = 5;
	const auto bagPurchaseCostFallBack = 5000;
}

Inventory::Inventory(const bool isCity /* = false */, int startingBagNumber /* = 1 */, int StartingGold /* = 0 */)
{
	city = isCity;

	lowestBagUpgradePrice = 0;

	if (StartingGold < 0)
		StartingGold = 0;
	purse.reset(new InventoryPurse(StartingGold));

	if (startingBagNumber <= 0)
		startingBagNumber = 1;
	AddBag(startingBagNumber);

	LoadDefauts();
}

Inventory::~Inventory()
{
}

void Inventory::LoadDefauts()
{
	auto purseStartingLimit = thePrefs.GetInt("InventorySettings", "startingGoldLimit");
	if (!purseStartingLimit)
		purseStartingLimit = purseStartingLimitFallBack;

	numPerLine = thePrefs.GetInt("InventorySettings", "numPerLine");
	if (!numPerLine)
		numPerLine = numPerLineFallBack;

	maxBagNumber = thePrefs.GetInt("InventorySettings", "maxBagNumber");
	if (!maxBagNumber)
		maxBagNumber = maxBagNumberFallBack;

	bagPurchaseCost = thePrefs.GetInt("InventorySettings", "bagPurchaseCost");
	if (!bagPurchaseCost)
		bagPurchaseCost = bagPurchaseCostFallBack;
	purse->SetGoldLimit(purseStartingLimit);
}

void Inventory::AddBag(const int numBag /*= 1*/)
{
	for (int i = 0; i < numBag; i++)
	{
		std::shared_ptr<InventoryBag> newBag;
		newBag.reset(new InventoryBag(city));

		bagContainer.push_back(newBag);
	}
}

unsigned int Inventory::GetBagUpgradeCost()
{
	unsigned int bagPrice = 0;
	lowestBagUpgradePrice = bagContainer.at(0)->GetBagUpradeCost();
	lowestUpgradePriceBagPtr = bagContainer.at(0);

	for (std::weak_ptr<InventoryBag> bagPtr : bagContainer)
	{
		bagPrice = bagPtr.lock()->GetBagUpradeCost();
		if ((bagPrice < lowestBagUpgradePrice) || (bagPrice == 0))
		{
			lowestBagUpgradePrice = bagPrice;
			lowestUpgradePriceBagPtr = bagPtr;
		}
	}

	return lowestBagUpgradePrice;
}

bool Inventory::GetBagUpgradeAvailable()
{
	for (std::weak_ptr<InventoryBag> bagPtr : bagContainer)
	{
		if (bagPtr.lock()->CheckBagUpgradeable())
			return true;
	}

	return false;
}

void Inventory::UpgradeBag()
{
	// Upgrade bag.
	lowestUpgradePriceBagPtr.lock()->UpgradeBag();
}

void Inventory::UpgradePurse()
{
		purse->Upgrade();
}

void Inventory::AdjustResource(const std::string resourceName, const int quantity)
{
	// Check for a bag with available space, once found, add that resource quantity to it.
	for (auto i : bagContainer)
	{
		if (i->CheckBagSpace(quantity))
		{
			i->AdjustResource(resourceName, quantity);
			break;
		}
	}
}

void Inventory::SetResource(const std::string resourceName, const int quantity)
{
	auto quantityRemaining = quantity;

	// Zero the resource.
	for (auto i : bagContainer)
		i->SetResource(resourceName, 0);

	for (auto i : bagContainer)
	{
		auto available = i->GetAvailableBagSpace();
		if (available >= quantityRemaining)
			i->SetResource(resourceName, quantityRemaining);
		else
		{
			// Add as much as possible and move to next bag.
			i->SetResource(resourceName, available);
			quantityRemaining -= available;
		}
	}
}

void Inventory::SetResource(const int bagNumber, const int iter, const int quantity)
{
	if (static_cast<unsigned int>(bagNumber) <= bagContainer.size())
		bagContainer.at(bagNumber)->SetResource(iter, quantity);
	else
		sysLog.Log("ERROR: SetResource command used with an invalid bag number.");
}

void Inventory::AdjustResource(const int bagNumber, const int iter, const int quantity)
{
	if (static_cast<unsigned int>(bagNumber) <= bagContainer.size())
		bagContainer.at(bagNumber)->AdjustResource(iter, quantity);
	else
		sysLog.Log("ERROR: AddToResource command used with an invalid bag number.");
}

int Inventory::GetResourceInBag(const int bagNumber, const int iter)
{
	if (static_cast<unsigned int>(bagNumber) <= bagContainer.size())
		return bagContainer.at(bagNumber)->GetResourceQuantity(iter);
	else
		sysLog.Log("ERROR: GetResourceInBag command used with an invalid bag number.");

	return 0;
}

int Inventory::GetTotalResourceAmount(const int iter)
{
	auto total = 0;

	for (auto i : bagContainer)
		total += i->GetResourceQuantity(iter);

	return total;
}

int Inventory::GetTotalResourceAmount(const std::string resourceName)
{
	auto total = 0;

	for (auto i : bagContainer)
		total += i->GetResourceQuantity(theResourceManager.GetResourceIterator(resourceName));

	return total;
}

bool Inventory::GetEmpty()
{
	for (int i = 0; i < theResourceManager.GetResourceNames().size(); i++)
	{
		if (GetTotalResourceAmount(i))
			return false;
	}
	return true;
}

std::map<std::string, int> Inventory::GetAllResourcesFiltered()
{
	std::map<std::string, int> output;
	auto resourceNames = theResourceManager.GetResourceNames();

	for (int i = 0; i < resourceNames.size(); i++)
	{
		if (GetTotalResourceAmount(i) > 0)
			output.emplace(std::pair<std::string, int>(resourceNames.at(i), GetTotalResourceAmount(i)));
	}

	return output;
}

int Inventory::GetAvailableSpace() const
{
	auto emptySpace = 0;

	for (auto bag : bagContainer)
		emptySpace += bag->GetAvailableBagSpace();

	return emptySpace;
}

std::string Inventory::GetInventoryString()
{
	std::string inventoryString = "";

	inventoryString += "Gold: " + IntToString(purse->GetGold()) + " / " + IntToString(purse->GetGoldLimit()) + " \t " 
		+ "Space: " + IntToString(GetAvailableSpace()) + "\n";

	// Assemble the string.
	for (int i = 0; i < theResourceManager.GetTotalResources(); i++)
	{
		auto numResource = GetTotalResourceAmount(i);
		inventoryString += theResourceManager.GetResourceNames().at(i) + ": " + IntToString(numResource);

		if (i % numPerLine)
			inventoryString += "\n";
		else
			inventoryString += " \t ";
	}

	return inventoryString;
}