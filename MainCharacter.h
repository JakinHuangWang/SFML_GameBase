#ifndef MAINCHARACTER_H
#define MAINCHARACTER_H

#include "GameOver.h"
#include "Score.h"
#include "Screen.h"
#include "ZombieBlast.h"
#include "Citizen.h"
#include "Mage.h"
#include "DifficultySettings.h"
#include "MageBlast.h"
#include "AntiMagePotion.h"
#include "FileLoadException.h"
#include <ctime>
#include <vector>

using namespace Engine;

class MainCharacter : public GraphicalGameObject
{
	bool wKeyHeld = false;
	bool aKeyHeld = false;
	bool sKeyHeld = false;
	bool dKeyHeld = false;
	DIRECTION direction = DIRECTION::DOWN;
	AntiMagePotion* potionPtr = nullptr;
	sf::Vector2u textureSize;
	sf::Vector2u imageCount;
	sf::Vector2u currentImage;
	sf::Clock aliveClock;
	sf::Clock hurtClock;
	sf::Clock trapClock;
	float totalAliveTime = 0.f;
	bool inTrap = false;
	bool isHurt = false;
	bool startDeath = false;
	bool isDead = false; // true when the zombie turns to invisible
	int deathCount = 0; // to control death animation
	int potionNum = 0;
	int maxPotionNum = 6;
	int health = 30 * 60 * 100;
	int maxHealth = 30 * 60 * 100;
	int healthDrain = 15;
	int additionalDrainPerMage = 1;
	int numCitizenEated = 0;
	int eatHeal = 7000;
	int eatDrainFreezeCountdown = 0;
	int attackHealthCost = 250;
	int baseSpeed = 3;
	int speed = 3;
	int maxSpeed = 4;
	int speedDecayDelay = 0;
	int speedRestoreDelay = 0;
	int colorRestoreDelay = 0;
	std::vector<sf::Vector2f> spawnPositions;
	std::string name;
public:
	MainCharacter(std::string name) : GraphicalGameObject(sf::Sprite())
	{
		sf::Texture* texturePtr = ResourceManager<sf::Texture>::GetResource("zombie.png");
		this->sprite()->setTexture(*texturePtr);
		this->name = name;
		this->textureSize = this->sprite()->getTexture()->getSize();
		this->textureSize.x /= 4;
		this->textureSize.y /= 12;
		this->imageCount.x = 0;
		this->sprite()->setTextureRect(sf::IntRect(this->imageCount.x * this->textureSize.x,
			this->imageCount.y * this->textureSize.y, this->textureSize.x, this->textureSize.y));
		sf::IntRect size = this->sprite()->getTextureRect();
		sf::Vector2f collisionSizeRatio(0.4f, 0.3f); //these numbers shrink the collision size of the player, and the code below adjusts it to be positioned at the bottom of the sprite
		this->obstacleCollisionSize.width = static_cast<float>(size.width) * collisionSizeRatio.x;
		this->obstacleCollisionSize.height = static_cast<float>(size.height) * collisionSizeRatio.y;
		this->obstacleCollisionSize.left = ((1.f - collisionSizeRatio.x) * static_cast<float>(size.width)) / 2.f;
		this->obstacleCollisionSize.top = ((1.f - collisionSizeRatio.y) * static_cast<float>(size.height));
		this->aliveClock.restart();

		//difficulty adjustments
		this->maxHealth += DifficultySettings::Player::maxHealthModifier;
		this->eatHeal += DifficultySettings::Player::eatHealModifier;
		this->healthDrain += DifficultySettings::Player::healthDrainModifier;
		this->attackHealthCost += DifficultySettings::Player::attackHealthCostModifier;
		this->maxPotionNum += DifficultySettings::Player::maxPotionNumModifier;
	}
	void KeyPressed(sf::Event e)
	{
		if (!this->startDeath)
		{
			switch (e.key.code)
			{
			case sf::Keyboard::W:
				this->wKeyHeld = true;
				break;
			case sf::Keyboard::A:
				this->aKeyHeld = true;
				break;
			case sf::Keyboard::S:
				this->sKeyHeld = true;
				break;
			case sf::Keyboard::D:
				this->dKeyHeld = true;
				break;
			case sf::Keyboard::Escape:
				if (DifficultySettings::currentDifficulty == DifficultySettings::DIFFICULTY::TEST)
				{
					this->healthDrain = 10000;
					DifficultySettings::Mage::healthDrainModifier = 0;
				}
			default:
				break;
			}
		}
	}

