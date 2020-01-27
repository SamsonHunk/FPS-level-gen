#include "Base.h"

Base::Base()
{
	minSize.x = 50;
	minSize.y = 50;

	maxSize.x = 100;
	maxSize.y = 100;

	maxChildRooms = 7;
	minChildRooms = 4;

	setOutlineColor(sf::Color::Red);
	setFillColor(sf::Color::Green);
	setOutlineThickness(0.5f);
}

Base::~Base()
{
}

bool Base::generate()
{
	//determine the size of the room and how many children it has
	childCount = rand() % (maxChildRooms - minChildRooms) + minChildRooms;

	sf::Vector2f size;

	size.x = rand() % (maxSize.x - minSize.x) + minSize.x;
	size.y = rand() % (maxSize.y - minSize.y) + minSize.y;

	setSize(size);

	setOrigin(sf::Vector2f(size.x / 2, size.y / 2));

	return true;
}
