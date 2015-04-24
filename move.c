#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "io.h"
#include "dice.h"

/* Ugly hack: There is no way to pass a pointer to the dungeon into the *
 * heap's comparitor funtion without modifying the heap.  Copying the   *
 * pc_distance array is a possible solution, but that doubles the       *
 * bandwidth requirements for dijkstra, which would also be bad.        *
 * Instead, make a global pointer to the dungeon in this file,          *
 * initialize it in dijkstra, and use it in the comparitor to get to    *
 * pc_distance.  Otherwise, pretend it doesn't exist, because it really *
 * is ugly.                                                             */
static dungeon_t *dungeon;

typedef struct path {
  heap_node_t *hn;
  uint8_t pos[2];
} path_t;

static int32_t dist_cmp(const void *key, const void *with) {
  return ((int32_t) dungeon->pc_distance[((path_t *) key)->pos[dim_y]]
                                        [((path_t *) key)->pos[dim_x]] -
          (int32_t) dungeon->pc_distance[((path_t *) with)->pos[dim_y]]
                                        [((path_t *) with)->pos[dim_x]]);
}

static int32_t tunnel_cmp(const void *key, const void *with) {
  return ((int32_t) dungeon->pc_tunnel[((path_t *) key)->pos[dim_y]]
                                      [((path_t *) key)->pos[dim_x]] -
          (int32_t) dungeon->pc_tunnel[((path_t *) with)->pos[dim_y]]
                                      [((path_t *) with)->pos[dim_x]]);
}

void dijkstra(dungeon_t *d)
{
  /* Currently assumes that monsters only move on floors.  Will *
   * need to be modified for tunneling and pass-wall monsters.  */

  heap_t h;
  uint32_t x, y;
  static path_t p[DUNGEON_Y][DUNGEON_X], *c;
  static uint32_t initialized = 0;

  if (!initialized) {
    initialized = 1;
    dungeon = d;
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        p[y][x].pos[dim_y] = y;
        p[y][x].pos[dim_x] = x;
      }
    }
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      d->pc_distance[y][x] = 255;
    }
  }
  d->pc_distance[d->pc.position[dim_y]][d->pc.position[dim_x]] = 0;

  heap_init(&h, dist_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) >= ter_floor) {
        p[y][x].hn = heap_insert(&h, &p[y][x]);
      }
    }
  }

  while ((c = heap_remove_min(&h))) {
    c->hn = NULL;
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn) &&
        (d->pc_distance[c->pos[dim_y] - 1][c->pos[dim_x] - 1] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y] - 1][c->pos[dim_x] - 1] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn) &&
        (d->pc_distance[c->pos[dim_y] - 1][c->pos[dim_x]    ] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y] - 1][c->pos[dim_x]    ] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn) &&
        (d->pc_distance[c->pos[dim_y] - 1][c->pos[dim_x] + 1] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y] - 1][c->pos[dim_x] + 1] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn) &&
        (d->pc_distance[c->pos[dim_y]    ][c->pos[dim_x] - 1] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y]    ][c->pos[dim_x] - 1] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn) &&
        (d->pc_distance[c->pos[dim_y]    ][c->pos[dim_x] + 1] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y]    ][c->pos[dim_x] + 1] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn) &&
        (d->pc_distance[c->pos[dim_y] + 1][c->pos[dim_x] - 1] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y] + 1][c->pos[dim_x] - 1] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn) &&
        (d->pc_distance[c->pos[dim_y] + 1][c->pos[dim_x]    ] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y] + 1][c->pos[dim_x]    ] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn) &&
        (d->pc_distance[c->pos[dim_y] + 1][c->pos[dim_x] + 1] >
         d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1)) {
      d->pc_distance[c->pos[dim_y] + 1][c->pos[dim_x] + 1] =
        d->pc_distance[c->pos[dim_y]][c->pos[dim_x]] + 1;
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn);
    }
  }
  heap_delete(&h);
}

