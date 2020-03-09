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
	Cover
};

//Data for each pixel of the map
struct AreaPixel
{
	TileType tile;
	float heat;
	Room::RoomType roomType;
};

//storage for data representation of generated level
std::vector<AreaPixel> area;

int roomTileSize = 5; //minumum size of a room measurement
int maxHeirarchy = 6; //generation variable to control how many rooms to make

//sfml rectangles to visualise the created level
std::vector<Room> rooms;

//small function to ease accessing and organising data in the vector
int index(int x, int y)
{
	return x + size * y;
}

int index(sf::Vector2i xy)
{
	return xy.x + size * xy.y;
}

//function encompassing the level generation
void generate();

//function to insert a new room into the structure
void drawRoom(sf::Vector2f pos, sf::Vector2f size, Room::RoomType type);

//generate and parse a new level file of the currently displayed level
void outputFile();

//create a heat map for a certain room
void generateHeat(Room* room);

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

	sf::Image heatMap;
	sf::Color heatcolor = sf::Color::Red;

	//generate the heatmap image from the data structure
	heatMap.create(size, size, sf::Color::Black);

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			float heat = area[index(x, y)].heat;
			if (heat >= 6969.f)
			{
				heatMap.setPixel(x, y, sf::Color::Magenta);
			}
			else
			{
				heatcolor.r = (int)(heat * 255.f);
				heatMap.setPixel(x, y, heatcolor);
			}
		}
	}
	heatMap.createMaskFromColor(sf::Color::Black);

	sf::Texture heatTexture;
	heatTexture.loadFromImage(heatMap);
	sf::Sprite heatSprite;
	heatSprite.setTexture(heatTexture);

	//render and show the final level
	sf::RenderWindow window(sf::VideoMode(size, size), "Output");
	sf::RenderWindow heatWindow(sf::VideoMode(size, size), "Heatmap");
	while (window.isOpen() && heatWindow.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				heatWindow.close();
			}

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::A)
			{//press the a button to make a new level
				//reset the level


				for (int it = 0; it < area.size(); it++)
				{
					area[it].tile = Empty;
					area[it].heat = 0;
					area[it].roomType = Room::Empty;
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
		//for (int it = 0; it < rooms.size(); it++)
		{//draw all the room graphics
			window.draw(rooms[it].shape);
		}
		window.draw(heatSprite);
		window.display();

		//display the heatmap
		heatWindow.clear(sf::Color::Transparent);
		heatWindow.draw(heatSprite);
		heatWindow.display();

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

	//write the rooms into the data structure and generate the walls

	//initialise the structure as empty
	area.clear();

	AreaPixel emptyPixel;
	emptyPixel.heat = 0;
	emptyPixel.tile = Empty;
	emptyPixel.roomType = Room::RoomType::Empty;

	for (int it = 0; it < area.capacity(); it++)
	{
		area.push_back(emptyPixel);
	}

	std::cout << "Generating walls" << std::endl;
	
	//first create bare floor of each room
	for (int it = 0; it < rooms.size(); it++)
	{
		drawRoom(rooms[it].shape.getPosition(), rooms[it].shape.getSize(), rooms[it].type);
	}
	
	//run edge detection and colour the edges in as walls
	for (int y = 0; y < size; y++)
	{
		for (int x = 1; x < size - 1; x++)
		{
			if ((area[index(x - 1, y)].tile == Empty || area[index(x + 1, y)].tile == Empty) && area[index(x, y)].tile == Floor)
			{
				area[index(x, y)].tile = Wall;
			}
		}
	}

	for (int x = 0; x < size; x++)
	{
		for (int y = 1; y < size - 1; y++)
		{
			if ((area[index(x, y - 1)].tile == Empty || area[index(x, y + 1)].tile == Empty) && area[index(x, y)].tile == Floor)
			{
				area[index(x, y)].tile = Wall;
			}
		}
	}
	
	//figure out a heat map of the map and place cover in areas to reduce that heat
	for (int it = 0; it < 1; it++)
	{
		if (rooms[it].type != Room::CorridorType)
		{// dont put cover in the corridors, only in the open areas
			std::cout << "Generating heatmap for room " << std::to_string(it) << std::endl;

			generateHeat(&rooms[it]);
		}
	}
}

void drawRoom(sf::Vector2f pos, sf::Vector2f size, Room::RoomType type)
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
			area[index(pos.x + x, pos.y + y)].tile = Floor;
			area[index(pos.x + x, pos.y + y)].roomType = type;
		}
	}
}

