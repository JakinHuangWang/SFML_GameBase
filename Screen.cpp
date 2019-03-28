#include "SFML/Graphics.hpp"
#include "Screen.h"
#include "GameObject.h"
#include "FileLoadException.h"
#include "DebugManager.h"
#include <utility>
#include <functional>

using namespace Engine;

static bool renderStarted = false;
static int currentFPS;
static sf::RenderWindow* windowPtr = nullptr;
static std::queue<std::pair<GameObject*, bool>> removeQueue;

namespace Engine
{
	typedef std::map<GameObjectID, GameObject*> GameObjectMap;
	unsigned int Screen::windowWidth = 0;
	unsigned int Screen::windowHeight = 0;
	const char* Screen::windowTitle = nullptr;
	static Screen* currentScreen;
	static Screen* pendingSwitch;
	bool running = true;
	bool windowInitialized = false;
	
	Screen::Screen() {}

	void Screen::addMap(TileMap* map)
	{
		this->tMap = map;
	}

	void Screen::addMainCharacter(GameObject* mainCharacter)
	{
		this->add(mainCharacter);
		this->mainCharacter = mainCharacter;
		mainCharacter->AddedToScreen();
	}

	GameObject* Screen::getMainCharacter() const
	{
		return this->mainCharacter;
	}

	void Screen::remove(GameObject* gameObject, bool autoDelete)
	{
		if (this != currentScreen)
		{	
			GameObjectID id = gameObject->getID();
			#define REMOVE(fromMap) if (fromMap.find(id) != fromMap.end())\
			{\
				fromMap.erase(id);\
				if (autoDelete) { delete gameObject; }\
			}
			REMOVE(this->objects);
			REMOVE(this->gObjects);
			REMOVE(this->uiObjects);
			#undef REMOVE
		}
		else
		{
			removeQueue.push({ gameObject, autoDelete});
		}
	}

	sf::Vector2i Screen::getMousePosition() const
	{
		if (!windowPtr) { return sf::Vector2i(0, 0); }
		sf::Vector2i pixelPos = sf::Mouse::getPosition(*windowPtr);
		sf::Vector2f worldPos = windowPtr->mapPixelToCoords(pixelPos, windowPtr->getView());
		return sf::Vector2i(static_cast<int>(worldPos.x), static_cast<int>(worldPos.y));
	}

	const TileMap* Screen::getMap() const
	{
		return this->tMap;
	}

	void Screen::close()
	{
		running = false;
	}

