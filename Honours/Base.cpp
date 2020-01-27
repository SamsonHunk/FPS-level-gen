#include "Base.h"

Base::Base()
{
	minSize.x = 50;
	minSize.y = 50;

	maxSize.x = 100;
	maxSize.y = 100;

	maxChildRooms = 10;
	minChildRooms = 4;
}

Base::~Base()
{
}
