#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#include "dungeon.h"
#include "character.h"
#include "utils.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "io.h"
#include "parser.h"
#include "parser_objects.h"


// static void usage(char *name)
// {
//   fprintf(stderr,
//           "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]] [-s|--store]\n"
//           "%*c[-n|--nummon <number of monsters>]\n",
//           name, (int) strlen(name) + 8, ' ');

//   exit(-1);
// }

int main(int argc, char *argv[])
{

  //parser();
  parser_objects();


  // dungeon_t d;
  // time_t seed;
  // uint32_t nummon;
  // struct timeval tv;
  // uint32_t i;
  // uint32_t do_load, do_store, do_seed;
  // uint32_t long_arg;
  // char *save_file;

  // /* Default behavior: Seed with the time, generate a new dungeon, *
  //  * and don't write to disk.                                      */
  // do_load = do_store = 0;
  // do_seed = 1;
  // save_file = NULL;
  // nummon = 10;

  // /* The project spec requires '--load' and '--store'.  It's common *
  //  * to have short and long forms of most switches (assuming you    *
  //  * don't run out of letters).  For now, we've got plenty.  Long   *
  //  * forms use whole words and take two dashes.  Short forms use an *
  //  * abbreviation after a single dash.  We'll add '--rand' (to      *
  //  * specify a random seed), which will take an argument of its     *
  //  * own, and we'll add short forms for all three commands, '-l',   *
  //  * '-s', and '-r', respectively.  We're also going to allow an    *
  //  * optional argument to load to allow us to load non-default save *
  //  * files.  No means to store to non-default locations, however.   */
  // if (argc > 1) {
  //   for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
  //     if (argv[i][0] == '-') { /* All switches start with a dash */
  //       if (argv[i][1] == '-') {
  //         argv[i]++;    /* Make the argument have a single dash so we can */
  //         long_arg = 1; /* handle long and short args at the same place.  */
  //       }
  //       switch (argv[i][1]) {
  //       case 'r':
  //         if ((!long_arg && argv[i][2]) ||
  //             (long_arg && strcmp(argv[i], "-rand")) ||
  //             argc < ++i + 1 /* No more arguments */ ||
  //             !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
  //           usage(argv[0]);
  //         }
  //         do_seed = 0;
  //         break;
  //       case 'l':
  //         if ((!long_arg && argv[i][2]) || 
  //                 (long_arg && strcmp(argv[i], "-load")) ) {
  //           usage(argv[0]);
  //         }
  //         do_load = 1;
  //         if ((argc > i + 1) && argv[i + 1][0] != '-') {
  //           /* There is another argument, and it's not a switch, so *
  //            * we'll treat it as a save file and try to load it.    */
  //           save_file = argv[++i];
  //         }
  //         break;
  //       case 's':
  //         if ((!long_arg && argv[i][2]) ||
  //             (long_arg && strcmp(argv[i], "-save"))) {
  //           usage(argv[0]);
  //         }
  //         do_store = 1;
  //         break;
  //       case 'n':
  //         if ((!long_arg && argv[i][2]) ||
  //             (long_arg && strcmp(argv[i], "-nummon")) ||
  //             argc < ++i + 1 /* No more arguments */ ||
  //             !sscanf(argv[i], "%u", &nummon) /* Argument is not an int */) {
  //           usage(argv[0]);
  //         }
  //         break;
  //       default:
  //         usage(argv[0]);
  //       }
  //     } else { /* No dash */
  //       usage(argv[0]);
  //     }
  //   }
  // }

  // if (do_seed) {
  //   /* Allows me to generate more than one dungeon *
  //    * per second, as opposed to time().           */
  //   gettimeofday(&tv, NULL);
  //   seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  // }

  // printf("Seed is %ld.\n", seed);
  // srand(seed);

  // io_init_terminal();
  // init_dungeon(&d);

  // uint8_t flag = 0;
  
  // //file previously saved ... load it 
  // char *filename;
  // char *home;
  // size_t len;
  // if (!(home = getenv("HOME"))) {
  //   fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
  //   home = ".";
  // }
  // len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
  //          1 /* The NULL terminator */                               +
  //          2 /* The slashes */);

  //   filename = malloc(len * sizeof (*filename));
  //   sprintf(filename, "%s/%s/%s", home, SAVE_DIR, DUNGEON_SAVE_FILE);
  // if(access(filename,F_OK) != -1){
  //     do_load =1;
  // }
  
  // if (do_load) {
  //   read_dungeon(&d, save_file);
  //   printf("Dungeon Read!! Line 121 main.c\n");
  //   flag = 1;
  //   config_pc(&d,flag);
  //   gen_monsters(&d, nummon, 0, flag);
  // } else {
  //   gen_dungeon(&d);
  //   config_pc(&d,flag);
  //   gen_monsters(&d, nummon, 0, flag);
  // }
  

  // io_display(&d);
  // while (pc_is_alive(&d) && dungeon_has_npcs(&d) &&
  //        !(d.save_and_exit || d.quit_no_save)) {
  //   do_moves(&d);
  //   io_display(&d);
  //   if (!pc_is_alive(&d)) {
  //     break;
  //   }
  //   io_handle_input(&d);
  // }
  // io_display(&d);

  // io_reset_terminal();

  // if (pc_is_alive(&d) && dungeon_has_npcs(&d) == 0)  {
  //   printf("%s says, \"%s\"\n", d.pc.pc->name, d.pc.pc->catch_phrase);
  // } 
  // else if(pc_is_alive(&d) && dungeon_has_npcs(&d)){
  //   printf("Game Saved Automatically, come back soon!!.\n");
  // }
  // else {
  //   printf("The monster hordes growl menacingly.\n");
  //   remove(filename);
  // }

  // if (do_store || d.save_and_exit) {
  //   write_dungeon(&d);
  //   d.save_and_exit = 0;
  // }
  
  // free(filename); //free the filename
  // pc_delete(d.pc.pc);
  // delete_dungeon(&d);

  return 0;
}