void dijkstra_tunnel(dungeon_t *d)
{
  /* Currently assumes that monsters only move on floors.  Will *
   * need to be modified for tunneling and pass-wall monsters.  */

  heap_t h;
  uint32_t x, y;
  int size;
  static path_t p[DUNGEON_Y][DUNGEON_X], *c;
  static uint32_t initialized = 0;

  if (!initialized) {
    initialized = 1;
    dungeon = d;
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        p[y][x].pos[dim_y] = y;
        p[y][x].pos[dim_x] = x;
      }
    }
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      d->pc_tunnel[y][x] = 65535;
    }
  }
  d->pc_tunnel[d->pc.position[dim_y]][d->pc.position[dim_x]] = 0;

  heap_init(&h, tunnel_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        p[y][x].hn = heap_insert(&h, &p[y][x]);
      }
    }
  }

  size = h.size;
  while ((c = heap_remove_min(&h))) {
    if (--size != h.size) {
      exit(1);
    }
    c->hn = NULL;
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn) &&
        (d->pc_tunnel[c->pos[dim_y] - 1][c->pos[dim_x] - 1] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y] - 1][c->pos[dim_x] - 1] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn) &&
        (d->pc_tunnel[c->pos[dim_y] - 1][c->pos[dim_x]    ] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y] - 1][c->pos[dim_x]    ] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn) &&
        (d->pc_tunnel[c->pos[dim_y] - 1][c->pos[dim_x] + 1] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y] - 1][c->pos[dim_x] + 1] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn) &&
        (d->pc_tunnel[c->pos[dim_y]    ][c->pos[dim_x] - 1] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y]    ][c->pos[dim_x] - 1] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn) &&
        (d->pc_tunnel[c->pos[dim_y]    ][c->pos[dim_x] + 1] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y]    ][c->pos[dim_x] + 1] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn) &&
        (d->pc_tunnel[c->pos[dim_y] + 1][c->pos[dim_x] - 1] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y] + 1][c->pos[dim_x] - 1] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn) &&
        (d->pc_tunnel[c->pos[dim_y] + 1][c->pos[dim_x]    ] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y] + 1][c->pos[dim_x]    ] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn) &&
        (d->pc_tunnel[c->pos[dim_y] + 1][c->pos[dim_x] + 1] >
         d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60))) {
      d->pc_tunnel[c->pos[dim_y] + 1][c->pos[dim_x] + 1] =
        (d->pc_tunnel[c->pos[dim_y]][c->pos[dim_x]] + 1 +
         (d->hardness[c->pos[dim_y]][c->pos[dim_x]] / 60));
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn);
    }
  }
  heap_delete(&h);
}


uint32_t compute_damage_character(character_t **c){
  
  uint32_t damage = 0;
  if((*c)->pc){
   
    if((*c)->pc->weapon)   
    {
      damage += get_damage((*c)->pc->offhand);
    }
    if((*c)->pc->offhand)  damage += get_damage((*c)->pc->offhand);
    if((*c)->pc->ranged)   damage += get_damage((*c)->pc->ranged);
    if((*c)->pc->armor)    damage += get_damage((*c)->pc->armor);
    if((*c)->pc->cloak)    damage += get_damage((*c)->pc->cloak);
    if((*c)->pc->helmet)   damage += get_damage((*c)->pc->helmet);
    if((*c)->pc->gloves)   damage += get_damage((*c)->pc->gloves);
    if((*c)->pc->boots)    damage += get_damage((*c)->pc->boots);
    if((*c)->pc->amulet)   damage += get_damage((*c)->pc->amulet);
    if((*c)->pc->light)    damage += get_damage((*c)->pc->light);
    if((*c)->pc->ring[0])  damage += get_damage((*c)->pc->ring[0]);
    if((*c)->pc->ring[1])  damage += get_damage((*c)->pc->ring[1]);

    damage += roll_dice((*c)->damage);
  }
  //NPC
  else{
    damage += roll_dice((*c)->damage);
  }
  return damage;
}

void move_character(dungeon_t *d, character_t *c, pair_t next)
{
  d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
  pair_t previous;
  previous[dim_y] =  c->position[dim_y];
  previous[dim_x] =  c->position[dim_x];
  c->position[dim_y] = next[dim_y];
  c->position[dim_x] = next[dim_x];
  
  //Object Pick-UP
  //there is something were the character is going to move
  //Here we be handle the combat and the item pick-up
  if(d->object[c->position[dim_y]][c->position[dim_x]]){
    int i;
    for(i =0; i < 10; i++){
      if(!(d->pc.pc->equipment[i])){
        //add item to equipment available space
        d->pc.pc->equipment[i] = d->object[c->position[dim_y]][c->position[dim_x]];
        d->object[c->position[dim_y]][c->position[dim_x]] = NULL; //take out from map
        break;
      }
    }
  }

  //Both are NPC, so lets mode to another cell to avoid eating each other
  if(d->character[c->position[dim_y]][c->position[dim_x]] 
        && d->character[c->position[dim_y]][c->position[dim_x]]->npc 
          && c->npc){
      //we are going to just step back 
      if(d->hardness[previous[dim_y]++][previous[dim_x]++] > ter_floor){
        c->position[dim_y] = previous[dim_y]++;
        c->position[dim_x] = previous[dim_x]++;
      }else{
         c->position[dim_y] = previous[dim_y]--;
         c->position[dim_x] = previous[dim_x]--;
      }
      d->character[c->position[dim_y]][c->position[dim_x]] = c;
  }

  //IF POSITIVE THERE IS GONNA BE A FIGHT  NPC VS PC!! 
  //the movement of the character have to be handle inside of the if block
  else if (d->character[c->position[dim_y]][c->position[dim_x]]) {
    //FIGHT
    //d->character[c->position[dim_y]][c->position[dim_x]].hp - pc. rooll dice and fight.
    uint32_t damage = compute_damage_character(&c);
    d->character[c->position[dim_y]][c->position[dim_x]]->hp -= damage;
    if(d->character[c->position[dim_y]][c->position[dim_x]]->hp <= 0){
      d->character[c->position[dim_y]][c->position[dim_x]]->alive = 0;
      if (d->character[c->position[dim_y]][c->position[dim_x]] != &d->pc) 
      {
         d->num_monsters--;
      }
      d->character[c->position[dim_y]][c->position[dim_x]] = c;
    }
    else{

        if(d->character[c->position[dim_y]][c->position[dim_x]] == &d->pc)
        {
          //PC was attacked
          int32_t pc_health = d->character[c->position[dim_y]][c->position[dim_x]]->hp;
          io_display(d);
          mvprintw(23,0, "PC health is now %d",pc_health);
          refresh();
        }
        c->position[dim_y] = previous[dim_y];
        c->position[dim_x] = previous[dim_x];
        d->character[c->position[dim_y]][c->position[dim_x]] = c;
      return; // to avoid plavce the character in top of the monster 
    }
  }

  //nothing on the way, move character 
  else{
    d->character[c->position[dim_y]][c->position[dim_x]] = c;
  }
  
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character_t *c;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    heap_insert(&d->next_turn, &d->pc);
  }

  while (pc_is_alive(d) && ((c = heap_remove_min(&d->next_turn)) != &d->pc)) {
    if (!c->alive) {
      if (d->character[c->position[dim_y]][c->position[dim_x]] == c) {
        d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
      }
      if (c != &d->pc) {
        character_delete(c);
      }
      continue;
    }

    c->next_turn += (1000 / c->speed);

    npc_next_pos(d, c, next);
    move_character(d, c, next);

    heap_insert(&d->next_turn, c);
  }

  if (pc_is_alive(d) && c == &d->pc) {
    c->next_turn += (1000 / c->speed);
  }
}

