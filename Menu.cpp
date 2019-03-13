#include "Menu.h"
#include "TestLevel.h"
#include "DifficultySettings.h"
#include "ScoreBoard.h"
#include "Tutorial.h"
#include "UIButton.h"
#include <string>

class PlayerNameEntry : public GraphicalGameObject
{
public:
	PlayerNameEntry() : GraphicalGameObject(sf::Text())
	{
		this->font.loadFromFile("Lycanthrope.ttf");
		this->textPtr()->setFont(this->font);
		this->textPtr()->setStyle(sf::Text::Bold);
		this->textPtr()->setFillColor(sf::Color(179, 45, 0));
		this->textPtr()->setCharacterSize(50U);
		if (DifficultySettings::currentDifficulty != DifficultySettings::TEST)
		{
			this->textPtr()->setPosition(150.f, 150.f);
			this->textPtr()->setString("Enter your name while you can:\n");
		}
		else
		{
			this->textPtr()->setPosition(100.f, 80.f);
			this->textPtr()->setString("Oops, you've entered a secret base\n created by your family, but you need \na password to enter:\n");
		}
		background.setPosition(600.f, 400.f);
		texture.loadFromFile("bloodyhands.png");
		background.setTexture(texture);
		ready = false;
		decline = false;
		for (auto obj : Menu::getCurrentMenu()->getMenuObjects())
		{
			if (obj != this) { obj->disableEvents(); }
		}
	}
	void draw(sf::RenderWindow& win)
	{
		win.clear();
		win.draw(background);
		win.draw(*this->textPtr());
		Music::ID music;
		if (ready && DifficultySettings::currentDifficulty != DifficultySettings::TEST)
		{
			win.display();
			do
			{
				;
			} while (clock.getElapsedTime().asSeconds() < 1.5f); // wait for a second for the good luck text to display before entering the game
			{
				switch (DifficultySettings::currentDifficulty)
				{
				case DifficultySettings::EASY:
					music = Music::ID::EasyGame;
					break;
				case DifficultySettings::NORMAL:
					music = Music::ID::NormalGame;
					break;
				case DifficultySettings::HARD:
					music = Music::ID::HardGame;
					break;
				default:
					music = Music::ID::TestMode;
					break;
				}
				Engine::musicPlayer.stop();
				Engine::musicPlayer.play(music);
				Engine::musicPlayer.setVolume(20.f);
				this->screen->remove(this);
				Menu::getCurrentMenu()->startTestLevel(this->name);
			}
		}
		
		if (ready && DifficultySettings::currentDifficulty == DifficultySettings::TEST)
		{
			music = Music::ID::TestMode;
			win.display();
			do
			{
				;
			} while (clock.getElapsedTime().asSeconds() < 1.5f);
			Engine::musicPlayer.stop();
			Engine::musicPlayer.play(music);
			Engine::musicPlayer.setVolume(20.f);
			this->screen->remove(this);
			Menu::getCurrentMenu()->startTestLevel(this->name);
		}

		if (decline)
		{
			win.display();
			do
			{
				;
			} while (clock.getElapsedTime().asSeconds() < 1.5f);
			decline = false;
			this->screen->remove(this);
		}
	}
	void TextEntered(sf::Event e)
	{
		int key = e.text.unicode;
		if (key == 8) //8 is delete
		{
			if (this->name.size() > 0)
			{
				this->name = this->name.substr(0, this->name.size() - 1);
			}

			if (DifficultySettings::currentDifficulty != DifficultySettings::TEST)
			{
				this->textPtr()->setString("Enter your name while you can:\n" + this->name);
			}
			else
			{
				this->textPtr()->setString("Oops, you've entered a secret base\n created by your family, but you need \na password to enter:\n" + this->name);
			}
		}
		else if (key == static_cast<int>('\r') || key == static_cast<int>('\n'))
		{
			if (DifficultySettings::currentDifficulty != DifficultySettings::TEST)
			{
				this->textPtr()->setString("Enter your name while you can:\n" + this->name + "\n\nGood luck, " + this->name + "!");
				ready = true;
				clock.restart();
			}
			else
			{
				this->textPtr()->setPosition(290.f, 280.f);
				if (this->name != "our game is better than yours! :)")
				{
					this->textPtr()->setString("Sorry, get out!!!");
					decline = true;
					clock.restart();
				}
				else
				{
					this->textPtr()->setString("Welcome, my master...");
					ready = true;
					clock.restart();
				}
			}
		}
		else
		{
			this->name += static_cast<char>(e.text.unicode);
			if (DifficultySettings::currentDifficulty != DifficultySettings::TEST)
			{
				this->textPtr()->setString("Enter your name while you can:\n" + this->name);
			}
			else
			{
				this->textPtr()->setString("Oops, you've entered a secret base\n created by your family, but you need \na password to enter:\n" + this->name);
			}
		}
	}
private:
	std::string name;
	sf::Font font;
	sf::Text* textPtr() { return dynamic_cast<sf::Text*>(this->graphic); }
	sf::Texture texture;
	sf::Sprite background;
	bool ready;
	bool decline;
	sf::Clock clock;
};

