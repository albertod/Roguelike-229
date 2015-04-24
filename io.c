#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>

#include "io.h"
#include "move.h"
#include "object.h"
#include "pc.h"
#include "descriptions.h"

/* We're going to be working in a standard 80x24 terminal, and, except when *
 * the PC is near the edges, we're going to restrict it to the centermost   *
 * 60x18, defining the center to be at (40,12).  So the PC can be in the    *
 * rectangle defined by the ordered pairs (11, 4) and (70, 21).  When the   *
 * PC leaves this zone, if already on the corresponding edge of the map,    *
 * nothing happens; otherwise, the map shifts by +-40 in x or +- 12 in y,   *
 * such that the PC is in the center 60x18 region.  Look mode will also     *
 * shift by 40x12 blocks.  Thus, the set of all possible dungeon positions  *
 * to correspond with the upper left corner of the dungeon are:             *
 *                                                                          *
 *   ( 0,  0), (40,  0), (80,  0)                                           *
 *   ( 0, 12), (40, 12), (80, 12)                                           *
 *   ( 0, 24), (40, 24), (80, 24)                                           *
 *   ( 0, 36), (40, 36), (80, 36)                                           *
 *   ( 0, 48), (40, 48), (80, 48)                                           *
 *   ( 0, 60), (40, 60), (80, 60)                                           *
 *   ( 0, 72), (40, 72), (80, 72)                                           */

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();
}

void io_calculate_offset(dungeon_t *d)
{
  d->io_offset[dim_x] = ((d->pc.position[dim_x] - 20) / 40) * 40;
  if (d->io_offset[dim_x] < 0) {
    d->io_offset[dim_x] = 0;
  }
  if (d->io_offset[dim_x] > 80) {
    d->io_offset[dim_x] = 80;
  }
  d->io_offset[dim_y] = ((d->pc.position[dim_y] - 6) / 12) * 12;
  if (d->io_offset[dim_y] < 0) {
    d->io_offset[dim_y] = 0;
  }
  if (d->io_offset[dim_y] > 72) {
    d->io_offset[dim_y] = 72;
  }

#if 0
  uint32_t y;
  uint32_t min_diff, diff;

  min_diff = diff = abs(d->pc.position[dim_x] - 40);
  d->io_offset[dim_x] = 0;
  if ((diff = abs(d->pc.position[dim_x] - 80)) < min_diff) {
    min_diff = diff;
    d->io_offset[dim_x] = 40;
  }
  if ((diff = abs(d->pc.position[dim_x] - 120)) < min_diff) {
    min_diff = diff;
    d->io_offset[dim_x] = 80;
  }

  /* A lot more y values to deal with, so use a loop */

  for (min_diff = 96, d->io_offset[dim_y] = 0, y = 12; y <= 72; y += 12) {
    if ((diff = abs(d->pc.position[dim_y] - (y + 12))) < min_diff) {
      min_diff = diff;
      d->io_offset[dim_y] = y;
    }
  }
#endif
}

void io_update_offset(dungeon_t *d)
{
  int32_t x, y;

  x = (40 + d->io_offset[dim_x]) - d->pc.position[dim_x];
  y = (12 + d->io_offset[dim_y]) - d->pc.position[dim_y];

  if (x >= 30 && d->io_offset[dim_x]) {
    d->io_offset[dim_x] -= 40;
  }
  if (x <= -30 && d->io_offset[dim_x] != 80) {
    d->io_offset[dim_x] += 40;
  }
  if (y >= 8 && d->io_offset[dim_y]) {
    d->io_offset[dim_y] -= 12;
  }
  if (y <= -8 && d->io_offset[dim_y] != 72) {
    d->io_offset[dim_y] += 12;
  }
}

void io_display_tunnel(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvprintw(y, x*2, "%02hhx",
               d->pc_tunnel[y][x] <= 255 ? d->pc_tunnel[y][x] : 255);
    }
  }
  refresh();
}