	void Screen::render(int fps)
	{
		if (fps < 1) { fps = 1; }
		else if (fps > 1000) { fps = 1000; }
		currentFPS = fps;

		unsigned int width = (Screen::windowWidth) ? Screen::windowWidth : 500;
		unsigned int height = (Screen::windowHeight) ? Screen::windowHeight : 500;
		const char* title = (Screen::windowTitle) ? Screen::windowTitle : "<no title>";
		static sf::RenderWindow window(sf::VideoMode(width, height), title, sf::Style::Close);
		static sf::Clock clock;
		static uint64_t frameCount = 0;

		sf::View view(sf::Vector2f(static_cast<float>(width / 2), static_cast<float>(height / 2)), sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
		windowPtr = &window;
		window.setView(view);

		if (renderStarted)
		{
			pendingSwitch = this;
			return;
		}
		else
		{
			pendingSwitch = nullptr;
		}
		currentScreen = this;
		renderStarted = true;

		#ifdef _DEBUG
		sf::Clock eventClock;
		sf::Clock collisionClock;
		sf::Clock drawClock;
		sf::Int64 eventDurationSum = 0;
		sf::Int64 collisionDurationSum = 0;
		sf::Int64 drawDurationSum = 0;
		sf::Int64 frameDurationSum = 0;
		#endif
		
		//GAME LOOP
		while (window.isOpen() && !pendingSwitch)
		{
			try
			{
				clock.restart();
				#ifdef _DEBUG
				eventClock.restart();
				#endif

				//run the EveryFrame event on all objects
				#define RUN_EVERYFRAME(forMap) for (auto const& pair : forMap)\
				{\
					GameObject* obj = pair.second;\
					if (!obj->eventsDisabled) { obj->EveryFrame(frameCount); }\
				}
				RUN_EVERYFRAME(currentScreen->objects);
				RUN_EVERYFRAME(currentScreen->gObjects);
				RUN_EVERYFRAME(currentScreen->uiObjects);
				#undef RUN_EVERYFRAME

				sf::Event ev;
				while (window.pollEvent(ev))
				{
					if (ev.type == sf::Event::Closed || !running)
					{
						window.close();
						return;
					}
					else if (ev.type == sf::Event::Resized)
					{
						// update the view to the new size of the window
						sf::FloatRect visibleArea(0.f, 0.f, static_cast<float>(ev.size.width), static_cast<float>(ev.size.height));
						view = sf::View(visibleArea);
					}
					//handle events on each object

					#define HANDLE_EVENTS(forMap) for (auto const& pair : forMap)\
					{\
						GameObject* obj = pair.second;\
						if (!obj->eventsDisabled) { obj->dispatchEvent(ev); }\
					}
					HANDLE_EVENTS(currentScreen->objects);
					HANDLE_EVENTS(currentScreen->gObjects);
					HANDLE_EVENTS(currentScreen->uiObjects);
					#undef HANDLE_EVENTS
				}

				#ifdef _DEBUG
				eventDurationSum += eventClock.getElapsedTime().asMicroseconds();
				collisionClock.restart();
				#endif

				const GraphicalGameObject* mainCharacterGraphical = dynamic_cast<const GraphicalGameObject*>(mainCharacter);
				const sf::Sprite* mainCharacterSprite = (mainCharacterGraphical != nullptr) ? dynamic_cast<const sf::Sprite*>(mainCharacterGraphical->graphic) : nullptr;

				//trigger collision events				
				for (auto const& p1 : currentScreen->gObjects)
				{
					GraphicalGameObject* eventReciever = p1.second;
					if (eventReciever->eventsDisabled) { continue; }
					sf::Sprite* receiverSprite = dynamic_cast<sf::Sprite*>(eventReciever->getGraphic());
					if (!receiverSprite) { continue; }
					for (auto const& p2 : currentScreen->gObjects)
					{
						GraphicalGameObject* eventArg = p2.second;
						if (eventArg == eventReciever || !eventArg->triggerCollisionEvents || !eventReciever->triggerCollisionEvents) { continue; }
						sf::Sprite* argSprite = dynamic_cast<sf::Sprite*>(eventArg->getGraphic());
						if (!argSprite) { continue; }
						sf::FloatRect r1 = receiverSprite->getGlobalBounds();
						sf::FloatRect r2 = argSprite->getGlobalBounds();
						if (r1.intersects(r2))
						{
							eventReciever->Collision(*eventArg);
						}
					}
				}

				unsigned int mapWidth = 0;
				unsigned int mapHeight = 0;
				if (currentScreen->tMap)
				{
					mapWidth = currentScreen->tMap->width() * currentScreen->tMap->tileSize().x;
					mapHeight = currentScreen->tMap->height() * currentScreen->tMap->tileSize().y;
				}

				//prevent objects from leaving the map
				for (auto const& pair : currentScreen->gObjects)
				{
					GraphicalGameObject* obj = pair.second;
					if (obj->ignoreObstacles) { continue; }
					if (!dynamic_cast<sf::Transformable*>(obj->graphic)) { continue; }
					sf::Sprite* sprite = dynamic_cast<sf::Sprite*>(obj->graphic);					
					if (!sprite) { continue; }

					#define X (sprite->getPosition().x)
					#define Y (sprite->getPosition().y)
					sf::Vector2u size(0, 0);
					if (sprite) { size = sf::Vector2u(sprite->getTextureRect().width, sprite->getTextureRect().height); }
					if (X < 0.f) { sprite->setPosition(0.f, Y); }
					if (Y < 0.f) { sprite->setPosition(X, 0.f); }
					if (Y + size.y > mapHeight) { sprite->setPosition(X, static_cast<float>(mapHeight - size.y)); }
					if (X + size.x > mapWidth) { sprite->setPosition(static_cast<float>(mapWidth - size.x), Y); }

					sf::Vector2f offsets(0.f, 0.f);
					if (obj->obstacleCollisionSize.width > 0.f && obj->obstacleCollisionSize.height > 0.f)
					{
						offsets.x = obj->obstacleCollisionSize.left;
						offsets.y = obj->obstacleCollisionSize.top;
						size.x = static_cast<unsigned int>(obj->obstacleCollisionSize.width);
						size.y = static_cast<unsigned int>(obj->obstacleCollisionSize.height);
					}

					do
					{
						sf::Vector2f corners[4] = {
							{X + static_cast<float>(offsets.x)          , Y + static_cast<float>(offsets.y)          },
							{X + static_cast<float>(size.x + offsets.x) , Y + static_cast<float>(offsets.y)          },
							{X + static_cast<float>(offsets.x)          , Y + static_cast<float>(size.y + offsets.y) },
							{X + static_cast<float>(size.x + offsets.x) , Y + static_cast<float>(size.y + offsets.y) }
						};

						bool collision = false;
						for (auto corner : corners)
						{
							if (currentScreen->tMap->isObstacle(corner))
							{
								collision = true;
								break;
							}
						}
						if (collision)
						{
							if (obj->spawnCollisionsResolved) { sprite->setPosition(obj->lastPos); }
							else
							{
								auto positions = this->tMap->getSafeSpawnPositions();
								sprite->setPosition(positions[rand() % positions.size()]);
							}
						}
						else { obj->spawnCollisionsResolved = true; }
					} while (!obj->spawnCollisionsResolved);
					obj->lastPos = { X , Y };
					#undef X
					#undef Y
				}

				#ifdef _DEBUG
				collisionDurationSum += collisionClock.getElapsedTime().asMicroseconds();
				drawClock.restart();
				#endif

				window.clear();

				//draw the map
				if (currentScreen->tMap) { window.draw(*currentScreen->tMap); }

				//draw the objects
				for (auto const& pair : currentScreen->gObjects)
				{
					GraphicalGameObject* obj = pair.second;
					obj->draw(window);
				}

				//draw the UI objects
				for (auto const& pair : currentScreen->uiObjects)
				{
					GraphicalGameObject* obj = dynamic_cast<GraphicalGameObject*>(pair.second);
					sf::Transformable* transformable = dynamic_cast<sf::Transformable*>(obj->getGraphic());
					if (!transformable) { continue; }
					sf::Vector2f viewPos = window.getView().getCenter();
					sf::Vector2f screenPosition = transformable->getPosition();
					transformable->setPosition(viewPos - sf::Vector2f(static_cast<float>(currentScreen->windowWidth / 2), static_cast<float>(currentScreen->windowHeight / 2)) + screenPosition);
					obj->draw(window);
					transformable->setPosition(screenPosition);
				}

				//view moves with character
				if (mainCharacterSprite != nullptr)
				{
					sf::Vector2f pos = mainCharacterSprite->getPosition();
					float x = pos.x;
					float y = pos.y;
					float fWidth = static_cast<float>(mapWidth);
					float fHeight = static_cast<float>(mapHeight);
					float halfWidth = static_cast<float>(windowWidth / 2);
					float halfHeight = static_cast<float>(windowHeight / 2);
					if (x > halfWidth && x < (fWidth - halfWidth)
						&& y > halfHeight && y < (fHeight - halfHeight))
					{
						view.setCenter(pos);
					}
					else if (x >= 0.f && x <= halfWidth &&
						y >= 0.f && y <= halfHeight)
					{
						view.setCenter(halfWidth, halfHeight);
					}
					else if (x >= 0.f && x <= halfWidth &&
						y >= fHeight - halfHeight && y <= fHeight)
					{
						view.setCenter(halfWidth, fHeight - halfHeight);
					}
					else if (x >= fWidth - halfWidth && x <= fWidth &&
						y >= 0.f && y <= halfHeight)
					{
						view.setCenter(fWidth - halfWidth, halfHeight);
					}
					else if (x >= fWidth - halfWidth && x <= fWidth &&
						y >= fHeight - halfHeight && y <= fHeight)
					{
						view.setCenter(fWidth - halfWidth, fHeight - halfHeight);
					}
					else if (x > halfWidth && x < fWidth - halfWidth &&
						y >= 0.f && y <= halfHeight)
					{
						view.setCenter(x, halfHeight);
					}
					else if (x > halfWidth && x < fWidth - halfWidth &&
						y >= fHeight - halfHeight && y <= fHeight)
					{
						view.setCenter(x, fHeight - halfHeight);
					}
					else if (x >= 0.f && x <= halfWidth &&
						y > halfHeight && y < fHeight - halfHeight)
					{
						view.setCenter(halfWidth, y);
					}
					else if (x >= fWidth - halfWidth && x <= fWidth &&
						y > halfHeight && y < mapHeight - halfHeight)
					{
						view.setCenter(fWidth - halfWidth, y);
					}
				}
				#ifdef _DEBUG
				drawDurationSum += drawClock.getElapsedTime().asMicroseconds();
				#endif

				window.setView(view);
				window.display();

				//remove objects that are pending to be removed
				while (!removeQueue.empty())
				{
					std::pair<GameObject*, bool> pRemove = removeQueue.front();
					GameObject* toRemove = pRemove.first;
					bool autoDelete = pRemove.second;
					removeQueue.pop();
					GameObjectID id = toRemove->getID();
					#define REMOVE(fromMap) if (fromMap.find(id) != fromMap.end())\
					{\
						toRemove->RemovedFromScreen();\
						fromMap.erase(id);\
						if (autoDelete) { delete toRemove; }\
					}
					REMOVE(this->objects);
					REMOVE(this->gObjects);
					REMOVE(this->uiObjects);
					#undef REMOVE
				}

			}
			catch (GameException::FileLoadException& e)
			{
				std::cout << "Failed to load file: " << e.getFileName() << std::endl;
				std::cout << " -- Fatal error. Program must terminate." << std::endl;
				window.close();
			}
			catch (...)
			{
				#ifdef _DEBUG
				DebugManager::PrintMessage(DebugManager::MessageType::ERROR_REPORTING, "Unknown error.");
				#endif
				window.close();
			}
			frameCount++;

			#ifdef _DEBUG
			frameDurationSum += clock.getElapsedTime().asMicroseconds();
			int avgFrameReportFrequency = 60;
			if (frameCount % avgFrameReportFrequency == 0)
			{	
				DebugManager::MessageType msgType = DebugManager::MessageType::PERFORMANCE_REPORTING;
				DebugManager::PrintMessage(msgType, string("\naverage event compute time: ") + std::to_string(eventDurationSum / avgFrameReportFrequency));
				DebugManager::PrintMessage(msgType, string("average collision compute time: ") + std::to_string(collisionDurationSum / avgFrameReportFrequency));
				DebugManager::PrintMessage(msgType, string("average draw compute time: ") + std::to_string(drawDurationSum / avgFrameReportFrequency));
				DebugManager::PrintMessage(msgType, string("average total compute time: ") + std::to_string(frameDurationSum / avgFrameReportFrequency));
				DebugManager::PrintMessage(msgType, string("max total before slowdown: ") + std::to_string(1000000 / currentFPS));
				frameDurationSum = 0;
				eventDurationSum = 0;
				collisionDurationSum = 0;
				drawDurationSum = 0;
			}
			#endif
			while (clock.getElapsedTime().asMicroseconds() < (1000000 / currentFPS)) {}
		} //END OF GAME LOOP

		if (pendingSwitch)
		{
			renderStarted = false;
			pendingSwitch->render();
		}
	}

	Screen::~Screen()
	{
	}
	
	class Scheduler : public GameObject
	{
	private:
		uint64_t delay;
		uint64_t countdown;
		uint16_t repeatsRemaining;
		bool infinite;
		function<void()> func;
	public:
		Scheduler( function<void()> func, uint64_t delay, uint16_t repeatCount) : delay(delay), countdown(delay), repeatsRemaining(repeatCount), infinite(repeatCount == 0), func(func) {}
		void EveryFrame(uint64_t f)
		{
			if (this->countdown == 0)
			{
				func();
				if (repeatsRemaining > 0)
				{
					repeatsRemaining--;
					this->countdown = this->delay;
				}
				else if(!this->infinite) { this->screen->remove(this); }				
			}
			else { this->countdown--; }
		}
	};

	void Screen::schedule(function<void()> func, TimeUnit::Time delay, uint16_t repeatCount)
	{
		this->add(new Scheduler(func, delay, repeatCount));
	}
}