class TestModeButton : public UIButton
{
public:
	TestModeButton() : UIButton("?", { 100.f, 100.f }, { 100.f, 100.f })
	{
		myFont.loadFromFile("DoubleFeature.ttf");
		this->textPtr()->setFont(myFont);
	}
	void KeyReleased(sf::Event e)
	{
		if (e.key.code == sf::Keyboard::Z)
		{
			ZPressed++;
			if (ZPressed == 4 && this->enabled)
			{
				this->activated = true;
			}
		}
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (!this->enabled || !this->activated) { return; }
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			DifficultySettings::setDifficulty(DifficultySettings::TEST);
			Engine::soundPlayer.play(SoundEffect::ID::MenuClick, 20.f);
			this->screen->addUIObject(new PlayerNameEntry());
		}
	}
	void draw(sf::RenderWindow& win)
	{
		if (this->activated && this->enabled) { win.draw(*this->graphic); }
	}
	void EveryFrame(uint64_t f)
	{
		if (f % 60 == 0)
		{
			if (this->ZPressed > 0) { this->ZPressed--; }
		}
	}
	void enable()
	{
		this->enabled = true;
	}
	void disable()
	{
		this->enabled = false;
	}
private:
	bool activated = false;
	bool enabled = true;
	int ZPressed = 0;
};

class GameTitle : public GraphicalGameObject {
public:
	GameTitle() : GraphicalGameObject(sf::Text())
	{
		font.loadFromFile("DoubleFeature.ttf");
		this->textPtr()->setFont(font);
		this->textPtr()->setString("Cursed Zombie");
		this->textPtr()->setStyle(sf::Text::Bold);
		this->textPtr()->setFillColor(sf::Color(179, 45, 0));
		this->textPtr()->setCharacterSize(50);
		this->textPtr()->setPosition(340.f, 50.f);
	}
private:
	sf::Font font;
	sf::Text* textPtr() { return dynamic_cast<sf::Text*>(this->getGraphic()); }
};

class EasyLevelButton : public UIButton
{
public:
	EasyLevelButton() : UIButton("Easy", { 412.f, 162.f }, { 200.f, 75.f })
	{
		this->setFont("DoubleFeature.ttf");
		this->setTextSize(35.f);
		this->setTextColor(sf::Color(179, 45, 0));
		this->setBackgroundColor(sf::Color(0, 0, 0, 0));
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			DifficultySettings::setDifficulty(DifficultySettings::EASY);
			Engine::soundPlayer.play(SoundEffect::ID::MenuClick, 20.f);
			this->screen->addUIObject(new PlayerNameEntry());
		}
	}
};