void io_display_distance(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvprintw(y, x*2, "%02hhx", d->pc_distance[y][x]);
    }
  }
  refresh();
}

void io_display_huge(dungeon_t *d)
{
  uint32_t y, x;

  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (d->character[y][x]) {
        attron(COLOR_PAIR(d->character[y][x]->color));
        mvaddch(y, x, d->character[y][x]->symbol);
        attroff(COLOR_PAIR(d->character[y][x]->color));
      } else if (d->object[y][x]) {
        attron(COLOR_PAIR(get_color(d->object[y][x])));
        mvaddch(y, x, get_symbol(d->object[y][x]));
        attroff(COLOR_PAIR(get_color(d->object[y][x])));
      } else {
        switch (mapxy(x, y)) {
        case ter_wall:
        case ter_wall_no_room:
        case ter_wall_no_floor:
        case ter_wall_immutable:
          mvaddch(y, x, '#');
          break;
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
        case ter_floor_tunnel:
          mvaddch(y, x, '.');
          break;
        case ter_debug:
          mvaddch(y, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y, x, '>');
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(y, x, '0');
        }
      }
    }
  }
  refresh();
}

void io_display(dungeon_t *d)
{
  uint32_t y, x;

  if (d->render_whole_dungeon) {
    io_display_huge(d);
    return;
  }

  clear();
  for (y = 0; y < 24; y++) {
    for (x = 0; x < 80; x++) {
      if (d->character[d->io_offset[dim_y] + y]
                      [d->io_offset[dim_x] + x]) {
        attron(COLOR_PAIR(d->character[d->io_offset[dim_y] + y]
                                      [d->io_offset[dim_x] + x]->color));
        mvaddch(y, x, d->character[d->io_offset[dim_y] + y]
                                  [d->io_offset[dim_x] + x]->symbol);
        attroff(COLOR_PAIR(d->character[d->io_offset[dim_y] + y]
                                       [d->io_offset[dim_x] + x]->color));
      } else if (d->object[d->io_offset[dim_y] + y][d->io_offset[dim_x] + x]) {
        attron(COLOR_PAIR(get_color(d->object[d->io_offset[dim_y] + y]
                                             [d->io_offset[dim_x] + x])));
        mvaddch(y, x, get_symbol(d->object[d->io_offset[dim_y] + y]
                                          [d->io_offset[dim_x] + x]));
        attroff(COLOR_PAIR(get_color(d->object[d->io_offset[dim_y] + y]
                                              [d->io_offset[dim_x] + x])));
      } else {
        switch (mapxy(d->io_offset[dim_x] + x,
                      d->io_offset[dim_y] + y)) {
        case ter_wall:
        case ter_wall_no_room:
        case ter_wall_no_floor:
        case ter_wall_immutable:
          mvaddch(y, x, '#');
          break;
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
        case ter_floor_tunnel:
          mvaddch(y, x, '.');
          break;
        case ter_debug:
          mvaddch(y, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y, x, '>');
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(y, x, '0');
        }
      }
    }
  }
  mvprintw(0, 0, "PC position is (%3d,%2d); offset is (%3d,%2d).",
           d->pc.position[dim_x], d->pc.position[dim_y],
           d->io_offset[dim_x], d->io_offset[dim_y]);


  refresh();
}

