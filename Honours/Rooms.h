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
		CorridorType,
		Empty
	};

	enum Direction
	{
		Up,
		Down,
		Left,
		Right
	};

	enum RoomPattern
	{
		Null,
		Arena,
		Flank,
		ChokePoint,
		Objective
	};

	struct Connection
	{
		sf::Vector2f pos;
		Direction dir;
		int roomIndex;
	};

	//shared variables between all room types
	RoomType type;
	RoomPattern pattern = Null;

	sf::RectangleShape shape;

	int heirarchy;
	int parentIndex = -1;
	int arrayIndex;

	int sizeConstraint;

	std::vector<Connection> connections;

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