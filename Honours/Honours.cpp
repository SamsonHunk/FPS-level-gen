#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <writer.h>
#include "document.h"
#include "filewritestream.h"
#include "Rooms.h"

using namespace rapidjson;

//grid dimensions
int size;

std::string fileName = "test";

enum TileType
{
	Empty,
	Floor,
	Wall,
};

//storage for data representation of generated level
std::vector<short int> area;

int roomTileSize = 4; //minumum size of a room measurement
int maxHeirarchy = 6; //generation variable to control how many rooms to make

//sfml rectangles to visualise the created level
std::vector<Room> rooms;

//small function to ease accessing and organising data in the vector
int index(int x, int y)
{
	return x + size * y;
}

//function encompassing the level generation
void generate();

//function to insert a new room into the structure
void drawRoom(sf::Vector2f pos, sf::Vector2f size);

//generate and parse a new level file of the currently displayed level
void outputFile();

int main()
{
	//determine how big the map is going to be
	std::cout << "How large is the map area? 100x100 min" << std::endl;
	std::cin >> size;

	srand(time(NULL));

	//reserve the container in memory
	area.reserve(size*size);

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

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::A)
			{//press the a button to make a new level
				//reset the level


				for (int it = 0; it < area.size(); it++)
				{
					area[it] = Empty;
				}

				rooms.clear();

				//make a new one
				generate();
			}

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::S)
			{//press s to generate the json level file
				std::cout << "Enter level file name: ";
				std::cin >> fileName;
				outputFile();
			}
		}


		window.clear();
		for (int it = rooms.size() - 1; it > -1; it--)
		{//draw all the room graphics
			window.draw(rooms[it].shape);
		}
		window.display();
	}

	return 0;
}

void generate()
{
	std::cout << "Creating the base" << std::endl;
	//figure out the center of the grid

	int center = (size / 2);



	Base base;
	//generate the initial room, no other room in the map can be bigger than this room
	base.generate(sf::Vector2f(center, center), size);

	int generationHeriarchy = base.heirarchy; //current position in the generation loop


	//center the first room into the center of the screen
	base.shape.setPosition(base.shape.getPosition().x - base.shape.getSize().x, base.shape.getPosition().y - base.shape.getSize().y);

	rooms.push_back(base);

	//index storage to improve efficiency moving through the vector
	int roomIndex = 0;

	do
	{//generate the rooms in a flower pattern around the parent room until we reach the end of the heirarchy loop
		NormalRoom temp;
		generationHeriarchy /= 2;
		//check the current room and see if it needs more rooms attatched to it
		if (rooms[roomIndex].heirarchy > 1)
		{
			for (int it = 0; it < rooms[roomIndex].heirarchy; it++)
			{
				std::cout << "Attaching child room on layer " + std::to_string(generationHeriarchy) + " ";
				temp.heirarchy = generationHeriarchy;
				temp.parentIndex = roomIndex;

				//it may be the case that it's just not possible to attach a new room
				int iterations = 500;
				bool intersectFlag = false;
				do
				{//keep trying to connect a new room to the current room but make sure that it does not intersect with another room
					iterations--;

					//choose a random point on the parent's shape edge to attatch the new room to, make sure it doesn't intersect with another room
					sf::Vector2f point = rooms[temp.parentIndex].shape.getPosition();

					sf::Vector2f roomSize = rooms[temp.parentIndex].shape.getSize();

					//choose a random edge to shove the room onto and what direction to generate the shape to
					//we have to offset the shape as all shapes are drawn from their top left corner
					int dir = rand() % 4;
					switch (dir)
					{
					case 0: // left edge
						point.x -= roomTileSize;
						point.y += rand() % (int)roomSize.y;
						break;
					case 1: //upper edge
						point.x += rand() % (int)roomSize.x;
						point.y -= roomTileSize;
						break;
					case 2: //right edge
						point.y += rand() % (int)roomSize.y;
						point.x += roomTileSize + roomSize.x;
						break;
					case 3: //bottom edge`
						point.x += rand() % (int)roomSize.x;
						point.y += roomSize.y + roomTileSize;
						break;
					}


					//generate the shape with the chosen point in mind
					temp.generate(point, rooms[temp.parentIndex].sizeConstraint);

					roomSize = temp.shape.getSize();

					//we have to offset it again if it is above or to the left of the parent, remember the origin of the shape is the top left corner
					switch (dir)
					{
					case 0:
						temp.shape.move(sf::Vector2f(-roomSize.x, 0));
						break;
					case 1:
						temp.shape.move(sf::Vector2f(0, -roomSize.y));
						break;
					}

					intersectFlag = false;

					//now finally check if the shape overlaps another shape
					for (int it = 0; it < rooms.size(); it++)
					{
						if (rooms[it].shape.getGlobalBounds().intersects(temp.shape.getGlobalBounds()))
						{
							intersectFlag = true;
							break;
						}
					}

					//if the shape does not overlap another shape then we can finalise it as a valid room
					if (!intersectFlag)
					{
						rooms.push_back(temp);
						switch (dir)
						{
						case 0:
							std::cout << "on the left edge: ";
							break;
						case 1:
							std::cout << "on the upper edge: ";
							break;
						case 2:
							std::cout << "on the right edge: ";
							break;
						case 3:
							std::cout << "on the bottom edge: ";
							break;
						}
					}

				} while (intersectFlag && iterations != 0);

				if (iterations == 0)
				{
					std::cout << "Failed" << std::endl;
				}
				else
				{
					std::cout << "Success" << std::endl;
				}
			}
		}
		roomIndex++;
	} while (generationHeriarchy != 0);

	//after we create the rooms we need to connect them together with corridors
	//all rooms need to be connected to the closest room of the same heirarchy and their parent room but make sure that they dont intersect with anyone

	roomIndex = 0;

	do
	{
		if (rooms[roomIndex].parentIndex != -1)
		{//if the room has a parent, make a corridor connecting them together
			Corridor tempCorridor;
			if (tempCorridor.generate(&rooms[roomIndex], &rooms[rooms[roomIndex].parentIndex]))
			{
				rooms.push_back(tempCorridor);
			}
		}

		for (int it = 0; it < rooms.size(); it++)
		{//connect each room on the same heirarchy level together
			Corridor tempCorridor;
			if (it != roomIndex && rooms[it].type != Room::CorridorType && rooms[it].heirarchy == rooms[roomIndex].heirarchy)
			{
				if (tempCorridor.generate(&rooms[roomIndex], &rooms[it]))
				{
					bool intersectFlag = false;
					//check that the corridor doesn't intersect with other rooms
					for (int collisionIt = 0; collisionIt < rooms.size(); collisionIt++)
					{
						if (collisionIt != it && collisionIt != roomIndex && rooms[collisionIt].shape.getGlobalBounds().intersects(tempCorridor.shape.getGlobalBounds()))
						{
							intersectFlag = true;
						}
					}

					//if there is no other intersection then we can create the corridor
					if (!intersectFlag)
					{
						rooms.push_back(tempCorridor);
					}
				}
			}
		}


		roomIndex++;
		//dont draw corridors onto corridors

		if (roomIndex >= rooms.size())
		{
			std::cout << "Unable to make any corridors" << std::endl;
			break;
		}

	} while (rooms[roomIndex].type != Room::CorridorType);


}