void io_look_mode(dungeon_t *d)
{
  int32_t key;

  do {
    if ((key = getch()) == 27 /* ESC */) {
      io_calculate_offset(d);
      io_display(d);
      return;
    }
    
    switch (key) {
    case '1':
    case 'b':
    case KEY_END:
    case '2':
    case 'j':
    case KEY_DOWN:
    case '3':
    case 'n':
    case KEY_NPAGE:
      if (d->io_offset[dim_y] != 72) {
        d->io_offset[dim_y] += 12;
      }
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
    case '5':
    case ' ':
    case KEY_B2:
    case '6':
    case 'l':
    case KEY_RIGHT:
      break;
    case '7':
    case 'y':
    case KEY_HOME:
    case '8':
    case 'k':
    case KEY_UP:
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (d->io_offset[dim_y]) {
        d->io_offset[dim_y] -= 12;
      }
      break;
    }
    switch (key) {
    case '1':
    case 'b':
    case KEY_END:
    case '4':
    case 'h':
    case KEY_LEFT:
    case '7':
    case 'y':
    case KEY_HOME:
      if (d->io_offset[dim_x]) {
        d->io_offset[dim_x] -= 40;
      }
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
    case '5':
    case ' ':
    case KEY_B2:
    case '8':
    case 'k':
    case KEY_UP:
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
    case '6':
    case 'l':
    case KEY_RIGHT:
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (d->io_offset[dim_x] != 80) {
        d->io_offset[dim_x] += 40;
      }
      break;
    }
    io_display(d);
  } while (1);
}



//Handles input from user of the pc (moves it)
//the pc data (internal values is changed on move.c -> move_pc)
void io_handle_input(dungeon_t *d)
{
  uint32_t fail_code;
  int key;

  do {
    switch (key = getch()) {
    case 'w':
      //wear item
      wear_item(d);
      fail_code = 0;
      break;
    case 't':
      //take off item, promp equipment slot 
      take_off_item_equipment(d);
      fail_code = 0;
      break;
    case 'd':
      //drop item, prompts for carry slot
      drop_item(&d);
      fail_code = 0;
      break;
    case 'x':
      //expunge item from game, prompt carry slot, item is permantly deleted
      expunge_item(d);
      fail_code = 0;
      break;
    case '7':
    case 'y':
    case KEY_HOME:
      fail_code = move_pc(d, 7);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      fail_code = move_pc(d, 8);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      fail_code = move_pc(d, 9);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      fail_code = move_pc(d, 6);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      fail_code = move_pc(d, 3);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      fail_code = move_pc(d, 2);
      break;
    case '1':
    case 'b':
    case KEY_END:
      fail_code = move_pc(d, 1);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      fail_code = move_pc(d, 4);
      break;
    case '5':
    case ' ':
    case KEY_B2:
      fail_code = 0;
      break;
    case '>':
      fail_code = move_pc(d, '>');
      break;
    case '<':
      fail_code = move_pc(d, '<');
      break;
    case 'L':
      io_look_mode(d);
      fail_code = 0;
      break;
    case 'S':
      d->save_and_exit = 1;
      d->pc.next_turn -= (1000 / d->pc.speed);
      fail_code = 0;
      break;
    case 'Q':
      /* Extra command, not in the spec.  Quit without saving.          */
      d->quit_no_save = 1;
      fail_code = 0;
      break;
    case 'H':
      /* Extra command, not in the spec.  H is for Huge: draw the whole *
       * dungeon, the pre-curses way.  Doesn't use a player turn.       */
      io_display_huge(d);
      fail_code = 1;
      break;
    case 'T':
      /* New command.  Display the distances for tunnelers.  Displays   *
       * in hex with two characters per cell.                           */
      io_display_tunnel(d);
      fail_code = 1;
      break;
    case 'D':
      /* New command.  Display the distances for non-tunnelers.         *
       *  Displays in hex with two characters per cell.                 */
      io_display_distance(d);
      fail_code = 1;
      break;
    case 's':
      /* New command.  Return to normal display after displaying some   *
       * special screen.                                                */
      io_display(d);
      fail_code = 1;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      fail_code = 1;
    }
  } while (fail_code);
}

