#include <gb\gb.h>
#include <stdio.h>
#include "data\groundtiles.c"
#include "data\maps.c"
#include "data\sprites.c"
#include <gb\font.h>
#include "\gbdk\libc\malloc.c"
#include "\gbdk\libc\free.c"
#define BLANK_SPRITE_ID  7


unsigned char * winmap;
UINT16 count_r;
UINT16 count_l;
UINT16 count_d;
UINT16 count_u;
UINT16 player_location[2];
UINT16 speed;
UINT16 proxtest;
UINT16 compteur_cycles;
UINT16 startbool;
font_t min_font;
UBYTE epeestat;
UBYTE prox;
const UINT8 sizelistepnj = sizeof(listepnj)/sizeof(Pnj);

UINT8 epee_coords[2] = {0,0};

const unsigned char * blanktext = "                                                            ";

typedef struct{
  unsigned char * label;
  UINT8 VRAMid;
  UINT8 spritids;
	UINT8 x;
	UINT8 y;
  UBYTE actif;
  unsigned char *code;
  unsigned char  * text;
  unsigned char  * text2;
  UINT8 nb_txt;
  UBYTE gentil;
  UBYTE vivant;
}Pnj;

typedef struct{
  unsigned char *zonemap;
  UBYTE actif;
  UINT8 updir;
  UINT8 downdir;
  UINT8 leftdir;
  UINT8 rightdir;
}Zone;


Pnj listepnj[] ={
   {"vioc",8,1,88,80,0,vioc,"BIENVENUE AVENTURIERIL FAIT FROID ICI   VA TE COUVRIR","BON VOICI UNE EPEE  ELLE POURRA PEUT    ETRE T ETRE UTILE",2,1,1},
   {"meuf",9,1,88,80,0,meuf,"BONJOUR MON CHER    VOUS ETES TRES      SEDUISANT","",1,1,1},
   {"enemi",10,1,88,80,0,enemi,"GOUME MOI AVANT QUE JE NE TE GOUME","",1,1,1}
}; //liste des pnjs

Zone listezone[] = {
  {ground,1,3,0,0,0},
  {room,0,0,0,0,0},
  {room2,0,0,0,0,0},
  {ground2,0,0,0,0,4},
  {ground3,0,0,0,3,0},
};

int strcmp(const char *s1, const char *s2) {
    char ret = 0;

    while (!(ret = *s1 - *s2) && *s2)
	++s1, ++s2;

    if (ret < 0)
	return -1;
    else if (ret > 0)
	return 1;
    return 0;
}

UBYTE can_player_move(UINT8 newplayerx, UINT8 newplayery){

  UINT16 indexTLx, indexTLy, tileindexTL;
  UBYTE result =1 ;
  UINT8 iter;
  indexTLx =(newplayerx -8) / 8;
  indexTLy =(newplayery -16) / 8;
  tileindexTL = 20*indexTLy + indexTLx;

  if(indexTLy<18 && indexTLx<20){
    if(listezone[0].actif){
      result = !(ground[tileindexTL] == 0x2c);
    }
    else if(listezone[3].actif){
        result = !(ground2[tileindexTL] == 0x2c);
    }
    else if(listezone[4].actif){
        result = !(ground3[tileindexTL] == 0x2c);
    }
    else if(listezone[1].actif){
        result = !(room[tileindexTL] == 0x2a);
    }
    else if(listezone[2].actif){
      result = !(room2[tileindexTL] == 0x2a);
    }
  }

  for(iter=0 ; iter<sizelistepnj;iter++){
    if(listepnj[iter].actif == 1){
      if(newplayerx == listepnj[iter].x && newplayery == listepnj[iter].y) result =0;
    }
  }

  return result;
}

UINT8 proximity(){
  UINT8 sizelistepnj = sizeof(listepnj)/sizeof(Pnj);
  UINT8 iter ;
    for(iter = 0 ; iter < sizelistepnj ; iter++){
      if(listepnj[iter].actif){
        if(listepnj[iter].gentil ==1){
          if( ( player_location[0] == (listepnj[iter].x + 8) && player_location[1] ==(listepnj[iter].y) )    || ( player_location[0] == (listepnj[iter].x - 8) && player_location[1] ==( listepnj[iter].y) ) || ( player_location[0] == (listepnj[iter].x ) && player_location[1] ==( listepnj[iter].y+8) )  || ( player_location[0] == (listepnj[iter].x && player_location[1] ==( listepnj[iter].y-8) ) ) ){
            return iter;
        }
      }
    }
  }
  return 5;
}