void outputFile()
{
	std::cout << "Writing file..." << std::endl;

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
		writer.Int(area[it].tile);
	}
	writer.EndArray();
	writer.EndObject();
	fclose(fp);
	std::cout << "Done!" << std::endl;
}

void generateHeat(Room* room)
{
	struct particle
	{
		sf::Vector2i pos;
		Room::Direction dir;
		float heat;
		bool isNew;
		bool isSource;
	};

	std::vector<particle> particles;

	//generate a heat map for each room defining which areas of the map are the most dangerous
	for (int it = 0; it < room->connections.size(); it++) //each room connection is a heat source
	{
		//reset the particles
		particles.clear();

		Room::Connection* source = &room->connections[it];

		//we only need to move the heat along the single room
		int travelDistance;

		if (source->dir == Room::Left || source->dir == Room::Right)
		{
			travelDistance = room->shape.getSize().x;
		}
		else
		{
			travelDistance = room->shape.getSize().y;
		}

		//generate the initial particle
		particle newParticle;
		newParticle.pos = sf::Vector2i(source->pos.x , source->pos.y);
		newParticle.dir = source->dir;
		newParticle.heat = 0.5f;
		newParticle.isNew = true;
		newParticle.isSource = true;

		sf::Vector2i nextPos;

		//lambda function for the particle generation
		auto generateParticle = [&](sf::Vector2i pos, particle parentParticle)
		{//rules for particle generation
			if (area[index(pos)].tile != Wall)
			{
				newParticle.heat = parentParticle.heat;
				newParticle.pos = pos;
				//check there is not another particle already there in the same place
				bool isUnique = true;
				for (int it = 0; it < particles.size(); it++)
				{
					if (newParticle.pos == particles[it].pos)
					{
						isUnique = false;
						break;
					}
				}

				if (isUnique)
				{
					particles.push_back(newParticle);
				}
			}
		};

		particles.push_back(newParticle);

		newParticle.isSource = false;

		for (int distance = 0; distance < travelDistance; distance++)
		{
			int currentParticleCount = particles.size();

			//for each new particle use the generation rules to figure out where to spawn the next particle
			for (int particle = 0; particle < currentParticleCount; particle++)
			{
				if (particles[particle].isNew)
				{
					particles[particle].isNew = false;

					//try and generate 3 new particles in a triangle pointing in the particle's direction
					switch (particles[particle].dir)
					{
					case Room::Up:
						nextPos = particles[particle].pos;
						nextPos.y += 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.x -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.x += 1;
						generateParticle(nextPos, particles[particle]);
						break;
					case Room::Left:
						nextPos = particles[particle].pos;
						nextPos.x += 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);
						break;
					case Room::Right:
						nextPos = particles[particle].pos;
						nextPos.x -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);
						break;
					case Room::Down:
						nextPos = particles[particle].pos;
						nextPos.y += 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.x -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos = particles[particle].pos;
						nextPos.x += 1;
						generateParticle(nextPos, particles[particle]);
						break;
					}
				}
			}
		}

		//now record the particles to the data structure
		for (int count = 0; count < particles.size(); count++) 
		{
			if (particles[count].isSource)
			{
				area[index(particles[count].pos)].heat = 6969.f;
			}
			else if (area[index(particles[count].pos)].heat != 6969.f)
			{
				area[index(particles[count].pos)].heat += particles[count].heat;
			}
		}
	}
}