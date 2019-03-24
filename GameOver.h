#ifndef GAMEOVER_H
#define GAMEOVER_H

#include "SFML/Graphics.hpp"
#include "GameObject.h"
#include "DifficultySettings.h"

using namespace Engine;

class GameOver : public GraphicalGameObject
{
private:
	DifficultySettings::DIFFICULTY difficulty;
	int finalScore = 0;
	int internalClock = 0;
	sf::Sprite backSprite;
	sf::Sprite* spritePtr() { return dynamic_cast<sf::Sprite*>(this->graphic); }
public:
	GameOver(int finalScore, DifficultySettings::DIFFICULTY difficulty);
	void AddedToScreen();
	void EveryFrame(uint64_t f);
	void MouseButtonReleased(sf::Event e);
	void draw(sf::RenderWindow& win);
};

#endif
