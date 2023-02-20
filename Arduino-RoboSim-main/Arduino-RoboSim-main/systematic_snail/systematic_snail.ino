#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>  

TaskHandle_t task_H;
SemaphoreHandle_t semphr;  
SemaphoreHandle_t semphr_collect_out;

TaskHandle_t task_Navigation_H;

struct CoordinatesI {
  int coordinates[4];
  bool flag;
}

struct GetNeighsI {
  int getNeighs[2];
  bool flag;
}

struct FinishedI {
  bool flag;
}

struct BackI {
  bool flag;
}

struct MoveI {
  bool flag
}

struct CheckSnailAndUpdateI {
  int x;
  int y;
  bool flag;
}

/////////////////////////////////////////////////////
TaskHandle_t task_Navigation_SNavigation_H;

struct Navigation_SNavigation_mem {
  int neighs[4]; 
  int location[2];
  int line;
  int column;
  bool cT;
  bool ceT; 
  bool op;
  GetNeighsI getNeighsI;
  BackI backI;
  MoveI moveI;
};

enum Navigation_SNavigation_STATE {
  Navigation_SNavigation_i, Navigation_SNavigation_SInit, Navigation_SNavigation_joint, Navigation_SNavigation_SMove, Navigation_SNavigation_F  
};

Navigation_SNavigation_STATE Navigation_SNavigation_state = Navigation_SNavigation_i;
bool Navigation_SNavigation_exec = false;

void Navigation_SNavigation_i_f() {
  Navigation_SNavigation_state = Navigation_SNavigation_SInit;
  return;
}

void Navigation_SNavigation_SInit_f() {
  mod.Navigation.SNavigation.getNeighsI.getNeighs = location;
  mod.Navigation.SNavigation.flag = true;
  Navigation_SNavigation_write(&mod.Navigation.SNavigation.getNeighs);
  Navigation_SNavigation_state = Navigation_SNavigation_joint;
  return;
}

//joint

void Navigation_SNavigation_SMove_f() {
  op = false; // flag that signals whether CheckSnailAndUpdate() must be called or not.
  res = NULL_sne; // res = SNE::NULL
  if (localCoordinatesFiware.neighs[0] == FREE) {
    debug(2, "SMove, neighs[0] == FREE, line - 1, DIR = NORTH");
    line = line - 1; 
    op = true;
    dir = NORTH;
  }
  else {
    if (localCoordinatesFiware.neighs[1] == FREE) {
      debug(2, "SMove, neighs[1] == FREE, column + 1, DIR = EAST");
      column = column + 1;
      op = true;
      dir = EAST; // dir = DIR::EAST
    }
    else {
      if (localCoordinatesFiware.neighs[2] == FREE) {
        debug(2, "SMove, neighs[2] == FREE, column - 1, DIR = WEST");
        column = column - 1;
        op = true;
        dir = WEST; // dir = DIR::WEST
      }
      else {
        if (localCoordinatesFiware.neighs[3] == FREE) {
          debug(2, "SMove, neighs[3] == FREE, line + 1, DIR = SOUTH");
          line = line + 1;
          op = true;
          dir = SOUTH;
        }
        else {
          if (localCoordinatesFiware.neighs[0] == VISITED) {
            debug(2, "SMove, neighs[0] == VISITED, line - 1, DIR = NORTH");
            line = line - 1;
            op = true;
            dir = NORTH;
          }
          else {
            if (localCoordinatesFiware.neighs[1] == VISITED) {
              debug(2, "SMove, neighs[1] == VISITED, column + 1, DIR = EAST");
              column = column + 1;
              op = true;
              dir = EAST; // dir = DIR::EAST
            }
            else {
              if (localCoordinatesFiware.neighs[2] == VISITED) {
                debug(2, "SMove, neighs[2] == VISITED, column - 1, DIR = WEST");
                column = column - 1;
                op = true;
                dir = WEST; // dir = DIR::WEST
              }
              else {
                if (localCoordinatesFiware.neighs[3] == VISITED) {
                  debug(2, "SMove, neighs[3] == VISITED, line + 1, DIR = SOUTH");
                  line = line + 1;
                  op = true;
                  dir = SOUTH;
                }
                else {
                  if (localCoordinatesFiware.neighs[0] == SNAIL_sne) {
                    debug(2, "SMove, neighs[0] == SNAIL_sne, line - 1, DIR = NORTH");
                    line = line - 1;
                    op = true;
                    dir = NORTH;
                  }
                  else {
                    if (localCoordinatesFiware.neighs[1] == SNAIL_sne) {
                      debug(2, "SMove, neighs[1] == SNAIL_sne, column + 1, DIR = EAST");
                      column = column + 1;
                      op = true;
                      dir = EAST; // dir = DIR::EAST
                    }
                    else {
                      if (localCoordinatesFiware.neighs[2] == SNAIL_sne) {
                        debug(2, "SMove, neighs[2] == SNAIL_sne, column - 1, DIR = WEST");
                        column = column - 1;
                        op = true;
                        dir = WEST; // dir = DIR::WEST
                      }
                      else {
                        if (localCoordinatesFiware.neighs[3] == SNAIL_sne) {
                          debug(2, "SMove, neighs[3] == SNAIL_sne, line + 1, DIR = SOUTH");
                          line = line + 1;
                          op = true;
                          dir = SOUTH;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    } // end of if (localCoordinatesFiware.neighs[0] == FREE) {...} else {
  } 
  if(op) { 
    debug(2, "SMove -> checkSnailAndUpdate(line,column)");
    move(vel,flight_time,dir); // $move(vel,time,dir)
    x_csau = line;
    y_csau = column;
    state = i_csau;
    exec = false;      
    return;
  }
  else {
    res = NOSNAIL;
    state = SMove_exit;
    exec = false; 
    return;
  }
}