unsigned char get_player_direction(){
  if( get_sprite_tile(0)==0){
    return 'd';
  }
  else if( get_sprite_tile(0)==1){
    return 'r';
  }
  else if( get_sprite_tile(0)==2){
    return 'l';
  }
  else if( get_sprite_tile(0)==3){
    return 'u';
  }
}

UINT16 get_player_tile(){
  UINT16 indexTLx, indexTLy, tileindexTL;

  indexTLx =(player_location[0] -8) / 8;
  indexTLy =(player_location[1] -16) / 8;
  tileindexTL = 20*indexTLy + indexTLx;

  return tileindexTL;
}

void set_player_tile(UINT16 newtile){
  player_location[0] = (newtile %20)*8 + 8 ;


  player_location[1] = (newtile/20)*8 + 16;



  move_sprite(0,player_location[0],player_location[1]);
}

void set_pnj(Pnj * p){
  set_sprite_data(p->VRAMid,1,p->code);
  set_sprite_tile(p->spritids,p->VRAMid);
  move_sprite(p->spritids,p->x,p->y);
  p->actif = 1;
  SHOW_SPRITES;
}

void hide_pnj(Pnj * p){
  set_sprite_tile(p->spritids,BLANK_SPRITE_ID); //faire disparaitre le vieux

  p->actif =0;
}

void set_zone(Zone * z){
  UINT8 sizelistezone = sizeof(listezone)/sizeof(Zone);
  UINT8 iter;

  set_bkg_tiles(0,0,20,18,z->zonemap);
  for(iter = 0 ; iter < sizelistezone ; iter++){
    listezone[iter].actif =0;
  }
  z->actif = 1;
  SHOW_BKG;
}

void move_player(unsigned char dir){
    INT8 iter = 0;
    if(dir =='l'){

      iter = 0;
      player_location[0] -= 8;
      while(iter<8){
        set_sprite_tile(0,4);
        scroll_sprite(0,-1,0);
        iter++;
        delay(5/speed);
        set_sprite_tile(0,2);
        delay(5/speed);
      }
      //delay(100/speed);
    }
    else if(dir =='r'){
      //set_sprite_tile(0,1);
      iter = 0;
      player_location[0] += 8;
      while(iter<8){
        set_sprite_tile(0,0);
        scroll_sprite(0,1,0);
        iter++;
        delay(5/speed);
        set_sprite_tile(0,1);
        delay(5/speed);
      }
      //delay(100/speed);
    }
    else if(dir =='u'){
      iter = 0;
      player_location[1] -= 8;
      while(iter<8){
        set_sprite_tile(0,6);
        scroll_sprite(0,0,-1);
        iter++;
        delay(5/speed);
        set_sprite_tile(0,3);
        delay(5/speed);
      }
      //delay(100/speed);
    }
    else if(dir =='d'){

      iter = 0;
      player_location[1] += 8;
      while(iter<8){
        set_sprite_tile(0,5);
        scroll_sprite(0,0,1);
        iter++;
        delay(5/speed);
        set_sprite_tile(0,0);
        delay(5/speed);
      }
      //delay(100/speed);
    }
}

UINT8 get_active_zone(){
  UINT8 sizelistezone = sizeof(listezone)/sizeof(Zone);
  UINT8 iter2;
  for(iter2 = 0 ; iter2 < sizelistezone ; iter2++){
    if(listezone[iter2].actif ==1){
      return iter2;
    }
  }
}

UINT8 get_active_pnj(){
  UINT8 sizelistepnj = sizeof(listepnj)/sizeof(Pnj);
  UINT8 iter;
  for(iter = 0 ; iter < sizelistepnj ; iter++){
    if(listepnj[iter].actif ==1){
      return iter;
    }
  }
}

