#include "src\functions.c"
// the gameboy is 160 x 144

void main(){
    count_r = 0;
    count_l = 0;
    count_u = 0;
    count_d = 0;
    game_init();


    while(1){ //GAME LOOP

      //Start menu
      detect_start();

      //CHGTs de salles
      detect_zone_change();

      //detection de contact avec les pnjs et projectiles
      detect_proximity();

      if(epeestat){

        if((joypad() & J_A)){
            show_epee();
            detect_epee_collision();
            waitpadup();
            hide_epee();

          }
        }

      //MOUVEMENT
      detect_movement();

      if(compteur_cycles>=250){
        compteur_cycles=0;
      }
      compteur_cycles++;

      delay(10);
    }
}
