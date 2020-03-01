#include "Rooms.h"

void Base::generate(sf::Vector2f pos, int fieldSize)
{
	type = BaseType;

	shape.setFillColor(sf::Color::Yellow);

	int maxSize = fieldSize / 50;
	int minSize = (maxSize * 0.7);

	sf::Vector2f size;
	size.x = rand() % ((maxSize - minSize) * roomTileSize) + minSize * roomTileSize;
	size.y = rand() % ((maxSize - minSize) * roomTileSize) + minSize * roomTileSize;

	//generate a random starter room, the size based on the initial field size

	shape.setSize(size);
	shape.setPosition(pos);

	heirarchy = minSize;

	sizeConstraint = maxSize;
}

void NormalRoom::generate(sf::Vector2f pos, int maxSize)
{
	type = NormalType;

	int minSize = maxSize / 2;

	//generate a new room based on it's parent's size constraints
	sf::Vector2f size;
	size.x = rand() % ((maxSize - minSize) * roomTileSize) + minSize * roomTileSize;
	size.y = rand() % ((maxSize - minSize) * roomTileSize) + minSize * roomTileSize;

	shape.setSize(size);
	shape.setPosition(pos);

	sizeConstraint = maxSize;
}

bool Corridor::generate(Room * room0, Room * room1)
{
	type = CorridorType;

	shape.setFillColor(sf::Color::Red);

	heirarchy = -2;

	sf::Vector2f origin, target, originshape, targetshape, direction;

	origin = room0->shape.getPosition();
	target = room1->shape.getPosition();
	originshape = room0->shape.getSize();
	targetshape = room1->shape.getSize();

	//figure out the direction the target it to the origin
	direction.x = target.x - origin.x;
	direction.y = target.y - origin.y;

	//we don't need an accurate vector we just need to figure out if the room is top-left, or bottom-right, etc

	bool foundX = false;
	bool foundY = false;

	//determine a point on the origin's perimeter to draw the corridor from
	if (direction.x > 0)
	{
		for (int x = 0; x < originshape.x; x += roomTileSize)
		{
			if (origin.x + x > target.x && origin.x + x < target.x + targetshape.x)
			{
				//we found a point on our shape's x axis that overlaps the target
				foundX = true;
				origin.x += x + roomTileSize;
				break;
			}
		}
	}
	else
	{
		for (int x = originshape.x; x > 0; x -= roomTileSize)
		{
			if (origin.x + x > target.x && origin.x + x < target.x + targetshape.x)
			{
				//we found a point on our shape's x axis that overlaps the target
				foundX = true;
				origin.x += x - roomTileSize;
				break;
			}
		}
	}

	//do the same for the y axis
	if (direction.y > 0)
	{
		for (int y = 0; y < originshape.y; y += roomTileSize)
		{
			if (origin.y + y > target.y && origin.y + y < target.y + targetshape.y)
			{
				//we found a point on our shape's x axis that overlaps the target
				foundY = true;
				origin.y += y + roomTileSize;
				break;
			}
		}
	}
	else
	{
		for (int y = originshape.y; y > 0; y -= roomTileSize)
		{
			if (origin.y + y > target.y && origin.y + y < target.y + targetshape.y)
			{
				//we found a point on our shape's x axis that overlaps the target
				foundY = true;
				origin.y += y - roomTileSize;
				break;
			}
		}
	}

	if (!foundX)
	{
		//if we haven't found an x overlap point then set it to the corresponding edge
		if (direction.x > 0)
		{
			origin.x += originshape.x;
		}
	}

	if (!foundY)
	{
		if (direction.y > 0)
		{
			origin.y += originshape.y;
		}
	}

	//now we have a point to draw the shape from we will try and draw the corridor
	if (foundX)
	{
		//if we found an x overlap point then we can assume the right corridor to create is vertical
		if (direction.y > 0)
		{
			shape.setPosition(sf::Vector2f(origin.x, origin.y));
			shape.setSize(sf::Vector2f(roomTileSize, target.y - origin.y));
			shape.setFillColor(sf::Color::Red);
			room0->connectionsPos.push_back(origin);
			room1->connectionsPos.push_back(target);
			return true;
		}
		else
		{
			shape.setPosition(sf::Vector2f(origin.x, target.y));
			shape.setSize(sf::Vector2f(roomTileSize, origin.y - target.y));
			shape.setFillColor(sf::Color::Red);
			room0->connectionsPos.push_back(origin);
			room1->connectionsPos.push_back(target);
			return true;
		}
	}

	if (foundY)
	{
		//if we found an y overlap point then we can assume the right corridor to create is horizontal
		if (direction.x > 0)
		{
			shape.setPosition(sf::Vector2f(origin.x, origin.y));
			shape.setSize(sf::Vector2f(target.x - origin.x, roomTileSize));
			shape.setFillColor(sf::Color::Red);
			room0->connectionsPos.push_back(origin);
			room1->connectionsPos.push_back(target);
			return true;
		}
		else
		{
			shape.setPosition(sf::Vector2f(target.x, origin.y));
			shape.setSize(sf::Vector2f(origin.x - target.x, roomTileSize));
			shape.setFillColor(sf::Color::Red);
			room0->connectionsPos.push_back(origin);
			room1->connectionsPos.push_back(target);
			return true;
		}
	}
	return false;
}