UBYTE detect_edge(UINT8 newplayerx, UINT8 newplayery){

  UINT8 updir = listezone[get_active_zone()].updir;
  UINT8 downdir = listezone[get_active_zone()].downdir;
  UINT8 leftdir = listezone[get_active_zone()].leftdir;
  UINT8 rightdir = listezone[get_active_zone()].rightdir;
  if(newplayery<16){

    set_zone(&listezone[updir]);
    player_location[1] = 152;
    move_sprite(0,player_location[0],player_location[1]);
    return 1;
  }
  if(newplayery>152){

    set_zone(&listezone[downdir]);
    player_location[1] = 16;
    move_sprite(0,player_location[0],player_location[1]+1);
    move_sprite(0,player_location[0],player_location[1]);
    return 1;
  }

  if(newplayerx<8){

    set_zone(&listezone[leftdir]);
    player_location[0] = 160;
    move_sprite(0,player_location[0],player_location[1]);
    return 1;
  }
  if(newplayerx>160){

    set_zone(&listezone[rightdir]);
    player_location[0] = 8;
    move_sprite(0,player_location[0],player_location[1]);
    return 1;
  }


  return 0;
}

unsigned char chr_to_hex(unsigned char c){
  switch (c) {
    case 'A': return 0x0c;
    case 'B': return 0x0d;
    case 'C': return 0x0e;
    case 'D': return 0x0f;
    case 'E': return 0x10;
    case 'F': return 0x11;
    case 'G': return 0x12;
    case 'H': return 0x13;
    case 'I': return 0x14;
    case 'J': return 0x15;
    case 'K': return 0x16;
    case 'L': return 0x17;
    case 'M': return 0x18;
    case 'N': return 0x19;
    case 'O': return 0x1a;
    case 'P': return 0x1b;
    case 'Q': return 0x1c;
    case 'R': return 0x1d;
    case 'S': return 0x1e;
    case 'T': return 0x1f;
    case 'U': return 0x20;
    case 'V': return 0x21;
    case 'W': return 0x22;
    case 'X': return 0x23;
    case 'Y': return 0x24;
    case 'Z': return 0x25;
    case ' ': return 0x26;

  }

}

void print_to_win(unsigned char * text ,UINT8 pos){
  unsigned char * hex_data = malloc(sizeof(unsigned char)*60);
  UINT8 iter = 0;

  while (text[iter] != '\0') {
    hex_data[iter] = chr_to_hex(text[iter]);
    iter++;
  }
    set_win_tiles(0,0,20,3,hex_data);
    move_win(7,pos);
    SHOW_WIN;
    SHOW_BKG;
    DISPLAY_ON;

  free(hex_data);
}

void hide_win_text(){
  print_to_win(blanktext,60,0);
  HIDE_WIN;
}

void show_message(Pnj * pnj){
  UINT8 iter = 0;
  UINT8 compteur = 0;
    waitpadup();
    print_to_win(pnj->text,120);
    waitpad(J_A);
    waitpadup();
    hide_win_text();

    if(pnj->nb_txt>1){
      print_to_win(pnj->text2,120);
      waitpad(J_A);
      waitpadup();
      hide_win_text();
    }

    if(strcmp(pnj->label , "vioc") ==0){
      epeestat = 1;
    }
    if(strcmp(pnj->label , "meuf") ==0 && epeestat == 1){
      print_to_win("OH UNE EPEE         IMPRESSIONNANT",120);
      waitpad(J_A);
      waitpadup();
      hide_win_text();

    }
    if(strcmp(pnj->label , "enemi") ==0){
      pnj->gentil =0;
    }

}

void detect_movement(){
  UBYTE edge_shift ;

  speed = 1;
  if(joypad() & J_B){
    speed = 4;
  }

  if((joypad() & J_LEFT) ){
    delay(5);

    set_sprite_tile(0,2);
    if(can_player_move(player_location[0]-8 ,player_location[1]) && count_l >=15){
      count_l = 0;
      edge_shift = detect_edge(player_location[0]-8 ,player_location[1]);
      if(!edge_shift){move_player('l');}
    }
    else{count_l += speed;}
  }
  else if(joypad() & J_RIGHT ){
    set_sprite_tile(0,1);
    if(can_player_move(player_location[0]+8,player_location[1]) && count_r >=15){
      count_r = 0;
      edge_shift = detect_edge(player_location[0]+8 ,player_location[1]);
      if(!edge_shift){move_player('r');}

    }
    else{count_r += speed;}
  }
  else if(joypad() & J_UP ){
    set_sprite_tile(0,3);
    if(can_player_move(player_location[0],player_location[1]-8) && count_u >=15){
      count_u = 0;
      edge_shift = detect_edge(player_location[0] ,player_location[1]-8);
      if(!edge_shift){move_player('u');}
    }
    else{count_u += speed;}
  }
  else if(joypad() & J_DOWN ){
      set_sprite_tile(0,0);

      if(can_player_move(player_location[0],player_location[1]+8) && count_d >=15){
        count_d = 0;
        edge_shift = detect_edge(player_location[0] ,player_location[1]+8);
        if(!edge_shift){move_player('d');}
      }
      else{count_d += speed;}
    }
}

