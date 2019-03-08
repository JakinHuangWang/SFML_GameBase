#ifndef MAGE_HEADER
#define MAGE_HEADER

#include <ctime>
#include <cmath>
#include "GameObject.h"
#include "RespawnManager.h"
#include "ZombieBlast.h"
#include "MageBlast.h"
#include "Screen.h"
#include "DifficultySettings.h"
#include "Score.h"

template<typename T> class RespawnManager;

static int numMagesAlive;

class Mage : public Engine::GraphicalGameObject
{
private:
	friend class RespawnManager<Mage>;
	bool W_KeyHeld = false;
	bool A_KeyHeld = false;
	bool S_KeyHeld = false;
	bool D_KeyHeld = false;
	bool alive;
	int deathCount;
	bool isShooting;
	uint64_t internalClock = 0;
	sf::Vector2f bullet_position;
	int bullet_cooldown;
	sf::Vector2u textureSize;
	sf::Vector2u imageCount;
	sf::Vector2u currentImage;
	RespawnManager<Mage>* respawnManager = nullptr;
public:
	Mage(sf::Sprite s, RespawnManager<Mage>* respawnManager) : Mage(s)
	{
		this->respawnManager = respawnManager;
	}

	Mage(sf::Sprite s) : Engine::GraphicalGameObject(s)
	{
		textureSize = this->spritePtr()->getTexture()->getSize();
		textureSize.x /= 4;
		textureSize.y /= 12;
		imageCount.x = 0;
		alive = true;
		deathCount = 0;
		isShooting = false;
		bullet_cooldown = 0;
		switch (rand() % 4)
		{
		case 0:
			W_KeyHeld = true;
			A_KeyHeld = false;
			S_KeyHeld = false;
			D_KeyHeld = false;
			break;
		case 1:
			A_KeyHeld = true;
			W_KeyHeld = false;
			S_KeyHeld = false;
			D_KeyHeld = false;
			break;
		case 2:
			S_KeyHeld = true;
			A_KeyHeld = false;
			W_KeyHeld = false;
			D_KeyHeld = false;
			break;
		case 3:
			D_KeyHeld = true;
			A_KeyHeld = false;
			S_KeyHeld = false;
			W_KeyHeld = false;
			break;
		}
		numMagesAlive++;
	}
	void EveryFrame(uint64_t f)
	{
		internalClock++;
		srand(static_cast<unsigned int>(time(0) * this->getID()));
		if (alive)
		{
			if (internalClock % 120 == 0)
			{
				GraphicalGameObject* player = dynamic_cast<GraphicalGameObject*>(this->screen->getMainCharacter());
				sf::Vector2f playerPosition = dynamic_cast<sf::Transformable*>(player->getGraphic())->getPosition();
				sf::Vector2f myPosition = this->spritePtr()->getPosition();
				DIRECTION xDirection = (playerPosition.x > myPosition.x) ? DIRECTION::RIGHT : DIRECTION::LEFT;
				DIRECTION yDirection = (playerPosition.y > myPosition.y) ? DIRECTION::DOWN : DIRECTION::UP;
				float a = (playerPosition.x - myPosition.x);
				float b = (playerPosition.y - myPosition.y);
				float distance = sqrt(a*a + b * b);

				this->W_KeyHeld = false;
				this->A_KeyHeld = false;
				this->S_KeyHeld = false;
				this->D_KeyHeld = false;

				int choice = rand() % 2;
				if (choice == 0)
				{
					if (xDirection == DIRECTION::RIGHT) { this->D_KeyHeld = true; }
					else { this->A_KeyHeld = true; }
				}
				else
				{
					if (yDirection == DIRECTION::DOWN) { this->S_KeyHeld = true; }
					else { this->W_KeyHeld = true; }
				}
			}
			if (this->W_KeyHeld)
			{
				if (!isShooting)
					imageCount.y = 3;
				this->spritePtr()->move(0, -0.5);
				this->spritePtr()->setTextureRect(sf::IntRect(imageCount.x * textureSize.x,
					imageCount.y * textureSize.y, textureSize.x, textureSize.y));
				if (internalClock % 100 == 0 && !isShooting)
				{
					imageCount.y = 7;
					isShooting = true;
					bullet_cooldown = 0;
					bullet_position = this->spritePtr()->getPosition();
					bullet_position.x += textureSize.x / 4;
					//Bullet* bullet = new Bullet(bullet_position, DIRECTION::UP);
					//this->screen->add(bullet);
				}

			}
			if (this->A_KeyHeld)
			{
				if (!isShooting)
					imageCount.y = 1;
				this->spritePtr()->move(-0.5, 0);
				this->spritePtr()->setTextureRect(sf::IntRect(imageCount.x * textureSize.x,
					imageCount.y * textureSize.y, textureSize.x, textureSize.y));
				if (internalClock % 100 == 0 && !isShooting)
				{
					imageCount.y = 5;
					isShooting = true;
					bullet_cooldown = 0;
					bullet_position = this->spritePtr()->getPosition();
					bullet_position.y += textureSize.y / 4;
					//Bullet* bullet = new Bullet(bullet_position, DIRECTION::LEFT);
					//this->screen->add(bullet);
				}
			}
			if (this->S_KeyHeld)
			{
				if (!isShooting)
					imageCount.y = 0;
				this->spritePtr()->move(0, 0.5);
				this->spritePtr()->setTextureRect(sf::IntRect(imageCount.x * textureSize.x,
					imageCount.y * textureSize.y, textureSize.x, textureSize.y));
				if (internalClock % 100 == 0 && !isShooting)
				{
					imageCount.y = 4;
					isShooting = true;
					bullet_cooldown = 0;
					bullet_position = this->spritePtr()->getPosition();
					bullet_position.x += textureSize.x / 4;
					bullet_position.y += textureSize.y;
					//Bullet* bullet = new Bullet(bullet_position, DIRECTION::DOWN);
					//this->screen->add(bullet);
				}
			}
			if (this->D_KeyHeld)
			{
				// row 3
				if (!isShooting)
					imageCount.y = 2;
				this->spritePtr()->move(0.5, 0);
				this->spritePtr()->setTextureRect(sf::IntRect(imageCount.x * textureSize.x,
					imageCount.y * textureSize.y, textureSize.x, textureSize.y));
				if (internalClock % 100 == 0 && !isShooting)
				{
					imageCount.y = 6;
					isShooting = true;
					bullet_cooldown = 0;
					bullet_position = this->spritePtr()->getPosition();
					bullet_position.x += textureSize.x;
					bullet_position.y += textureSize.y / 4;
					//Bullet* bullet = new Bullet(bullet_position, DIRECTION::RIGHT);
					//this->screen->add(bullet);
				}
			}


			if (this->internalClock % 100 == 0)
			{
				sf::Vector2f pos = this->spritePtr()->getPosition();
				sf::Vector2f playerPos = dynamic_cast<sf::Transformable*>(dynamic_cast<GraphicalGameObject*>(this->screen->getMainCharacter())->getGraphic())->getPosition();
				MageBlast* blast = new MageBlast(pos, playerPos, 2.5 + static_cast<double>(DifficultySettings::Mage::blastSpeedModifier), 130);
				this->screen->add(blast);
			}

			// shooting delay
			bullet_cooldown++;
			if (bullet_cooldown == 50)
			{
				isShooting = false;
			}
			// how often the sprite sheet is changing
			if (internalClock % 15 == 0)
			{
				if (imageCount.x == 3)
					imageCount.x = 0;
				else
					imageCount.x++;
			}
		}
		else
		{
			if (this->W_KeyHeld)
			{
				imageCount.y = 11;
			}
			if (this->A_KeyHeld)
				imageCount.y = 9;
			if (this->S_KeyHeld)
				imageCount.y = 8;
			if (this->D_KeyHeld)
				imageCount.y = 10;
			imageCount.x = deathCount;
			if (internalClock % 30 == 0)
			{
				deathCount++;
			}
			this->spritePtr()->setTextureRect(sf::IntRect(imageCount.x * textureSize.x,
				imageCount.y * textureSize.y, textureSize.x, textureSize.y));
			if (deathCount == 3)
				this->screen->remove(this);
		}
	}
	void Collision(GraphicalGameObject& other)
	{
		if (alive)
		{
			if (dynamic_cast<ZombieBlast*>(&other))
			{
				if (this->respawnManager) { this->respawnManager->died(this); }
				//std::cout << this->W_KeyHeld << this->A_KeyHeld << this->S_KeyHeld << this->D_KeyHeld << std::endl;
				alive = false;
				numMagesAlive--;
				DifficultySettings::Score::cumulativeBonusMultiplierCurrent = fmin(DifficultySettings::Score::cumulativeBonusMultiplierMax, DifficultySettings::Score::cumulativeBonusMultiplierCurrent + DifficultySettings::Score::cumulativeBonusMultiplier);
				(*Engine::scorePtr) += DifficultySettings::Score::applyMultipliers(20);
				this->screen->getSoundPlayer()->play(SoundEffect::ID::MageDeath, 30.f);
			}
		}
	}
	bool isAlive()
	{
		return this->alive;
	}
	sf::Sprite* spritePtr()
	{
		return dynamic_cast<sf::Sprite*>(this->graphic);
	}
};

#endif