void print_menu(dungeon_t *d){
  int i;
  mvprintw(2,1, "CARRY SLOT                                           ");
  for(i =0; i < 10; i++ ){
     //DISPLAY CARRY SLOTS 
     if(d->pc.pc->equipment[i]){
        mvprintw(3+i,1, "%d.%s                                        ",i,get_name(d->pc.pc->equipment[i]));
      }
     else{
        mvprintw(3+i,1, "%d.                                          ",i);
     }
  }
  mvprintw(13,1, "                                                    ");
  mvprintw(14,1, "                                                    ");

  mvprintw(2,45, "Equipment      "); 
  if(d->pc.pc->weapon){
    mvprintw(3,45, "a.%s",get_name(d->pc.pc->weapon));
  }else{
    mvprintw(3,45, "a.          ");
  }
  if(d->pc.pc->offhand){
    mvprintw(4,45, "b.%s",get_name(d->pc.pc->offhand));
  }else{
    mvprintw(4,45, "b.          ");
  }
   if(d->pc.pc->ranged){
    mvprintw(5,45, "c.%s",get_name(d->pc.pc->ranged));
  }else{
    mvprintw(5,45, "c.          ");
  }
   if(d->pc.pc->armor){
    mvprintw(6,45, "d.%s",get_name(d->pc.pc->armor));
  }else{
    mvprintw(6,45, "d.          ");
  }
   if(d->pc.pc->helmet){
    mvprintw(7,45, "e.%s",get_name(d->pc.pc->helmet));
  }else{
    mvprintw(7,45, "e.          ");
  }
   if(d->pc.pc->cloak){
    mvprintw(8,45, "f.%s",get_name(d->pc.pc->cloak));
  }else{
    mvprintw(8,45, "f.          ");
  }
   if(d->pc.pc->gloves){
    mvprintw(9,45, "g.%s",get_name(d->pc.pc->gloves));
  }else{
    mvprintw(9,45, "g.          ");
  }
   if(d->pc.pc->boots){
    mvprintw(10,45, "h.%s",get_name(d->pc.pc->boots));
  }else{
    mvprintw(10,45, "h.         ");
  }
   if(d->pc.pc->amulet){
    mvprintw(11,45, "i.%s",get_name(d->pc.pc->amulet));
  }else{
    mvprintw(11,45, "i.         ");
  }
   if(d->pc.pc->light){
    mvprintw(12,45, "j.%s",get_name(d->pc.pc->light));
  }else{
    mvprintw(12,45, "j.         ");
  }
     if(d->pc.pc->ring[0]){
    mvprintw(13,45, "k.%s",get_name(d->pc.pc->ring[0]));
  }else{
    mvprintw(13,45, "k.         ");
  }
   if(d->pc.pc->ring[1]){
    mvprintw(14,45, "l.%s",get_name(d->pc.pc->ring[1]));
  }else{
    mvprintw(14,45, "l.         ");
  }
}

int wear_item(dungeon_t *d){

  //Implement method to try to wear item from carry 
  int key;
  uint32_t fail_code;
  io_display(d);
  print_menu(d);
  mvprintw(23,0, "Select a carry slot 0-9");
  do{
    switch(key = getch()){
      case 033:
        //User aborted command
        fail_code = 0;
        break;
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' :
        fail_code = equip_possible(d,key);
        break;
    }
  }while (fail_code);
  //Check which slot is the item the user wants to wear, if the user press esc (033) abort1
  return 0;
}


