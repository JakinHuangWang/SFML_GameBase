#ifndef SCREEN_H
#define SCREEN_H

#include "GameObject.h"
#include "TileMap.h"
#include "MusicPlayer.h"
#include "SoundPlayer.h"
#include <map>
#include <functional>
#include <queue>
#include <vector>
#include <utility>

using std::map;
using std::function;
using std::vector;
using std::map;

namespace Engine
{
	namespace TimeUnit
	{
		class Time
		{
		public:
			Time(uint64_t frames) : frames(frames) { }
			operator uint64_t() const { return this->frames; }
		private:
			uint64_t frames;
		};

		class Minutes : public Time
		{
		public:
			Minutes(uint64_t minutes) : Time(minutes * 60 * 60) {}
		};

		class Seconds : public Time
		{
		public:
			Seconds(uint64_t seconds) : Time(seconds * 60) {}
		};
		
		class Frames : public Time
		{
		public:
			Frames(uint64_t frames) : Time(frames) {}
		};
	}

	class Screen
	{
	public:
		Screen();
		~Screen();
		void addMap(TileMap* map);
		void addMainCharacter(GameObject* mainCharacter);

		template<typename T> void add(T* gameObject)
		{
			static_assert(std::is_base_of<GameObject, T>::value, "Argument of Screen::add must inherit from GameObject");
			if (gameObject == nullptr) { return; }
			if(GraphicalGameObject* ggo = dynamic_cast<GraphicalGameObject*>(gameObject)) { this->gObjects[ggo->getID()] = ggo; }
			else { this->objects[gameObject->getID()] = gameObject; }
			gameObject->screen = this;
			gameObject->AddedToScreen();
		}

		template<typename T> void addUIObject(T* uiObj)
		{
			static_assert(std::is_base_of<GameObject, T>::value, "Argument of Screen::addUIObject must inherit from GameObject");
			if (GraphicalGameObject* ggo = dynamic_cast<GraphicalGameObject*>(uiObj))
			{
				this->uiObjects[ggo->getID()] = ggo;
				ggo->screen = this;
				ggo->AddedToScreen();
			}
		}

		void remove(GameObject* gameObject, bool autoDelete = true);
		void schedule(function<void()> func, TimeUnit::Time delay, uint16_t repeatCount = 1);
		void render(int fps = 60);
		void close();
		sf::Vector2i getMousePosition() const;
		GameObject* getMainCharacter() const;
		const TileMap* getMap() const;
		unsigned static int windowWidth;
		unsigned static int windowHeight;
		static const char* windowTitle;
	private:
		map<GameObjectID, GameObject*> objects;
		map<GameObjectID, GraphicalGameObject*> gObjects; //GraphicalGameObjects go here so during rendering it doesn't have to check the other ones
		map<GameObjectID, GraphicalGameObject*> uiObjects; //UI objects have an absolute position on the screen so they follow the view. they have no collision either.
		GameObject* mainCharacter = nullptr;
		TileMap* tMap = nullptr;
	};
}
#endif