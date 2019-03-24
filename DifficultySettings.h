#ifndef DIFFICULTYSETTINGS_H
#define DIFFICULTYSETTINGS_H

#include <string>

namespace DifficultySettings
{

	namespace Map
	{
		static std::string picture;
		static std::string fileName;
	}

	// modifiers are added to the base value
	// example: Player::maxHealthModifier increases (if positive) or decreases (if negative) the player's max health
	namespace Player
	{
		static int maxHealthModifier; //adds to the player's maximum health
		static int eatHealModifier; //adds to the health gained by eating a citizen
		static int eatDrainFreezeDuration; //all passive health drain is stopped for this many frames when eating a citizen
		static float missingHealthHealBonus; //healing is increased based on how much health the player is missing
		static int healthDrainModifier; //adds a flat amount to the passive health drain
		static float highHealthDrainPenalty; //modifies passive health drain based on how high the player's health is
		static int attackHealthCostModifier; //adds to health cost of firing a regular attack
		static int maxPotionNumModifier; //adds to the maximum number of potions
		static int potionMakingCitizenRequired; //determines how many citizens must be eating to spawn a potion
	}

	namespace Mage
	{
		static int attackDamageModifier; //adds to the damage per frame of the mage's attack
		static int touchDamageModifier; //adds to the damage per frame of the mage's melee damage
		static int movementSpeedModifier; //unimplemented
		static int healthDrainModifier; //adds to the passive health drain per mage alive
		static int mageHealthModifier; //adds to the mage's health
		static float blastSpeedModifier; //multiplies the speed of mage attacks
	}

	namespace Citizen
	{
		static int movementSpeedModifier; //unimplemented
	}

	namespace Score
	{
		static float multiplierPerSecond; //a score multiplier which passively increases over time
		static float baseMultiplier; //base score multiplier
		static float cumulativeBonusMultiplier; //score multiplier gained per mage killed
		static float cumulativeBonusMultiplierCurrent; //do not set in difficulty settings. this is always 1.0 when the game starts. (tracks the multiplier)
		static float cumulativeBonusMultiplierMax; //the maximum score multiplier that can be gained by killing mages
		inline int applyMultipliers(int baseIncrease)
		{
			int result = baseIncrease * static_cast<int>(cumulativeBonusMultiplierCurrent * baseMultiplier);
			return (result > 0) ? result : 1;
		}
	}

	enum class DIFFICULTY { TEST, EASY, NORMAL, HARD };

	static DIFFICULTY currentDifficulty = DIFFICULTY::EASY;

	inline void setDifficulty(DIFFICULTY diff)
	{
		Player::maxHealthModifier = 0;
		Player::eatHealModifier = 0;
		Player::eatDrainFreezeDuration = 0;
		Player::missingHealthHealBonus = 0.f;
		Player::healthDrainModifier = 0;
		Player::highHealthDrainPenalty = 0.f;
		Player::attackHealthCostModifier = 0;
		Player::maxPotionNumModifier = 0;
		Player::potionMakingCitizenRequired = 0;
		Mage::attackDamageModifier = 0;
		Mage::touchDamageModifier = 0;
		Mage::movementSpeedModifier = 0;
		Mage::healthDrainModifier = 0;
		Mage::mageHealthModifier = 0;
		Mage::blastSpeedModifier = 0.f;
		Citizen::movementSpeedModifier = 0;
		Score::multiplierPerSecond = 0.f;
		Score::baseMultiplier = 0.f;
		Score::cumulativeBonusMultiplier = 0.f;
		Score::cumulativeBonusMultiplierMax = 0.f;
		Score::cumulativeBonusMultiplierCurrent = 1.0f;
		currentDifficulty = diff;
		switch (diff)
		{
		case DIFFICULTY::TEST:
			Map::picture = "tileset.png";
			Map::fileName = "map_easy.txt";
			Player::missingHealthHealBonus = 0.6f;
			Player::healthDrainModifier = -100000;
			Player::maxHealthModifier = 100000;
			Player::maxPotionNumModifier = 9994;
			Player::eatDrainFreezeDuration = 12;
			Player::potionMakingCitizenRequired = 1;
			Mage::attackDamageModifier = -1000;
			Mage::touchDamageModifier = -1000;
			Mage::healthDrainModifier = -1000;
			Score::baseMultiplier = 0.f;
			Score::cumulativeBonusMultiplier = 0.0f;
			Score::cumulativeBonusMultiplierMax = 0.0f;
			Score::multiplierPerSecond = 0.0f;
			break;
		case DIFFICULTY::EASY:
			Map::picture = "tileset.png";
			Map::fileName = "map_easy.txt";
			Player::missingHealthHealBonus = 0.7f;
			Player::highHealthDrainPenalty = 0.8f;
			Player::eatDrainFreezeDuration = 15;
			Player::potionMakingCitizenRequired = 3;
			Score::baseMultiplier = 1.0f;
			Score::cumulativeBonusMultiplier = 0.02f;
			Score::cumulativeBonusMultiplierMax = 1.10f;
			Score::multiplierPerSecond = -0.001f;
			break;
		case DIFFICULTY::NORMAL:
			Map::picture = "tileset.png";
			Map::fileName = "map_normal.txt";
			Player::missingHealthHealBonus = 0.3f;
			Player::healthDrainModifier = 1;
			Player::highHealthDrainPenalty = 1.1f;
			Player::eatDrainFreezeDuration = 11;
			Player::maxHealthModifier = -4500;
			Player::attackHealthCostModifier = 150;
			Player::potionMakingCitizenRequired = 4;
			Player::maxPotionNumModifier = -1;
			Mage::attackDamageModifier = 100;
			Mage::blastSpeedModifier = 0.8f;
			Mage::touchDamageModifier = 10;
			Mage::movementSpeedModifier = 1;
			Mage::healthDrainModifier = 1;
			Mage::mageHealthModifier = 2;
			Citizen::movementSpeedModifier = 1;
			Score::baseMultiplier = 1.5f;
			Score::cumulativeBonusMultiplier = 0.03f;
			Score::cumulativeBonusMultiplierMax = 1.29f;
			Score::multiplierPerSecond = 0.0005f;
			break;
		case DIFFICULTY::HARD:
			Map::picture = "tileset.png";
			Map::fileName = "map_insane.txt";
			Player::missingHealthHealBonus = 0.15f;
			Player::healthDrainModifier = 1;
			Player::highHealthDrainPenalty = 4.5f;
			Player::eatDrainFreezeDuration = 10;
			Player::maxHealthModifier = -6000;
			Player::attackHealthCostModifier = 350;
			Player::potionMakingCitizenRequired = 5;
			Player::maxPotionNumModifier = -2;
			Mage::attackDamageModifier = 250;
			Mage::blastSpeedModifier = 1.6f;
			Mage::touchDamageModifier = 50;
			Mage::movementSpeedModifier = 2;
			Mage::healthDrainModifier = 1;
			Mage::mageHealthModifier = 7;
			Citizen::movementSpeedModifier = 1;
			Score::baseMultiplier = 2.5f;
			Score::cumulativeBonusMultiplier = 0.02f;
			Score::cumulativeBonusMultiplierMax = 10.f;
			Score::multiplierPerSecond = 0.006f;
			break;
		default:
			break;
		}
	}
}

#endif
