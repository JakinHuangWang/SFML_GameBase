#ifndef ANTIMAGEPOTION_H
#define ANTIMAGEPOTION_H

#include "Screen.h"
#include "DifficultySettings.h"
#include "Score.h"
#include "ResourceManager.h"
#include "SpriteFactory.h"
#include "GameObjectAttribute.h"
#include <vector>
#include <ctime>

using namespace Engine;

class AntiMagePotion :
	public GraphicalGameObject,
	public Collision,
	public SpriteSheet
{
private:
	int wiggleDirection = 1;
	float wiggleSpeed = 1.1f;
	float wiggleMagnitude = 18.f;
	float currentRotation = 0.f;
public:
	AntiMagePotion() :
		GraphicalGameObject(SpriteFactory::generateSprite(Sprite::ID::AnimatedPotion)),
		SpriteSheet(8)
	{
		this->resetSpriteSheet();
		sf::IntRect size = this->spritePtr()->getTextureRect();
		this->spritePtr()->setOrigin(static_cast<float>(size.width / 2), static_cast<float>(size.height / 2));
	}

	void EveryFrame(uint64_t f)
	{
		if (f % 10 == 0) { this->spriteSheetRow++; }
		if (this->currentRotation >= this->wiggleMagnitude || this->currentRotation <= (-1.f * this->wiggleMagnitude)) { this->wiggleDirection *= -1; }
		this->currentRotation += static_cast<float>(this->wiggleDirection) * this->wiggleSpeed;
		this->spritePtr()->setRotation(this->currentRotation);
	}

	void die()
	{
		this->screen->remove(this);
	}

	sf::Sprite* spritePtr()
	{
		return dynamic_cast<sf::Sprite*>(this->graphic);
	}
};

#endif
