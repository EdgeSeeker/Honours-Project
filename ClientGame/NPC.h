#pragma once
#include "stdafx.h"
#include "AIGoal_Think.h"

class NPC
{
private:
	std::string name;

	std::unique_ptr<AIGoal_Think> aiBrain;
	std::shared_ptr<Inventory> inventory;
	std::weak_ptr<City> currentCity;

	bool aiControlled;

	float baseMoveSpeed, calculatedMoveSpeed;
	void LoadDefaults();
	void InitializeInventory(const int startingBagNumber, const int startingGoldNumber);

	virtual void CalculateMoveSpeed();

public:
	NPC();
	~NPC();

	void Update();

	void SetAIControlled(const bool _aiControlled) { aiControlled = _aiControlled; }
	bool GetAIControlled() { return aiControlled; }

	std::weak_ptr<Inventory> GetInventory() { return inventory; }
	std::weak_ptr<City> GetCurrentCityPtr() { return currentCity; }

	void GenerateName();
	void GenerateStartingCity();

	void SetCity(const int cityIterator) { currentCity = theMap.GetCityContainerRef().get().at(cityIterator); }
	void SetCity(const std::weak_ptr<City> destination);
	void SetStartingCity(const std::string cityName);

	std::string GetName() { return name; }
	std::string GetCityName() { return currentCity.lock()->GetName(); }

	float GetMoveSpeed()const;
};