class NormalLevelButton : public UIButton {
public:
	NormalLevelButton() : UIButton("Normal", { 412.f, 242.f }, { 200.f, 75.f })
	{
		this->setFont("DoubleFeature.ttf");
		this->setTextSize(35.f);
		this->setTextColor(sf::Color(179, 45, 0));
		this->setBackgroundColor(sf::Color(0, 0, 0, 0));
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			DifficultySettings::setDifficulty(DifficultySettings::NORMAL);
			Engine::soundPlayer.play(SoundEffect::ID::MenuClick, 20.f);
			this->screen->addUIObject(new PlayerNameEntry());
		}
	}
};

class HardLevelButton : public UIButton {
public:
	HardLevelButton() : UIButton("Insane", { 412.f, 322.f }, { 200.f, 75.f })
	{
		this->setFont("DoubleFeature.ttf");
		this->setTextSize(35.f);
		this->setTextColor(sf::Color(179, 45, 0));
		this->setBackgroundColor(sf::Color(0, 0, 0, 0));
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			DifficultySettings::setDifficulty(DifficultySettings::HARD);
			Engine::soundPlayer.play(SoundEffect::ID::MenuClick, 20.f);
			this->screen->addUIObject(new PlayerNameEntry());
		}
	}
};

class TutorialButton : public UIButton {
public:
	TutorialButton() : UIButton("Tutorial", { 412.f, 402.f }, { 200.f, 75.f })
	{
		this->setFont("DoubleFeature.ttf");
		this->setTextSize(35.f);
		this->setTextColor(sf::Color(179, 45, 0));
		this->setBackgroundColor(sf::Color(0, 0, 0, 0));
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			Tutorial* tutorial = new Tutorial();
			this->screen->addUIObject(tutorial);
		}
	}
};

class ScoreboardButton : public UIButton {
public:
	ScoreboardButton() : UIButton("Scoreboard", { 412.f, 482.f }, { 200.f, 75.f })
	{
		this->setFont("DoubleFeature.ttf");
		this->setTextSize(35.f);
		this->setTextColor(sf::Color(179, 45, 0));
		this->setBackgroundColor(sf::Color(0, 0, 0, 0));
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			ScoreBoard* scoreBoard = new ScoreBoard();
			this->screen->addUIObject(scoreBoard);
		}
	}
};

class QuitButton : public UIButton
{
public:
	QuitButton() : UIButton("Escape", { 412.f, 562.f }, { 200.f, 75.f })
	{
		this->setFont("DoubleFeature.ttf");
		this->setTextSize(35.f);
		this->setTextColor(sf::Color(179, 45, 0));
		this->setBackgroundColor(sf::Color(0, 0, 0, 0));
	}
	void MouseButtonReleased(sf::Event e)
	{
		if (e.mouseButton.button == sf::Mouse::Button::Left //if the left mouse button was clicked
			&& this->background.getGlobalBounds().contains(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y))) //if the click was inside the button
		{
			Engine::soundPlayer.play(SoundEffect::MenuClick, 20.f);
			Engine::musicPlayer.stop();
			this->screen->close();
		}
	}
};

namespace Engine
{
	static Menu* currentMenu = nullptr;

	Menu::Menu()
	{
		currentMenu = this;
		this->menuObjects = {
			new GameTitle(),
			new EasyLevelButton(),
			new NormalLevelButton(),
			new HardLevelButton(),
			new TutorialButton(),
			new ScoreboardButton(),
			new QuitButton(),
			new TestModeButton()
		};
	}

	Menu::~Menu()
	{
		for (auto obj : this->menuObjects)
		{
			delete obj;
			obj = nullptr;
		}
	}

	void Menu::start()
	{
		Engine::musicPlayer.play(Music::Menu);
		for (auto obj : this->menuObjects)
		{
			menuScreen.addUIObject(obj);
		}
		this->menuScreen.render();
	}

	void Menu::startTestLevel(std::string playerName)
	{
		this->testLevel.start(playerName);
	}

	Menu* Menu::getCurrentMenu()
	{
		return currentMenu;
	}
}
