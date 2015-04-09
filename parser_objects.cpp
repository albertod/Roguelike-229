#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>

using std::cout;
using std::endl;
using std::ifstream;
using namespace std;

/*TYPE of object*/
# define WEAPON               0x00000001
# define OFFHAND              0x00000002
# define RANGED               0x00000004
# define ARMOR                0x00000008
# define HELMET               0x00000010
# define CLOAK                0x00000020
# define GLOVES               0x00000040
# define BOOTS                0x00000080
# define RING                 0x00000100
# define AMULET               0x00000200
# define LIGHT                0x00000400

/*not gear*/
# define SCROLL               0x00001000
# define BOOK                 0x00002000
# define FLASK                0x00004000
# define GOLD                 0x00008000
# define AMMUNITION           0x00010000
# define FOOD                 0x00020000
# define WAND                 0x00040000
# define CONTAINER            0x00080000

/* Colors */
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW  3
#define COLOR_BLUE  4
#define COLOR_MAGENTA 5
#define COLOR_CYAN  6
#define COLOR_WHITE 7

/*OBJECTS Fields (USED for checksum)*/ 
# define NAME              0x00000001
# define DESC              0x00000002
# define TYPE              0x00000004
# define COLOR             0x00000008
# define HIT               0x00000010
# define DAM               0x00000020
# define DODGE             0x00000040
# define DEF               0x00000080
# define WEIGHT            0x00000100
# define SPEED             0x00000200
# define ATTR              0x00000400
# define VAL               0x00000800

#define COMPLETE           (NAME  | DESC  |TYPE  | COLOR | HIT |DAM | DODGE |DEF | WEIGHT | SPEED | ATTR  |VAL )   

///Store Base, dice and sides as values
string name;
string desc;
string type;
string color;
string hit;
string dam;
string dodge;
string def;
string weight;
string speed;
string attr;
string val;
 
unsigned int checksum = 0;
string delimeter = " ";

int base_speed;
int dice_speed;
int side_speed;

int base_dam;
int dice_dam;
int side_dam;


int base_weight;
int dice_weight;
int side_weight;

int base_hit;
int dice_hit;
int side_hit;

int base_attr;
int dice_attr;
int side_attr;

int base_val;
int dice_val;
int side_val;

int base_dodge;
int dice_dodge;
int side_dodge;

int base_def;
int dice_def;
int side_def;


int color_npc;
int abil_npc; // type
int type_obj; // type
extern "C" int parser_objects();

