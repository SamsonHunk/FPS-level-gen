// Honours.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <SFML/Graphics.hpp>
#include <iostream>
#include "Base.h"

//grid dimensions
int size;

enum TileType
{
	Empty,
	Floor,
	Wall,
};

//storage for data representation of generated level
std::vector<short int> area;

//sfml rectangles to visualise the created level
Base roomTree;

//small function to ease accessing and organising data in the vector
int index(int x, int y)
{
	return x + size * y;
}

int tileMultiplier = 2; //each tile is 2x2 in the image to try and keep scale good
//a player takes up about 1 space

//function encompassing the level generation
void generate();

//function to insert a new room into the structure
void drawRoom(sf::Vector2f pos, sf::Vector2f size);

int main()
{
	//determine how big the map is going to be
	std::cout << "How large is the map area? 100x100 min" << std::endl;
	std::cin >> size;

	//add 1 to the size if it is even, we just want to make sure we have a point in the middle
	if (size % 2 == 0)
	{
		size++;
	}

	srand(time(NULL));

	//initialise the container as empty
	area.reserve(size*size);

	for (int it = 0; it < area.capacity(); it++)
	{
		area.push_back(Empty);
	}

	//generate the level
	generate();

	//render and show the final level
	sf::RenderWindow window(sf::VideoMode(size, size), "Output");
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			if (event.type == sf::Event::KeyReleased)
			{//press a button to make a new level
				//reset the level

				for (int it = 0; it < area.size(); it++)
				{
					area[it] = Empty;
				}

				//make a new one
				generate();
			}
		}

		window.clear();
		window.draw(roomTree);
		window.display();
	}

	return 0;
}

void generate()
{
	std::cout << "Creating the base" << std::endl;
	//figure out the center of the grid

	int center = (size / 2);

	roomTree.setPosition(sf::Vector2f(center, center));
	roomTree.generate();
}

void drawRoom(sf::Vector2f pos, sf::Vector2f size)
{
	//save the room data into the final output

	//figure out the index of the shapes bottom left corner
	/*
	|
	|
	|
	|________
	*/

	int cornerCoord[2] = { pos.x - (size.x / 2), pos.y - (size.y / 2) };

	//then we build a room as a rectangle of floor surrounded by wall
	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			area[index(cornerCoord[0] + x, cornerCoord[1] + y)] = Floor;
		}
	}
}