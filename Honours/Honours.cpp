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

//max players in the map
int maxPlayers = 8;

std::string fileName = "test";

enum TileType
{
	Empty,
	Floor,
	Wall,
	Cover,
	Spawn
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

//heat map image
sf::Image heatMap;

//heat map texture
sf::Texture heatTexture;

int roomTileSize = 5; //minumum size of a room measurement
int maxHeirarchy = 6; //generation variable to control how many rooms to make

//sfml rectangles to visualise the created level
std::vector<Room> rooms;

//sfml circles to visualise the spawn points
std::vector<sf::CircleShape> spawns;

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

//function to generate a heatmap for a room
void generateHeat(Room* room);

//function to insert a new room into the structure
void drawRoom(sf::Vector2f pos, sf::Vector2f size, Room::RoomType type);

//generate and parse a new level file of the currently displayed level
void outputFile();

int main()
{
	bool showHeatMap = false;

	//determine how big the map is going to be
	std::cout << "How large is the map area? 100x100 min" << std::endl;
	std::cin >> size;

	std::cout << "How many players will the map support?" << std::endl;
	std::cin >> maxPlayers;

	srand(time(NULL));

	//reserve the container in memory
	area.reserve(size*size);

	//generate the level
	generate();

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

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::H)
			{//press h to show the heatmap overlay
				showHeatMap = !showHeatMap;
			}
		}


		window.clear();
		for (int it = rooms.size() - 1; it > -1; it--)
			//for (int it = 0; it < rooms.size(); it++)
		{//draw all the room graphics
			window.draw(rooms[it].shape);
		}
		if (showHeatMap)
		{
			window.draw(heatSprite);
		}
		for (int it = 0; it < spawns.size(); it++)
		{
			window.draw(spawns[it]);
		}
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

	int roomCount = 0;

	Base base;
	//generate the initial room, no other room in the map can be bigger than this room
	base.generate(sf::Vector2f(center, center), size);
	base.arrayIndex = 0;

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
					case 3: //bottom edge
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
						roomCount++;
						temp.arrayIndex = roomCount;
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
				roomCount++;
				tempCorridor.arrayIndex = roomCount;
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
						roomCount++;
						tempCorridor.arrayIndex = roomCount;
						rooms.push_back(tempCorridor);
					}
				}
			}
		}

		roomIndex++;

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
	for (int it = 0; it < rooms.size(); it++)
	{
		if (rooms[it].type != Room::CorridorType)
		{// dont put cover in the corridors, only in the open areas
			std::cout << "Generating heat for room " + std::to_string(it) << std::endl;
			generateHeat(&rooms[it]);
		}
	}

	//Now we look over the map and assign room patterns to each room to be used for spawning cover and spawn points

	//determine the two rooms that are furthest away from each other, the rooms can't be corridors or the central room
	int a, b;
	float highestDistance = 0.f;

	for (int roomA = 0; roomA < rooms.size(); roomA++)
	{
		if (rooms[roomA].type == Room::NormalType)
		{
			for (int roomB = 0; roomB < rooms.size(); roomB++)
			{
				if (rooms[roomB].type == Room::NormalType)
				{
					//for each room, check the distance between each other room, recording the highest distance we find

					//get the center of each point to compare to
					sf::Vector2f posA, posB, dir;
					posA = rooms[roomA].shape.getPosition();
					posA.x += rooms[roomA].shape.getSize().x / 2.f;
					posA.y += rooms[roomA].shape.getSize().y / 2.f;

					posB = rooms[roomB].shape.getPosition();
					posB.x += rooms[roomB].shape.getSize().x / 2.f;
					posB.y += rooms[roomA].shape.getSize().y / 2.f;

					dir.x = posB.x - posA.x;
					dir.y = posB.y - posA.y;

					float dist = sqrt(dir.x * dir.x + dir.y * dir.y);

					if (dist > highestDistance)
					{
						a = roomA;
						b = roomB;
						highestDistance = dist;
					}
				}
			}
		}
	}

	//now we have the two furthest rooms apart, make them the team objective rooms
	rooms[a].pattern = Room::RoomPattern::Objective;
	rooms[b].pattern = Room::RoomPattern::Objective;

	rooms[a].shape.setFillColor(sf::Color::Red);
	rooms[b].shape.setFillColor(sf::Color::Blue);

	//the base should always be an arena as it is in the middle
	rooms[0].pattern = Room::RoomPattern::Arena;

	//now figure out what kind of room each of the other rooms are depending on the distance between them, the spawns and the middle

	//each objective room needs a chokepoint connected to them
	for (int it = 0; it < rooms.size(); it++)
	{
		if (rooms[it].pattern == Room::RoomPattern::Null && rooms[it].type != Room::RoomType::CorridorType)
		{//check if the room connects to either team base
			for (int conIt = 0; conIt < rooms[it].connections.size(); conIt++)
			{
				if (rooms[it].connections[conIt].roomIndex == a || rooms[it].connections[conIt].roomIndex == b)
				{
					rooms[it].pattern = Room::RoomPattern::ChokePoint;
					rooms[it].shape.setFillColor(sf::Color::Magenta);
					break;
				}
			}
		}
	}


	//each room that is connected to the center and not a chokepoint is an arena
	for (int it = 0; it < rooms.size(); it++)
	{
		if (rooms[it].pattern == Room::RoomPattern::Null && rooms[it].type != Room::RoomType::CorridorType)
		{
			for (int conIt = 0; conIt < rooms[it].connections.size(); conIt++)
			{
				if (rooms[it].connections[conIt].roomIndex == 0)
				{
					rooms[it].pattern = Room::RoomPattern::Arena;
					rooms[it].shape.setFillColor(sf::Color::Yellow);
				}
			}
		}
	}

	//everything else is a flank
	for (int it = 0; it < rooms.size(); it++)
	{
		if (rooms[it].type != Room::RoomType::CorridorType && rooms[it].pattern == Room::RoomPattern::Null)
		{
			rooms[it].pattern = Room::RoomPattern::Flank;
		}
	}

	int totalSpawnCount = 0;

	//now create an amount of spawn points in each room depending on what kind of room it is
	for (int it = 0; it < rooms.size(); it++)
	{
		int spawnCount = 0;
		if (rooms[it].type != Room::RoomType::CorridorType)
		{
			std::cout << "Generating spawns for room " + std::to_string(it);
			std::cout << std::endl;

			switch (rooms[it].pattern)
			{
			case Room::RoomPattern::Arena:
				spawnCount = 4;
				break;
			case Room::RoomPattern::ChokePoint:
				spawnCount = (rand() % 2) + 2;
				break;
			case Room::RoomPattern::Flank:
				spawnCount = rand() % 2;
				break;
			default:
				break;
			}

			//for each spawn needed to be generated
			if (spawnCount > 0)
			{
				//create a profile of the heat in the room
				struct RoomTile
				{
					float averageHeat = 0;
					sf::Vector2i pos;
					TileType type;
					bool isSource = false;
				};

				std::vector<RoomTile> room;
				std::vector<sf::CircleShape> generatedSpawns;

				sf::Vector2f roomSize = rooms[it].shape.getSize();
				sf::Vector2f roomPos = rooms[it].shape.getPosition();

				float minRoomHeat = 99999999999999;
				float maxRoomHeat = 0;
				float averageRoomHeat = 0;

				int numOfSources = 0;

				for (int y = 0; y < roomSize.y; y += roomTileSize)
				{
					for (int x = 0; x < roomSize.x; x += roomTileSize)
					{
						RoomTile newTile;
						newTile.pos.x = x + roomPos.x;
						newTile.pos.y = y + roomPos.y;
						newTile.type = area[index(newTile.pos)].tile;

						//get the average heat of the room tile
						for (int tileY = 0; tileY < roomTileSize; tileY++)
						{
							for (int tileX = 0; tileX < roomTileSize; tileX++)
							{
								newTile.averageHeat += area[index(roomPos.x + x + tileX, roomPos.y + y + tileY)].heat;
								if (area[index(roomPos.x + x + tileX, roomPos.y + y + tileY)].heat >= 999999999.f)
								{
									newTile.isSource = true;
									numOfSources++;
								}
							}
						}

						newTile.averageHeat /= roomTileSize * roomTileSize;

						room.push_back(newTile);

						//don't let the high heat value from a heat source muddy the room averages
						if (!newTile.isSource)
						{
							//record entire room temperature data
							if (newTile.averageHeat > maxRoomHeat)
							{
								maxRoomHeat = newTile.averageHeat;
							}
							else if (newTile.averageHeat < minRoomHeat)
							{
								minRoomHeat = newTile.averageHeat;
							}

							averageRoomHeat += newTile.averageHeat;
						}
					}
				}

				averageRoomHeat /= (room.size() - numOfSources);


				//now place spawn points in the room in the places with the lowest amount of relative heat
				for (int spawnIt = 0; spawnIt < spawnCount; spawnIt++)
				{
					//point to a random tile in the room
					int pointer = rand() % room.size();
					int initialPos = pointer;

					float targetHeat = minRoomHeat;
					bool foundTile = false;

					//check that the tile we are pointing to is a valid place to put a spawn point at
					do
					{
						//we can't put a spawn point inside a wall or in space >.>
						if (room[pointer].type != TileType::Wall && room[pointer].type != TileType::Empty)
						{
							//check the heat of the tile
							if (room[pointer].averageHeat <= targetHeat)
							{
								sf::CircleShape newSpawn;
								newSpawn.setPosition(sf::Vector2f(room[pointer].pos.x, room[pointer].pos.y));
								newSpawn.setRadius(1.f);
								newSpawn.setFillColor(sf::Color::Cyan);

								foundTile = true;

								//check the distance between the selected spawn and each other generated spawn
								for (int i = 0; i < generatedSpawns.size(); i++)
								{
									sf::Vector2f checkPos = generatedSpawns[i].getPosition();
									if (sqrt(std::pow(checkPos.x - newSpawn.getPosition().x, 2) + std::pow(checkPos.y - newSpawn.getPosition().y, 2)) < (roomTileSize * 2))
									{
										foundTile = false;
										break;
									}
								}

								//check that the selected tile is not too near a corridor entrance (heat source)
								if (foundTile)
								{
									for (int i = 0; i < room.size(); i++)
									{
										if (room[i].isSource)
										{
											sf::Vector2i checkPos = room[i].pos;
											if (sqrt(std::pow(checkPos.x - newSpawn.getPosition().x, 2) + std::pow(checkPos.y - newSpawn.getPosition().y, 2)) < (roomTileSize * 1.25f))
											{
												foundTile = false;
												break;
											}
										}
									}
								}

								//if we didn't find any spawn points too close then save the spawn
								if (foundTile)
								{
									generatedSpawns.push_back(newSpawn);
								}
							}
						}

						if (!foundTile)
						{
							//if we have not found a good tile on this point then move the pointer up by one
							pointer++;
							if (pointer >= room.size())
							{
								pointer = 0;
							}
						}

						//if we made a whole loop of the room increase the heat limit
						if (pointer == initialPos)
						{
							targetHeat += .2f;
						}

					} while (targetHeat < (maxRoomHeat - .2f) && !foundTile);
				}

				for (int i = 0; i < generatedSpawns.size(); i++)
				{
					spawns.push_back(generatedSpawns[i]);
				}
			}
		}
	}

	std::cout << "Rendering heatmap" << std::endl;

	//generate the heatmap image from the data structure
	heatMap.create(size, size, sf::Color::Black);
	sf::Color heatcolor = sf::Color::Red;

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
				heatcolor.r = (int)(heat * 25.f) + heatMap.getPixel(x, y).r;
				heatMap.setPixel(x, y, heatcolor);
			}
		}
	}
	heatMap.createMaskFromColor(sf::Color::Black);

	//load the generated map into a texture
	heatTexture.loadFromImage(heatMap);

	std::cout << "Done!" << std::endl;
}

void drawRoom(sf::Vector2f pos, sf::Vector2f size, Room::RoomType type)
{
	//save the room data into the final output
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
		newParticle.pos = sf::Vector2i(source->pos.x, source->pos.y);
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
						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.x -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.x += 2;
						generateParticle(nextPos, particles[particle]);
						break;

					case Room::Left:
						nextPos = particles[particle].pos;
						nextPos.x -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.y += 2;
						generateParticle(nextPos, particles[particle]);
						break;


					case Room::Right:
						nextPos = particles[particle].pos;
						nextPos.x += 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.y -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.y += 2;
						generateParticle(nextPos, particles[particle]);
						break;

					case Room::Down:
						nextPos = particles[particle].pos;
						nextPos.y += 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.x -= 1;
						generateParticle(nextPos, particles[particle]);

						nextPos.x += 2;
						generateParticle(nextPos, particles[particle]);
						break;

					default:
						break;
					}
				}
			}
		}

		//now record the particles to the data structure
		for (int count = 0; count < particles.size(); count++)
		{
			if (!particles[count].isSource)
			{
				area[index(particles[count].pos)].heat += particles[count].heat;
			}
			else
			{
				area[index(particles[count].pos)].heat += 999999999999.f;
			}
		}
	}
}