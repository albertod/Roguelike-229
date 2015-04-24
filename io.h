#ifndef IO_H
# define IO_H

# include "dungeon.h"

void io_init_terminal(void);
void io_reset_terminal(void);
void io_calculate_offset(dungeon_t *d);
void io_display(dungeon_t *d);
void io_update_offset(dungeon_t *d);
void io_handle_input(dungeon_t *d);

int wear_item(dungeon_t *d);
int take_off_item_equipment(dungeon_t *d);
int drop_item(dungeon_t **d);
int expunge_item(dungeon_t *d);
uint32_t equip_possible(dungeon_t *d, int key);
uint32_t equip_item(dungeon_t *d, object_t **item);
#endif