	void KeyReleased(sf::Event e)
	{
		if (!this->startDeath)
		{
			switch (e.key.code)
			{
			case sf::Keyboard::W:
				this->wKeyHeld = false;
				this->setDirection(DIRECTION::UP);
				break;
			case sf::Keyboard::A:
				this->aKeyHeld = false;
				this->setDirection(DIRECTION::LEFT);
				break;
			case sf::Keyboard::S:
				this->sKeyHeld = false;
				this->setDirection(DIRECTION::DOWN);
				break;
			case sf::Keyboard::D:
				this->dKeyHeld = false;
				this->setDirection(DIRECTION::RIGHT);
				break;
			default:
				break;
			}
		}
	}

	void MouseButtonReleased(sf::Event e)
	{
		if (this->health > 0)
		{
			if (e.mouseButton.button == sf::Mouse::Left)
			{
				soundPlayer.play(SoundEffect::ID::ZombieAttack, 40.f);
				if (this->health > 0.2 * this->maxHealth) { this->changeHealth(-1 * this->attackHealthCost); } //health cost of ranged attack only applies if health is above 20%
				sf::Vector2i mousePos = this->screen->getMousePosition();
				sf::Vector2f shotOrigin = this->sprite()->getPosition();
				sf::IntRect size = this->sprite()->getTextureRect();
				shotOrigin.x += static_cast<float>(size.width / 2);
				shotOrigin.y += static_cast<float>(size.height / 4);
				ZombieBlast* blast = new ZombieBlast("blast.png", shotOrigin, sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)), 3.5f, 140);
				this->screen->add(blast);
			}
			else if (this->potionNum > 0)
			{
				soundPlayer.play(SoundEffect::ID::ZombieAttack, 40.f);
				sf::Vector2i mousePos = this->screen->getMousePosition();
				sf::Vector2f shotOrigin = this->sprite()->getPosition();
				sf::IntRect size = this->sprite()->getTextureRect();
				shotOrigin.x += static_cast<float>(size.width / 2);
				shotOrigin.y += static_cast<float>(size.height / 4);
				SuperZombieBlast* blast = new SuperZombieBlast("brain.png", shotOrigin, sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)), 2.25f, 180, 1000, 0.1f, 0.2f);
				this->screen->add(blast);
				this->potionNum--;
			}
		}
	}

	void EveryFrame(uint64_t f)
	{
		sf::Sprite* s = this->sprite();
		sf::Vector2f adjustPos;
		adjustPos.x = s->getPosition().x + this->textureSize.x / 2;
		adjustPos.y = s->getPosition().y + this->textureSize.y;
		if (this->screen->getMap()->isTrap(adjustPos))
		{
			if (!this->inTrap)
			{
				this->inTrap = true;
				this->trapClock.restart();
				soundPlayer.play(SoundEffect::ID::Trap, 20.f);
			}
			this->speed = 1;
		}
		if (this->trapClock.getElapsedTime().asSeconds() > 2) { this->inTrap = false; }

		if (this->health > 0)
		{
			float fSpeed = static_cast<float>(this->speed);
			if (f % 20 == 0 && (this->wKeyHeld || this->aKeyHeld || this->sKeyHeld || this->dKeyHeld))
			{
				if (this->imageCount.x == 3) { this->imageCount.x = 0; }
				else { this->imageCount.x++; }
			}
			if (!this->wKeyHeld && !this->aKeyHeld && !this->sKeyHeld && !this->dKeyHeld) { this->imageCount.x = 0; }

			if (this->wKeyHeld)
			{
				this->imageCount.y = 3;
				s->move(0.f, -1.f * fSpeed);
			}
			if (this->aKeyHeld)
			{
				this->imageCount.y = 1;
				s->move(-1.f * fSpeed, 0.f);
			}
			if (this->sKeyHeld)
			{
				this->imageCount.y = 0;
				s->move(0.f, fSpeed);
			}
			if (this->dKeyHeld)
			{
				this->imageCount.y = 2;
				s->move(fSpeed, 0.f);
			}
			this->drain();
			if (f % 120 == 0) { *scorePtr += DifficultySettings::Score::applyMultipliers(1); }

			//speed goes back to base speed gradually
			if (f % 6 == 0)
			{
				if (this->speed > this->baseSpeed)
				{
					this->speedRestoreDelay = 0;
					if (this->speedDecayDelay > 0) { this->speedDecayDelay--; }
					else { this->changeSpeed(-1); }
				}
				else if (this->speed < this->baseSpeed)
				{
					this->speedDecayDelay = 0;
					if (this->speedRestoreDelay > 0) { this->speedRestoreDelay--; }
					else { this->changeSpeed(1); }
				}
			}

			//color goes back to base
			if (!this->isDead)
			{
				if (this->colorRestoreDelay > 0) { this->colorRestoreDelay--; }
				else { this->sprite()->setColor({ 255, 255, 255 }); }
			}
		}
		else
		{
			this->sprite()->setColor({ 255, 100, 100 });
			if (!this->startDeath)
			{
				this->totalAliveTime = this->aliveClock.getElapsedTime().asSeconds();
				this->startDeath = true;
				soundPlayer.play(SoundEffect::ID::ZombieDeath, 60.f);
			}
			this->imageCount.x = this->deathCount;
			if (f % 50 == 0 && !this->isDead)
			{
				this->deathCount++;
				if (deathCount == 2)
				{
					musicPlayer.stop();
					musicPlayer.play(Music::ID::GameOver);
					musicPlayer.setVolume(20.f);
				}
			}
			if (this->direction == DIRECTION::DOWN || this->sKeyHeld) { this->imageCount.y = 4; }
			if (this->direction == DIRECTION::LEFT || this->aKeyHeld) { this->imageCount.y = 5; }
			if (this->direction == DIRECTION::RIGHT || this->dKeyHeld) { this->imageCount.y = 6; }
			if (this->direction == DIRECTION::UP || this->wKeyHeld) { this->imageCount.y = 7; }
			if (this->imageCount.x == 3) { this->die(); }
		}

		if (!this->isDead)
		{
			this->sprite()->setTextureRect(sf::IntRect(this->imageCount.x * this->textureSize.x, this->imageCount.y * this->textureSize.y, this->textureSize.x, this->textureSize.y));
		}
	}

	void drain()
	{
		if (this->eatDrainFreezeCountdown > 0)
		{
			this->eatDrainFreezeCountdown--;
			return;
		}

		float time = this->aliveClock.getElapsedTime().asSeconds();
		float highHealthDrainPenalty = DifficultySettings::Player::highHealthDrainPenalty + (time * (0.02f + DifficultySettings::Player::highHealthDrainPenalty * 0.01f));
		float healthRatio = static_cast<float>(this->health) / static_cast<float>(this->maxHealth);
		if (healthRatio > 1.f) { healthRatio = 1.f; }
		float drainPenaltyModifier = (healthRatio + (healthRatio * healthRatio)) * 0.5f;
		float drainPenalty = 1.f + (highHealthDrainPenalty * drainPenaltyModifier);
		int baseDrain = static_cast<int>(static_cast<float>(this->healthDrain) * drainPenalty) + static_cast<int>(time/15.f);
		int mageDrain = numMagesAlive * (this->additionalDrainPerMage + DifficultySettings::Mage::healthDrainModifier);
		int totalDrain = baseDrain + mageDrain;
		this->changeHealth(-1 * totalDrain);
	}

	void die()
	{
		if (!this->isDead)
		{
			this->sprite()->setColor({ 0, 0, 0, 0 });
			this->isDead = true;
			scorePtr->freeze();
			this->screen->addUIObject(new GameOver(scorePtr->get(), DifficultySettings::currentDifficulty));
		}
	}

	std::string getName() const
	{
		return this->name;
	}

	int getHealth() const
	{
		return this->health;
	}

	int getMaxHealth() const
	{
		return this->maxHealth;
	}

	float getCurrAliveTime() const
	{
		return this->aliveClock.getElapsedTime().asSeconds();
	}

	float getTotalAliveTime() const
	{
		return this->totalAliveTime;
	}

	int getNumCitizenEated() const
	{
		return this->numCitizenEated;
	}

	bool isAlive() const
	{
		return !this->startDeath;
	}

	void setDirection(DIRECTION direction)
	{
		this->direction = direction;
	}

	void takeDamage(int damage)
	{
		this->changeHealth(-1 * damage);
		this->sprite()->setColor({ 255, 100, 100 });
		this->colorRestoreDelay = 2;
	}

	void changeHealth(int change)
	{
		this->health += change;
		if (this->health > this->maxHealth) { this->health = this->maxHealth; }
	}

	void addPotionNum()
	{
		this->potionNum++;
		if (this->potionNum > this->maxPotionNum) { this->potionNum = this->maxPotionNum; }
	}

	int getPotionNum() const
	{
		return this->potionNum;
	}

	int getMaxPotionNum() const
	{
		return this->maxPotionNum;
	}

	void changeSpeed(int change)
	{
		this->speed += change;
		if (this->speed > this->maxSpeed) { this->speed = this->maxSpeed; }
		else if (this->speed < 0) { this->speed = 0; }
	}

	void Collision(GraphicalGameObject& other)
	{
		if (this->health > 0)
		{
			if (MageBlast* blast = dynamic_cast<MageBlast*>(&other))
			{
				if (!this->isHurt)
				{
					this->isHurt = true;
					this->hurtClock.restart();
					soundPlayer.play(SoundEffect::ID::ZombieGroan, 15.f);
				}
				else if (this->hurtClock.getElapsedTime().asSeconds() > 0.5) { this->isHurt = false; }
				float repeatDamageDampening = (1.f + 0.01f*(static_cast<float>(blast->getHits())));
				if (repeatDamageDampening < 0.5f) { repeatDamageDampening = 0.5f; }
				int baseDmg = 250;
				int damage = static_cast<int>(static_cast<float>(250 + DifficultySettings::Mage::attackDamageModifier) / repeatDamageDampening);
				int minDmg = 18;
				if (damage < minDmg && DifficultySettings::currentDifficulty != DifficultySettings::DIFFICULTY::TEST) { damage = minDmg; }
				this->takeDamage(damage);
				blast->hitPlayer();
			}
			else if (Mage* mage = dynamic_cast<Mage*>(&other))
			{
				if (!this->isHurt)
				{
					this->isHurt = true;
					this->hurtClock.restart();
					soundPlayer.play(SoundEffect::ID::ZombieGroan, 10.f);
				}
				else if (this->hurtClock.getElapsedTime().asSeconds() > 0.5) { this->isHurt = false; }
				if (!mage->isAlive()) { return; }
				this->takeDamage(100 + DifficultySettings::Mage::touchDamageModifier);
				this->speed = 1;
			}
			else if (Citizen* citizen = dynamic_cast<Citizen*>(&other))
			{
				srand(static_cast<int>(time(0) * other.getID()));
				int randSound = rand() % 3;
				switch (randSound)
				{
				case 0:
					soundPlayer.play(SoundEffect::ID::ZombieEat1, 80.f);
					break;
				case 1:
					soundPlayer.play(SoundEffect::ID::ZombieEat2, 80.f);
					break;
				case 2:
					soundPlayer.play(SoundEffect::ID::ZombieEat3, 50.f);
					break;
				default:
					break;
				}
				citizen->die();
				this->numCitizenEated++;
				if (this->numCitizenEated % DifficultySettings::Player::potionMakingCitizenRequired == 0)
				{
					srand(static_cast<int>(time(0) << 7));
					switch (rand() % 4)
					{
					case 0:
						soundPlayer.play(SoundEffect::ID::ZombieBurp1, 70.f);
						break;
					case 1:
						soundPlayer.play(SoundEffect::ID::ZombieBurp2, 70.f);
						break;
					case 2:
						soundPlayer.play(SoundEffect::ID::ZombieBurp3, 50.f);
						break;
					case 3:
						soundPlayer.play(SoundEffect::ID::ZombieBurp4, 50.f);
						break;
					}
					this->spawnPositions = this->screen->getMap()->getSafeSpawnPositions();
					size_t randIndex = rand() % spawnPositions.size();
					sf::Vector2f position = spawnPositions[randIndex];
					potionPtr = new AntiMagePotion();
					potionPtr->spritePtr()->setPosition(position);
					this->screen->add(potionPtr);
				}
				float missingHealthBonus = DifficultySettings::Player::missingHealthHealBonus;
				float missingHealthMultiplier = (1.0f + missingHealthBonus) - (missingHealthBonus * (static_cast<float>(this->health) / static_cast<float>(this->maxHealth)));
				this->changeHealth(static_cast<int>(static_cast<float>(this->eatHeal) * missingHealthMultiplier));
				this->changeSpeed(1);
				this->speedDecayDelay = 60;
				*scorePtr += DifficultySettings::Score::applyMultipliers(10);
				this->eatDrainFreezeCountdown = DifficultySettings::Player::eatDrainFreezeDuration;
			}
			else if (AntiMagePotion* potion = dynamic_cast<AntiMagePotion*>(&other))
			{
				soundPlayer.play(SoundEffect::ID::Potion, 40.f);
				this->addPotionNum();
				this->changeHealth(this->eatHeal / 2);
				this->eatDrainFreezeCountdown = DifficultySettings::Player::eatDrainFreezeDuration * 3;
				potion->die();
			}
		}
	}

	void changeScore(int change)
	{
		*scorePtr += change;
	}

	sf::Sprite* sprite()
	{
		return dynamic_cast<sf::Sprite*>(this->graphic);
	}
};

#endif
