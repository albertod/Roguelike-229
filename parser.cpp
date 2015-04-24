#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
#include <sstream>
using std::ifstream;
using namespace std;

#include <cstring>
#include <string>
#include <cstdlib>




/* Colors */
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW  3
#define COLOR_BLUE  4
#define COLOR_MAGENTA 5
#define COLOR_CYAN  6
#define COLOR_WHITE 7

/*ABIL*/
# define NPC_SMART         0x00000001
# define NPC_TELEPATH      0x00000002
# define NPC_TUNNEL        0x00000004
# define NPC_PASS          0x00000008

/*NPC Fields (USED for checksum)*/ 
# define NAME              0x00000001
# define DESC              0x00000002
# define SYMB              0x00000004
# define COLOR             0x00000008
# define SPEED             0x00000010
# define ABIL              0x00000020
# define HP                0x00000040
# define DAM               0x00000080

#define COMPLETE           (NAME | SYMB | COLOR | ABIL | HP | DAM | SPEED | DESC)

using namespace std;




namespace monsters {

///Store Base, dice and sides as values
string name;
string color;
string symb;
string abil;
string speed;
string hp;
string dam;
string desc;

unsigned int checksum = 0;
string delimeter = " ";

int base_speed;
int dice_speed;
int side_speed;

int base_hp;
int dice_hp;
int side_hp;

int base_dam;
int dice_dam;
int side_dam;

int color_npc;
int abil_npc;
char symbol_result;



int32_t speed_result, hp_result, dam_result;


#include "dungeon.h"
#include "character.h"
#include "npc.h"


extern "C" {

int parser_monsters(dungeon_t *d)
{
  // create a file-reading object
  ifstream fin;
  const char* p = getenv("HOME");
  string path(p);
  path = path + "/.rlg229/monster_desc.txt";
  fin.open(path.c_str());
  if (!fin.good())
    return 1; // exit if file not found

  string line;
  getline(fin, line);

  if (line.compare("RLG229 MONSTER DESCRIPTION 1") != 0)
  {
    std::cout << "Header Metadata is incorrect" << endl;
    return 0;
  }

  // While file continues lines
  while (!fin.eof())
  {
    while (line.compare("BEGIN MONSTER") != 0 && !fin.eof()) {
      getline(fin, line); //Conitnue until findint valide header
      //cout << line <<endl;
    }

    //Start checkSum until End is found, is checksum is not correct when "ERROR"
    //is found, then the monster is dropped
    while (!fin.eof() && (line.compare("END") != 0) && checksum != COMPLETE) {
      

      //Create an instance of a Dice 
      dice *dice_object = new dice();
      dice *dam_dice = new dice();//to be used only by damage 

      string parse_line = line.substr(0,line.find(delimeter));

      if (parse_line.compare("NAME") == 0) {
        name = line.substr(line.find(delimeter) + 1, line.find("\n"));
        checksum |= NAME;
      }
      else if (parse_line.compare("SYMB") == 0) {
        symb = line.substr(line.find(delimeter) + 1, line.find("\n")); 
        symbol_result = symb[0];
        checksum |= SYMB;
      }
      else if (parse_line.compare("COLOR") == 0) {
        color = line.substr(line.find(delimeter) +1, line.find("\n"));

        /* Here we store the color in a variable for future use in an NCP */
        if(color.compare("BLACK") == 0) color_npc = COLOR_BLACK ;
        else if(color.compare("RED") == 0) color_npc = COLOR_RED;
        else if(color.compare("GREEN") == 0) color_npc = COLOR_GREEN;
        else if(color.compare("YELLOW") == 0) color_npc = COLOR_YELLOW;
        else if(color.compare("MAGENTA") == 0) color_npc = COLOR_MAGENTA ;
        else if(color.compare("CYAN") == 0) color_npc = COLOR_CYAN;
        else if(color.compare("WHITE") == 0) color_npc = COLOR_WHITE;
        else if(color.compare("BLUE") == 0) color_npc = COLOR_BLUE;

        checksum |= COLOR;
      }

      else if (parse_line.compare("DESC") == 0) {
        getline(fin,line);
        string cont_line;

        while(cont_line.compare(".") != 0)
        {
          getline(fin,cont_line,'\n');
          line = line + "\n" + cont_line;
        }
        desc = line.substr(0,line.length()-1);
        checksum |= DESC;
      }
      else if (parse_line.compare("SPEED") == 0 ) {
        speed = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = speed.substr(0,speed.find('+'));
        base_speed = atoi(sb.c_str());
        
        string sd  = speed.substr(speed.find('+') + 1,speed.find('d') - 2);
        dice_speed = atoi(sd.c_str());
      
        string ss = speed.substr(speed.find('d')+1,speed.length()-1);
        side_speed = atoi(ss.c_str());
        
        dice_object.set(base_speed,dice_speed,side_speed);
        speed_result = dice_object.roll();
        checksum |= SPEED;
      }
      else if (parse_line.compare("DAM") == 0) {
        speed = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = speed.substr(0,speed.find('+'));
        base_dam   = atoi(sb.c_str());
        string sd  = speed.substr(speed.find('+') + 1,speed.find('d') - 2);
        dice_dam = atoi(sd.c_str());
        string ss = speed.substr(speed.find('d')+1,speed.length()-1);
        side_dam = atoi(ss.c_str());
        dam_dice.set(base_dam,dice_dam,side_dam);

        checksum |= DAM;
      }
      else if (parse_line.compare("HP") == 0) {
        hp = line.substr(line.find(delimeter) + 1, line.find("\n"));        speed = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = speed.substr(0,speed.find('+'));
        base_hp   = atoi(sb.c_str());
        string sd  = speed.substr(speed.find('+') + 1,speed.find('d') - 2);
        dice_hp = atoi(sd.c_str());
        string ss = speed.substr(speed.find('d')+1,speed.length()-1);
        side_hp = atoi(ss.c_str());
        

        dice_object.set(base_hp,dice_hp,side_hp);
        hp_result = dice_object.roll();
        

        checksum |= HP;
      }
      else if (parse_line.compare("ABIL") == 0) {
        

        abil = line.substr(line.find(delimeter) + 1, line.find("\n"));
        /*Here we setore the abil field of the NPC on a int variable */
        string s;
        istringstream iss(abil);
        while(getline(iss,s,' ')){
          if(s.compare("SMART") == 0) abil_npc |= NPC_SMART;
          else if(s.compare("TELE") == 0) abil_npc |= NPC_TELEPATH ;
          else if(s.compare("TUNNEL") == 0) abil_npc |= NPC_TUNNEL;
          else if(s.compare("PASS") == 0) abil_npc |= NPC_PASS;
        }        
        checksum |= ABIL;
      }
      
      getline(fin, line, '\n'); //go to the next line
    }

    if(checksum == COMPLETE)
    {

      // cout<<name+"\n"<<desc<<symb+"\n" 
      //   <<color+"\n"<<base_speed <<'+'<<dice_speed<<'d'<<side_speed<<"\n"
      //   <<abil+"\n"<<base_hp<<'+'<<dice_hp<<'d'<<side_hp<<"\n"
      //   <<base_dam<<'+'<<dice_dam<<'d'<<side_dam<<"\n"<<endl;
    

        //call c funtion to add parapameters;
        character_t *m;
        uint32_t room;
        pair_t p;

      m = (character_t* )malloc(sizeof (*m));
      m->symbol = symbol_result;
      room = rand_range(1, d->num_rooms - 1);
      do {
        p[dim_y] = rand_range(d->rooms[room].position[dim_y],
          (d->rooms[room].position[dim_y] +
           d->rooms[room].size[dim_y] - 1));
        p[dim_x] = rand_range(d->rooms[room].position[dim_x],
          (d->rooms[room].position[dim_x] +
           d->rooms[room].size[dim_x] - 1));
      } while (d->character[p[dim_y]][p[dim_x]]);
      m->position[dim_y] = p[dim_y];
      m->position[dim_x] = p[dim_x];
      d->character[p[dim_y]][p[dim_x]] = m;
      m->speed = speed_result;
      m->next_turn = 0;
      m->alive = 1;
      m->sequence_number = ++d->character_sequence_number;
      m->pc = NULL;
      m->npc = ( npc_t* )malloc(sizeof (*m->npc));
      m->npc->characteristics = abil_npc;
      m->npc->have_seen_pc = 0;
      //new characteristics 
      m->npc->color = color_npc;
      m->npc->name  = name.c_str();
      m->npc->description = desc.c_str();
      m->npc->hp = hp_result;


      d->character[p[dim_y]][p[dim_x]] = m;
      heap_insert(&d->next_turn, m);


      //Results 
      // dam_dice;

    }
    
    /*the Npc have to be initialized before these lines*/ 
    checksum = 0; // reset checksum
    abil_npc = 0;  //reset ability
  }

  return 0; // no problems given :) 
 }

}

}

