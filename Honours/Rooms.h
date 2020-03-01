#pragma once
#include <SFML/Graphics.hpp>

//parent class for room objects


class Room
{
public:
	enum RoomType
	{
		BaseType,
		NormalType,
		CorridorType
	};

	//shared variables between all room types
	RoomType type;

	sf::RectangleShape shape;

	int heirarchy;
	int parentIndex = -1;

	int sizeConstraint;

	std::vector<sf::Vector2f> connectionsPos;

protected:
	int roomTileSize = 5;
};

class Base : public Room
{
public:
	void generate(sf::Vector2f pos, int fieldSize);
};

class NormalRoom : public Room
{
public:
	void generate(sf::Vector2f pos, int maxSize);
};

class Corridor : public Room
{
public:
	bool generate(Room* room0, Room* room1);
};