int take_off_item_equipment_helper(dungeon_t **d,int key){
  uint32_t fail_code = 0;
  int key_number = key - 97; // 97 is the offset of 'a' character
  uint8_t check_space = 0;
  uint8_t flag =0;
  //Cehck if there is space on carry
  for(check_space = 0; check_space < 12; check_space++){
      if(!((*d)->pc.pc->equipment[check_space])){
        flag = 1;
        break;
      }
  }
  if(!flag){
    //No space, tell the user to drop an item
    io_display(*d);
    mvprintw(23,0, "No space on carry slot, drop something");
    fail_code =1;
    return fail_code;
  }
  switch(key_number){
    case 0:
      if((*d)->pc.pc->weapon){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->weapon;
         (*d)->pc.pc->weapon = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No weapon equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 1:
      if(((*d))->pc.pc->offhand){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->offhand;
         (*d)->pc.pc->offhand = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No offhand equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 2:
      if(((*d))->pc.pc->ranged){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->ranged;
         (*d)->pc.pc->ranged = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No ranged equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 3:
      if(((*d))->pc.pc->armor){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->armor;
         (*d)->pc.pc->armor = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No armor equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 4:
      if(((*d))->pc.pc->helmet){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->helmet;
         (*d)->pc.pc->helmet = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No helmet equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 5:
      if(((*d))->pc.pc->cloak){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->cloak;
         (*d)->pc.pc->cloak = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No cloak equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 6:
      if(((*d))->pc.pc->gloves){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->gloves;
         (*d)->pc.pc->gloves = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No gloves equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 7:
      if(((*d))->pc.pc->boots){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->boots;
         (*d)->pc.pc->boots = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No boots equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 8:
      if(((*d))->pc.pc->amulet){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->amulet;
         (*d)->pc.pc->amulet = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No amulet equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 9:
      if(((*d))->pc.pc->light){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->light;
         (*d)->pc.pc->light = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No light equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 10:
      if(((*d))->pc.pc->ring[0]){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->ring[0];
         (*d)->pc.pc->ring[0] = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No ring[0] equiped, chose another or esc");
        fail_code = 1;
      }
      break;
    case 11:
      if(((*d))->pc.pc->ring[1]){
         (*d)->pc.pc->equipment[check_space] = (*d)->pc.pc->ring[1];
         (*d)->pc.pc->ring[1] = NULL;
      }else{
        io_display((*d));
        mvprintw(23,0, "No ring[1] equiped, chose another or esc");
        fail_code = 1;
      }
      break;
  }
  return fail_code;
}


int take_off_item_equipment(dungeon_t *d){

  int key;
  uint32_t fail_code; 
  io_display(d);
  print_menu(d);
  mvprintw(23,0, "Select a equipment slot a-l to switch to carry slot");
  do{
    switch(key = getch()){
      case 033:  
        //User aborted command
        fail_code = 0;
        break;
      case 'a' :
      case 'b' :
      case 'c' :
      case 'd' :
      case 'e' :
      case 'f' :
      case 'g' :
      case 'h' :
      case 'i' :
      case 'j' :
      case 'k' :
      case 'l' :
        fail_code = take_off_item_equipment_helper(&d,key);
        break;
      default  :
        io_display(d);
        mvprintw(23,0, "Select a correct slot or esc");
        fail_code = 1;
        break;
    }
  }while (fail_code);
  //Check which slot is the item the user wants to wear, if the user press esc (033) abort1
  return 0;
}


int drop_item(dungeon_t **d){

  io_display(*d);
  print_menu(*d);
  mvprintw(23,0, "Select a carry slot to drop 0-9");
  uint32_t fail_code; 
  do{
    int key;
    key = getch();
    if(key != 033){
      key = key - 48; // offset of '0'
    }
    switch(key){
      case 033:  
        //User aborted command
        fail_code = 0;
        break;
      case  0 :
      case  1 :
      case  2 :
      case  3 :
      case  4 :
      case  5 :
      case  6 :
      case  7 :
      case  8 :
      case  9 :
          if((*d)->pc.pc->equipment[key])
          {
            (*d)->object[(*d)->pc.position[dim_y]][(*d)->pc.position[dim_x]] = (*d)->pc.pc->equipment[key];
            (*d)->pc.pc->equipment[key] = NULL;
            fail_code = 0;
          }
          else{
            io_display(*d);
            mvprintw(23,0, "No item on that slot");
            fail_code = 1;
          }
        break;
      default:
        io_display(*d);
        mvprintw(23,0, "Select a correct slot or esc");
        fail_code = 1;
        break;
    }
  }while (fail_code);
  //Check which slot is the item the user wants to wear, if the user press esc (033) abort1
  return 0;

}

uint32_t expunge_item_helper(dungeon_t *d, int key){

  uint32_t fail_code = 0;
  int key_number = key - 48; // 48 is the offset of '0' character
  if(key_number < 0 || key_number > 9){
      //Invalided caracater
      io_display(d);
      mvprintw(23,0, "Invalid carry slot");
      fail_code = 1;
  }
  if(d->pc.pc->equipment[key_number]){
    free(d->pc.pc->equipment[key_number]);
    d->pc.pc->equipment[key_number] = NULL;
    fail_code = 0;
  }
  else{
    io_display(d);
    mvprintw(23,0, "No item in that slot, chose another or esc");
    fail_code = 1;
  }

  return fail_code;
}

int expunge_item(dungeon_t *d){

 int key;
  uint32_t fail_code; 
  io_display(d);
  print_menu(d);
  mvprintw(23,0, "Select a carry slot 0-9 to expunge item");
  do{
    switch(key = getch()){
      case 033:  
        //User aborted command
        fail_code = 0;
        break;
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' :
        fail_code = expunge_item_helper(d,key);
        break;
    }
  }while (fail_code);
  //Check which slot is the item the user wants to wear, if the user press esc (033) abort1
  return 0;
}

//Check if actions are possible 
uint32_t equip_possible(dungeon_t *d, int key){

  
  int key_number = key - 48; // 48 is the offset of '0' character
  if(key_number < 0 || key_number > 9){
    return 1;
    //Invalided caracater
    io_display(d);
    mvprintw(23,0, "Invalid carry slot");
  }
  if(!d->pc.pc->equipment[key_number]){
    //slot doesnt't have an item chose another
    //print message at the bottom of the screen saying that
    io_display(d);
    mvprintw(23,0, "Not item on carry slot");
    return 1; 
  }
  return equip_item(d,&d->pc.pc->equipment[key_number]);
  //If possible return 0 and equip;
  return 0;
}

uint32_t equip_item(dungeon_t *d, object_t **item){

  object_t *temp = malloc(sizeof(object_t*));

  if(get_type(*item) == objtype_WEAPON){
    temp = d->pc.pc->weapon;
    d->pc.pc->weapon = *item;
    *item = temp;
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_OFFHAND){
    temp = d->pc.pc->offhand;
    d->pc.pc->offhand = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_RANGED){
    temp = d->pc.pc->ranged;
    d->pc.pc->ranged = *item;
    *item = temp;

    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_ARMOR){
    temp = d->pc.pc->armor;
    d->pc.pc->armor = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_HELMET){
    temp = d->pc.pc->helmet;
    d->pc.pc->helmet = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_GLOVES ){
    temp = d->pc.pc->gloves;
    d->pc.pc->gloves = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_BOOTS){
    temp = d->pc.pc->boots;
    d->pc.pc->boots = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_CLOAK ){
    temp = d->pc.pc->cloak;
    d->pc.pc->cloak = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_AMULET){
    temp = d->pc.pc->amulet;
    d->pc.pc->amulet = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_LIGHT){
    temp = d->pc.pc->light;
    d->pc.pc->light = *item;
    *item = temp;
    temp = 0;
    free(temp);
    return 0;
  }
  if(get_type(*item) == objtype_RING){
    temp = d->pc.pc->ring[0];
    d->pc.pc->ring[0] = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  else if(get_type(*item) == objtype_RING)
  {
    temp = d->pc.pc->ring[1];
    d->pc.pc->ring[1] = *item;
    *item = temp;
    
    temp = 0;
    free(temp);
    return 0;
  }
  //Cant be added 
  io_display(d);
  mvprintw(23, 0, "Error, check alberto");
  temp = 0;
  free(temp);
  return 1;
}