void detect_zone_change(){
  if(listezone[0].actif == 1 && get_player_tile() == 117){
    set_zone(&listezone[1]);
    set_player_tile(243);
    set_pnj(&listepnj[0]);
    DISPLAY_ON;
  }

  if(listezone[0].actif == 1 && get_player_tile() == 303){
    set_zone(&listezone[2]);
    set_player_tile(228);
    set_pnj(&listepnj[1]);
    DISPLAY_ON;
  }

  if(listezone[1].actif == 1 && get_player_tile() == 242){
    set_zone(&listezone[0]);
    hide_pnj(&listepnj[0]);
    set_player_tile(137);
    DISPLAY_ON;
  }

  if(listezone[2].actif == 1 && get_player_tile() == 227){
    set_zone(&listezone[0]);
    hide_pnj(&listepnj[1]);
    set_player_tile(283);
    DISPLAY_ON;
  }
  if(listezone[4].actif == 1 && listepnj[2].vivant == 1){
    set_pnj(&listepnj[2]);
    DISPLAY_ON;
  }
  if(listezone[4].actif == 0 && listezone[2].actif == 0 && listezone[1].actif == 0){
    hide_pnj(&listepnj[2]);
  }
}

void detect_proximity(){
  proxtest = proximity();
  if(proxtest!=5){

    if((joypad() & J_A)){
      show_message(&listepnj[proxtest]);
    }

  }

}

void detect_start(){
  if(compteur_cycles%10 ==0){
    if(joypad() & J_START){
      if(startbool == 0){
        startbool=1;
      }
      else{
        startbool =0;
      }
    }
  }
}

void game_init(){
    font_init();
    min_font = font_load(font_min); // 36 tile
    font_set(min_font);
    set_win_data(1,1,virgule);

    print_to_win("   BEST GAME EVER        PRESS START    ",66);
    waitpad(J_START);
    waitpadup();
    hide_win_text();

    compteur_cycles = 0;
    startbool = 0;

    set_bkg_data(39,7,groundtiles);
    set_zone(&listezone[0]);
    set_sprite_data(0,7,bonhomme); /* starts loading sprites from the zeroth and loads 2 */
    set_sprite_tile(0,0);

    player_location[0] =88;
    player_location[1] =80;
    move_sprite(0,player_location[0],player_location[1]);
    SHOW_SPRITES;
    DISPLAY_ON;

}

void show_epee(){
  unsigned char dir = get_player_direction();
  set_sprite_data(10,4,epee); /* starts loading sprites from the zeroth and loads 2 */
  set_sprite_tile(2,10);
  if(dir == 'u'){
      set_sprite_tile(2,10);
      epee_coords[0] = player_location[0];
      epee_coords[1] = player_location[1]-8;

  }
  if(dir == 'd'){
      set_sprite_tile(2,11);
      epee_coords[0] = player_location[0];
      epee_coords[1] = player_location[1]+8;

  }
  if(dir == 'r'){
      set_sprite_tile(2,12);
      epee_coords[0] = player_location[0]+8;
      epee_coords[1] = player_location[1];

  }
  if(dir == 'l'){
      set_sprite_tile(2,13);
      epee_coords[0] = player_location[0]-8;
      epee_coords[1] = player_location[1];

  }

move_sprite(2,epee_coords[0],epee_coords[1]);
}

void hide_epee(){
  set_sprite_tile(2,BLANK_SPRITE_ID);
}

void detect_epee_collision(){
  UINT8 iter;
  for(iter=0 ; iter<sizelistepnj;iter++){
    if(listepnj[iter].actif == 1 && listepnj[iter].gentil == 0){
      if(epee_coords[0] <= listepnj[iter].x + 4 && epee_coords[0] >= listepnj[iter].x-4 && epee_coords[1] <= listepnj[iter].y + 4 && epee_coords[1] >= listepnj[iter].y-4) {
        hide_pnj(&listepnj[2]);
        listepnj[iter].vivant = 0;
      }
    }
  }

}
