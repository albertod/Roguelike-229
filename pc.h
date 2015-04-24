#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"
# include "object.h"

typedef struct dungeon dungeon_t;


// typedef struct slot{

// 	object_t *object;
// 	uint32_t value;
// 	char* name;
// } slot_t;



typedef struct pc {
  char name[40];
  char catch_phrase[80];

  object_t *weapon;
  object_t *offhand;
  object_t *ranged;
  object_t *armor;
  object_t *helmet;
  object_t *cloak;
  object_t *gloves;
  object_t *boots;
  object_t *amulet;
  object_t *light;
  object_t *ring [2];

  //Equipment 
  object_t *equipment [10];

  //Store name and damage

} pc_t;

void pc_delete(pc_t *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);

#endif
