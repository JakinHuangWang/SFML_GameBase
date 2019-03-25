#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "FileLoadException.h"

using std::string;
using std::unordered_map;
using std::queue;
using std::cout;
using std::endl;

namespace Engine
{
	template<typename T> class ResourceManager
	{
	public:
		ResourceManager() = delete;
		static T* GetResource(string filename)
		{
			unordered_map<string, T*>& resourceCache = getResourceCache();
			auto iter = resourceCache.find(filename);
			if (iter != resourceCache.end())
			{
#ifdef _DEBUG
				cout << "Resource \"" << filename << "\" found in cache." << endl;
#endif
				return (*iter).second;
			}
			else
			{
#ifdef _DEBUG
				cout << "Resource \"" << filename << "\" not found in cache. Loading from file." << endl;
#endif
			}
			T* resourcePtr = new T();
			if (!resourcePtr->loadFromFile(filename)) { throw GameException::DataFileLoadException(filename); }
			resourceCache[filename] = resourcePtr;
#ifdef _DEBUG
			cout << "Resource \"" << filename << "\" loaded successfully." << endl;
#endif
			return resourcePtr;
		}

		static T* ReloadResource(string filename)
		{
			UnloadResource(filename);
			return GetResource(filename);
		}

		static void UnloadResource(string filename)
		{
			unordered_map<string, T*> resourceCache = getResourceCache();
			auto iter = resourceCache.find(filename);
			if (iter == resourceCache.end()) { return; }
			T* ptr = (*iter).second;
			resourceCache.erase(iter);
			delete ptr;
		}

		static void ReloadAllResources()
		{
			unordered_map<string, T*>& resourceCache = getResourceCache();
			queue<string> reloadQueue;
			for (auto pair : resourceCache)
			{
				reloadQueue.push(pair.first);
			}
			resourceCache.clear();
			while (!reloadQueue.empty())
			{
				string filename = reloadQueue.front();
				reloadQueue.pop();
				ReloadResource(filename);
			}
		}
	private:
		static unordered_map<string, T*>& getResourceCache()
		{
			static unordered_map<string, T*> resourceCache;
			return resourceCache;
		}
	};
}

#endif