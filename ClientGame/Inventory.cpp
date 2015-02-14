#include "stdafx.h"
#include "Inventory.h"

Inventory::Inventory(bool isCity /* = false */, int startingBagNumber /* = 1 */)
{
	city = isCity;

	AddBag(startingBagNumber);
}

Inventory::~Inventory()
{
}

void Inventory::AddBag(int numBag /*= 1*/)
{
	for (int i = 0; i < numBag; i++)
	{
		std::shared_ptr<InventoryBag> newBag;
		newBag.reset(new InventoryBag(city));

		bagContainer.push_back(newBag);
	}
}

void Inventory::AddToResource(std::string resourceName, int quantity)
{
	// Check for a bag with available space, once found, add that resource quantity to it.
	for (auto i : bagContainer)
	{
		if (i->CheckBagSpace(quantity))
		{
			i->AddResource(resourceName, quantity);
			break;
		}
	}
}

void Inventory::SetResource(std::string resourceName, int quantity)
{
	// Zero the resource.
	for (auto i : bagContainer)
		i->SetResource(resourceName, 0);

	for (auto i : bagContainer)
	{
		auto available = i->GetAvailableBagSpace();
		if (available >= quantity)
			i->SetResource(resourceName, quantity);
		else
		{
			// Add as much as possible and move to next bag.
			i->SetResource(resourceName, available);
			quantity -= available;
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

void Inventory::AddToResource(const int bagNumber, const int iter, const int quantity)
{
	if (static_cast<unsigned int>(bagNumber) <= bagContainer.size())
		bagContainer.at(bagNumber)->AddResource(iter, quantity);
	else
		sysLog.Log("ERROR: AddToResource command used with an invalid bag number.");
}

void Inventory::SubtractFromResource(const int bagNumber, const int iter, const int quantity)
{
	if (static_cast<unsigned int>(bagNumber) <= bagContainer.size())
		bagContainer.at(bagNumber)->SubtractResource(iter, quantity);
	else
		sysLog.Log("ERROR: SubtractFromResource command used with an invalid bag number.");
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