int parser_objects()
{
  // create a file-reading object
  ifstream fin;
  const char* p = getenv("HOME");
  string path(p);
  path = path + "/.rlg229/object_desc.txt";
  fin.open(path.c_str());
  if (!fin.good())
    return 1; // exit if file not found

  string line;
  getline(fin, line);

  if (line.compare("RLG229 OBJECT DESCRIPTION 1") != 0)
  {
    std::cout << "Header Metadata is incorrect" << endl;
    return 0;
  }

  // While file continues lines
  while (!fin.eof())
  {
    while (line.compare("BEGIN OBJECT") != 0 && !fin.eof()) {
      getline(fin, line); //Conitnue until findint valide header
      //cout << line <<endl;
    }

    //Start checkSum until End is found, is checksum is not correct when "ERROR"
    //is found, then the monster is dropped
    while (!fin.eof() && (line.compare("END") != 0) && checksum != COMPLETE) {
      
      string parse_line = line.substr(0,line.find(delimeter));

      if (parse_line.compare("NAME") == 0) {
        name = line.substr(line.find(delimeter) + 1, line.find("\n"));
        checksum |= NAME;
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
        checksum |= SPEED;
      }
      else if (parse_line.compare("DAM") == 0) {
        dam = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = dam.substr(0,dam.find('+'));
        base_dam   = atoi(sb.c_str());
        string sd  = dam.substr(dam.find('+') + 1,dam.find('d') - 2);
        dice_dam = atoi(sd.c_str());
        string ss = dam.substr(dam.find('d')+1,dam.length()-1);
        side_dam = atoi(ss.c_str());
       
        checksum |= DAM;
      }

      //TEST BEGIN//// DONT USE WITHOUT TESTING 
      else if (parse_line.compare("WEIGHT") == 0) {
        weight = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = weight.substr(0,weight.find('+'));
        base_weight   = atoi(sb.c_str());
        string sd  = weight.substr(weight.find('+') + 1,weight.find('d') - 2);
        dice_weight = atoi(sd.c_str());
        string ss = weight.substr(weight.find('d')+1,weight.length()-1);
        side_weight = atoi(ss.c_str());
       
        checksum |= WEIGHT;
      }
      else if (parse_line.compare("HIT") == 0) {
        hit = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = hit.substr(0,hit.find('+'));
        base_hit   = atoi(sb.c_str());
        string sd  = hit.substr(hit.find('+') + 1,hit.find('d') - 2);
        dice_hit = atoi(sd.c_str());
        string ss = hit.substr(hit.find('d')+1,hit.length()-1);
        side_hit = atoi(ss.c_str());
       
        checksum |= HIT;
      }
      else if (parse_line.compare("ATTR") == 0) {
        attr = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = attr.substr(0,attr.find('+'));
        base_attr   = atoi(sb.c_str());
        string sd  = attr.substr(attr.find('+') + 1,attr.find('d') - 2);
        dice_attr = atoi(sd.c_str());
        string ss = attr.substr(attr.find('d')+1,attr.length()-1);
        side_attr = atoi(ss.c_str());
       
        checksum |= ATTR;
      }
      else if (parse_line.compare("VAL") == 0) {
        val = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = val.substr(0,val.find('+'));
        base_val   = atoi(sb.c_str());
        string sd  = val.substr(val.find('+') + 1,val.find('d') - 2);
        dice_val = atoi(sd.c_str());
        string ss = val.substr(val.find('d')+1,val.length()-1);
        side_val = atoi(ss.c_str());
       
        checksum |= VAL;
      }
      else if (parse_line.compare("DODGE") == 0) {
        dodge = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = dodge.substr(0,dodge.find('+'));
        base_dodge   = atoi(sb.c_str());
        string sd  = dodge.substr(dodge.find('+') + 1,dodge.find('d') - 2);
        dice_dodge = atoi(sd.c_str());
        string ss = dodge.substr(dodge.find('d')+1,dodge.length()-1);
        side_dodge = atoi(ss.c_str());
       
        checksum |= DODGE;
      }
      else if (parse_line.compare("DEF") == 0) {
        def = line.substr(line.find(delimeter) + 1, line.find("\n"));
        string sb  = def.substr(0,def.find('+'));
        base_def   = atoi(sb.c_str());
        string sd  = def.substr(def.find('+') + 1,def.find('d') - 2);
        dice_def = atoi(sd.c_str());
        string ss = def.substr(def.find('d')+1,def.length()-1);
        side_def = atoi(ss.c_str());
       
        checksum |= DEF;
      }
      //////END TEST (HAVE TO BE TESTED BEFORE) //////////////////
      
      else if (parse_line.compare("TYPE") == 0) {
        
        type = line.substr(line.find(delimeter) + 1, line.find("\n"));
        /*Here we setore the type field of the NPC on a int variable */
        string s;
        istringstream iss(type);
        while(getline(iss,s,' ')){

          if(s.compare("WEAPON") == 0) type_obj |= WEAPON;
          else if(s.compare("OFFHAND") == 0) type_obj |= OFFHAND ;
          else if(s.compare("RANGED") == 0) type_obj |= RANGED;
          else if(s.compare("ARMOR") == 0) type_obj |= ARMOR;
          else if(s.compare("HELMET") == 0) type_obj |= HELMET;
          else if(s.compare("CLOAK") == 0) type_obj |= CLOAK;
          else if(s.compare("GLOVES") == 0) type_obj |= GLOVES;
          else if(s.compare("BOOTS") == 0) type_obj |= BOOTS;
          else if(s.compare("RING") == 0) type_obj |= RING;
          else if(s.compare("AMULET") == 0) type_obj |= AMULET;
          else if(s.compare("LIGHT") == 0) type_obj |= LIGHT;
          
          else if(s.compare("SCROLL") == 0) type_obj |= SCROLL;
          else if(s.compare("BOOK") == 0) type_obj |= BOOK;
          else if(s.compare("FLASK") == 0) type_obj |= FLASK;
          else if(s.compare("GOLD") == 0) type_obj |= GOLD;
          else if(s.compare("AMMUNITION") == 0) type_obj |= AMMUNITION;
          else if(s.compare("FOOD") == 0) type_obj |= FOOD;
          else if(s.compare("WAND") == 0) type_obj |= WAND;
          else if(s.compare("CONTAINER") == 0) type_obj |= CONTAINER;

        }        
        checksum |= TYPE;
      }
      
      getline(fin, line, '\n'); //go to the next line
    }

    if(checksum == COMPLETE)
    {
       cout<<"\n"<<name+"\n"
       <<type<<"\n"
       <<color<<"\n" 
        <<"WEIGHT "<<base_weight<<'+'<<dice_weight<<'d'<<side_weight<<"\n"
        <<"HIT "<<base_hit<<'+'<<dice_hit<<'d'<<side_hit<<"\n"
        <<"DAM "<<base_dam<<'+'<<dice_dam<<'d'<<side_dam<<"\n"
        <<"ATTR "<<base_attr<<'+'<<dice_attr<<'d'<<side_attr<<"\n"
        <<"VAL "<<base_val<<'+'<<dice_val<<'d'<<side_val<<"\n"
        <<"DODGE "<<base_dodge<<'+'<<dice_dodge<<'d'<<side_dodge<<"\n"
        <<"DEF "<<base_def<<'+'<<dice_def<<'d'<<side_def<<"\n"
        <<"SPEED "<<base_speed<<'+'<<dice_speed<<'d'<<side_speed<<"\n"
        <<desc<<'.'<<endl;
    }
    
    /*the Npc have to be initialized before these lines*/ 
    checksum = 0; // reset checksum
    type_obj = 0;  //reset ability
  }

  return 0; // no problems given :) 
}
