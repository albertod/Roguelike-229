#ifndef PARSER_OBJECTS_H
#define PARSER_OBJECTS_H


# ifdef __cplusplus

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "dice.h"

extern "C" {
# endif

int parser_objects();

# ifdef __cplusplus
} /* extern "C" */



// class object{
	
// 	Dice dam;
// 	int hit;
// 	int dodge;
// 	int def;
// 	int weight;
// 	int speed;
// 	int attr;
// 	int val;
// 	uint8_t color;
// 	unsigned int type;
// 	String name;
// 	String desc;
// public:
// 	//Contructor
// 	npc_obj(Dice,int,int,int.int,int,int,int,uint8_t,unsigned int,String,String);
// 	//Getters
// 	int get_dam();
// 	int get_speed();
// 	int get_dodge();
// 	int get_def();
// 	int get_weight();
// 	int get_attr();
// 	uint8_t get_color();
// 	unsigned int get_type();
// 	String get_name();
// 	String get_description();
// }

# endif

#endif
