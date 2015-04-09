#ifdef __cplusplus
extern "C" {
#endif

#ifndef IO_H
# define IO_H

# include "dungeon.h"

void io_init_terminal(void);
void io_reset_terminal(void);
void io_calculate_offset(dungeon_t *d);
void io_display(dungeon_t *d);
void io_update_offset(dungeon_t *d);
void io_handle_input(dungeon_t *d);

#endif

#ifdef __cplusplus
}
#endif