void drawRoom(sf::Vector2f pos, sf::Vector2f size)
{
	//save the room data into the final output

	//figure out the index of the shapes top right corner
	/*
	____________
	|
	|
	|
	|
	*/

	//then we build a room as a rectangle of floor surrounded by wall
	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			area[index(pos.x + x, pos.y + y)] = Floor;
		}
	}
}

void outputFile()
{
	//initialise the structure as empty
	area.clear();

	for (int it = 0; it < area.capacity(); it++)
	{
		area.push_back(Empty);
	}

	std::cout << "Generating walls" << std::endl;

	//first create bare floor of each room
	for (int it = 0; it < rooms.size(); it++)
	{
		drawRoom(rooms[it].shape.getPosition(), rooms[it].shape.getSize());
	}

	//run edge detection and colour the edges in as walls
	for (int y = 0; y < size; y++)
	{
		for (int x = 1; x < size - 1; x++)
		{
			if ((area[index(x - 1, y)] == Empty || area[index(x + 1, y)] == Empty) && area[index(x, y)] == Floor)
			{
				area[index(x, y)] = Wall;
			}
		}
	}

	for (int x = 0; x < size; x++)
	{
		for (int y = 1; y < size - 1; y++)
		{
			if ((area[index(x, y - 1)] == Empty || area[index(x, y + 1)] == Empty) && area[index(x, y)] == Floor)
			{
				area[index(x, y)] = Wall;
			}
		}
	}

	std::cout << "Writing file" << std::endl;

	//write the array into a json file
	FILE* fp = fopen(("output/" + fileName + ".json").data(), "wb");
	char writeBuffer[65536];
	FileWriteStream stream(fp, writeBuffer, sizeof(writeBuffer));

	Writer<FileWriteStream> writer(stream);

	writer.StartObject();
	writer.Key("Size");
	writer.Int(size);
	writer.Key("Data");
	writer.StartArray();
	for (int it = 0; it < area.size(); it++)
	{
		writer.Int(area[it]);
	}
	writer.EndArray();
	writer.EndObject();
	fclose(fp);
	std::cout << "Done!" << std::endl;
}