#pragma once
#include "Room.h"

//team base and basis of map generation

class Base: public Room
{
public:
	Base();
	~Base();

	bool generate();

private:

};