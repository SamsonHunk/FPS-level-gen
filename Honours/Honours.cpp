// Honours.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <SFML/Graphics.hpp>
#include <iostream>

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

int roomTileSize = 4; //minumum size of a room measurement
int maxHeirarchy = 6; //generation variable to control how many rooms to make

//the structure that contains all the information each room needs when generated
//the sfml class can also store the position and size of the shape for use in the exporting too
struct Room
{
	int heirarchy;
	sf::RectangleShape shape;
	int parentIndex = -1;
	int sizeConstraint;

	sf::Vector2i tileDimensions;

	void generate(int minSize, int maxSize, sf::Vector2f pos)
	{//general generation rules for each room
		sf::Vector2f size;
		size.x = rand() % ((maxSize - minSize) * roomTileSize) + minSize * roomTileSize;
		size.y = rand() % ((maxSize - minSize) * roomTileSize) + minSize * roomTileSize;

		//convert the sze into room tile measurements, usefult for corridor generation
		tileDimensions.x = size.x / roomTileSize;
		tileDimensions.y = size.y / roomTileSize;

		shape.setSize(size);
		shape.setPosition(pos);

		sizeConstraint = maxSize;
	}
};

struct Corridor : public Room
{// a one tile long room to connect to other rooms together
	bool generateCorridor(Room* room0, Room* room1)
	{
		heirarchy = 0;
		bool dir[2];
		//true or false to determine which direction the other room is on the x and y axis
		dir[0] = (room0->shape.getPosition().x - room1->shape.getPosition().x <= 0); //true if the shape is on the right, else on the left
		dir[1] = (room0->shape.getPosition().y - room1->shape.getPosition().y <= 0); //true if the shape is below, else above

		sf::Vector2f target = room1->shape.getPosition(); //the target point on the other room we are trying to draw a room towards
		sf::Vector2f origin = room0->shape.getPosition();

		if (!dir[0])
		{//depending on where they are in relation to us, we want to choose the closest corner on the shape to us
			target.x += room1->shape.getSize().x;
		}

		if (!dir[1])
		{
			target.y += room1->shape.getSize().y;
		}


		//now we have a target corner point on the target shape we need to choose a point on the origin shape to draw the rectangle from
		bool foundX = false;
		for (int x = 0; x < room0->tileDimensions.x; x++)
		{
			if (origin.x + roomTileSize * x > target.x)
			{
				foundX = true;
				origin.x += x * roomTileSize;
				break;
			}
		}

		bool foundY = false;
		for (int y = 0; y < room0->tileDimensions.y; y++)
		{
			if (origin.y + roomTileSize * y > target.y)
			{
				foundY = true;
				origin.y += y * roomTileSize;
				break;
			}
		}

		//now we have an origin point and a direction to draw the corridor from let's try and draw a corridor now

		//we can now check if it is possible to draw a corridor
		if (foundX || foundY)
		{
			//try making either a vertical or horizontal corridor, check if either of them overlap
			if (dir[0])
			{
				//if we need to go to the right, make the origin the origin of the new shape, else make it the target
				shape.setSize(sf::Vector2f(target.x - origin.x + roomTileSize, roomTileSize));
				shape.setPosition(origin);
				shape.setFillColor(sf::Color::Red);

				if (shape.getGlobalBounds().intersects(room1->shape.getGlobalBounds()))
				{
					return true;
				}
			}
			else
			{
				shape.setSize(sf::Vector2f(origin.x - target.x + roomTileSize, roomTileSize));
				shape.setPosition(sf::Vector2f(target.x, origin.y));
				shape.setFillColor(sf::Color::Red);

				if (shape.getGlobalBounds().intersects(room0->shape.getGlobalBounds()))
				{
					return true;
				}
			}

			if (dir[1])
			{
				//they share the same x so we need a vertical corridor
				shape.setPosition(origin);
				shape.setSize(sf::Vector2f(roomTileSize, target.y - origin.y));
				shape.setFillColor(sf::Color::Red);

				if (shape.getGlobalBounds().intersects(room1->shape.getGlobalBounds()))
				{
					return true;
				}
			}
			else
			{
				shape.setFillColor(sf::Color::Red);
				shape.setPosition(sf::Vector2f(origin.x,target.y));
				shape.setSize(sf::Vector2f(roomTileSize, origin.y - target.y));
				if (shape.getGlobalBounds().intersects(room0->shape.getGlobalBounds()))
				{
					return true;
				}
			}

			std::cout << "Impossible Corridor" << std::endl;
			return false;
		}
		else
		{
			std::cout << "Rooms don't align to build corridor" << std::endl;
			return false;
		}
	}
};

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
	sf::RenderWindow window(sf::VideoMode(size,size), "Output");
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

				rooms.clear();

				//make a new one
				generate();
			}
		}

		window.clear();
		for (int it = 0; it < rooms.size(); it++)
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

	Room base;
	int generationHeriarchy = maxHeirarchy; //current position in the generation loop

	//generate the initial room, no other room in the map can be bigger than this room
	base.heirarchy = generationHeriarchy;
	base.generate(10, 8, sf::Vector2f(center, center));
	base.shape.setFillColor(sf::Color::Yellow);

	//center the first room into the center of the screen
	base.shape.setPosition(base.shape.getPosition().x - base.shape.getSize().x, base.shape.getPosition().y - base.shape.getSize().y);

	rooms.push_back(base);

	//index storage to improve efficiency moving through the vector
	int roomIndex = 0;

	std::cout << "Done!" << std::endl;

	do
	{//generate the rooms in a flower pattern around the parent room until we reach the end of the heirarchy loop
		Room temp;
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
					case 3: //bottom edge
						point.x += rand() % (int)roomSize.x;
						point.y += roomSize.y + roomTileSize;
						break;
					}
					
					
					//generate the shape with the chosen point in mind
					temp.generate(2, rooms[temp.parentIndex].sizeConstraint, point);

					roomSize = temp.shape.getSize();

					//we have to offset it again if it is abover or to the left of the parent, remember the origin of the shape is the top left corner
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
	/*
	generationHeriarchy = maxHeirarchy;
	roomIndex = 0;
	do
	{
		Corridor tempCorridor;
		if (rooms[roomIndex].parentIndex != -1)
		{//if the room has a parent, make a corridor connecting them together
			if (tempCorridor.generateCorridor(&rooms[roomIndex], &rooms[rooms[roomIndex].parentIndex]))
			{
				rooms.push_back(tempCorridor);
			}
		}
		roomIndex++;
		//dont draw corridors onto corridors
		generationHeriarchy = rooms[roomIndex].heirarchy;
	} while (generationHeriarchy != 0);
	*/
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
			if (x == 0 || x == size.x - 1 || y == 0 || y == size.y - 1)
			{
				area[index(cornerCoord[0] + x, cornerCoord[1] + y)] = Wall;
			}
			else
			{
				area[index(cornerCoord[0] + x, cornerCoord[1] + y)] = Floor;
			}
		}
	}
}