void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (c->position[dim_x] != 1 && c->position[dim_x] != DUNGEON_X - 2) {
    dir[dim_x] = (c->position[dim_x] > DUNGEON_X - c->position[dim_x] ? 1 : -1);
  }
  if (c->position[dim_y] != 1 && c->position[dim_y] != DUNGEON_Y - 2) {
    dir[dim_y] = (c->position[dim_y] > DUNGEON_Y - c->position[dim_y] ? 1 : -1);
  }
}

uint32_t in_corner(dungeon_t *d, character_t *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(c->position[dim_x] - 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x] + 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] - 1) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

static void new_dungeon_level(dungeon_t *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir) {
  case '<':
  case '>':
    new_dungeon(d);
    break;
  default:
    break;
  }
}


//Update speed of pc depeing of the item gotten 
void update_speed(dungeon_t **d){

  (*d)->pc.speed = 10; // Base speed;
  if((*d)->pc.pc->weapon)   
  {
    (*d)->pc.speed += get_speed((*d)->pc.pc->weapon);
  }
  if((*d)->pc.pc->offhand)  (*d)->pc.speed += get_speed((*d)->pc.pc->offhand);
  if((*d)->pc.pc->ranged)   (*d)->pc.speed += get_speed((*d)->pc.pc->ranged);
  if((*d)->pc.pc->armor)    (*d)->pc.speed += get_speed((*d)->pc.pc->armor);
  if((*d)->pc.pc->cloak)    (*d)->pc.speed += get_speed((*d)->pc.pc->cloak);
  if((*d)->pc.pc->helmet)   (*d)->pc.speed += get_speed((*d)->pc.pc->helmet);
  if((*d)->pc.pc->gloves)   (*d)->pc.speed += get_speed((*d)->pc.pc->gloves);
  if((*d)->pc.pc->boots)    (*d)->pc.speed += get_speed((*d)->pc.pc->boots);
  if((*d)->pc.pc->amulet)   (*d)->pc.speed += get_speed((*d)->pc.pc->amulet);
  if((*d)->pc.pc->light)    (*d)->pc.speed += get_speed((*d)->pc.pc->light);
  if((*d)->pc.pc->ring[0])  (*d)->pc.speed += get_speed((*d)->pc.pc->ring[0]);
  if((*d)->pc.pc->ring[1])  (*d)->pc.speed += get_speed((*d)->pc.pc->ring[1]);

}

//Updates direction of pc 
//IO is done in io.c 
uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;

  next[dim_y] = d->pc.position[dim_y];
  next[dim_x] = d->pc.position[dim_x];

  switch (dir) {
  case 1:
  case 2:
  case 3:
    next[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    next[dim_y]--;
    break;
  }
  switch (dir) {
  case 1:
  case 4:
  case 7:
    next[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    next[dim_x]++;
    break;
  case '<':
    if (mappair(d->pc.position) == ter_stairs_up) {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (mappair(d->pc.position) == ter_stairs_down) {
      was_stairs = 1;
      new_dungeon_level(d, '>');
    }
    break;
  }

  if (was_stairs) {
    return 0;
  }

  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor)) {
    //Update speed of pc
    update_speed(&d);
    move_character(d, &d->pc, next);
    io_update_offset(d);
    dijkstra(d);

    return 0;
  }

  return 1;
}


