#pragma once
#include "SFML/Graphics.hpp"

class Room: public sf::RectangleShape
{
public:
	//generation rules for specific room
	virtual bool generate() = 0;

	enum RoomType
	{
		Base,
		Corridor,
		Small,
		Large
	};

protected:
	//a room has size constraints and connects to another amount of rooms
	sf::Vector2f minSize;
	sf::Vector2f maxSize;
	std::vector<Room> connectedRooms;
	int maxChildRooms = 4;
	int minChildRooms = 2;
};