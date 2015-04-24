#ifndef PARSER_H
#define PARSER_H


typedef struct dungeon dungeon_t;
typedef struct character character_t;
typedef struct npc npc_t;

# ifdef __cplusplus


#include <cstdlib>
#include "dice.h"
#include "utils.h"

extern "C" {
# endif

//Non-system C headers


int parser_monsters(dungeon_t *d);

# ifdef __cplusplus
} /* extern "C" */


// class npc_obj{
// 	Dice m_dam;
// 	int m_speed;
// 	int m_hp;
// 	String name;
// 	String description;
// 	uint8_t color;
// 	uint8_t abil;
// 	char symbol;
// public:
// 	//Contructor
// 	npc_obj(Dice,int,int,String,String,uint8_t,uint8_t,char);
// 	//Getters
// 	int get_dam();
// 	int get_speed();
// 	int get_hp();
// 	char* get_name();
// 	char* get_description();
// 	uint8_t get_color();
// 	uint8_t get_abil();
// 	char get_symbol();
// };

# endif

#endif