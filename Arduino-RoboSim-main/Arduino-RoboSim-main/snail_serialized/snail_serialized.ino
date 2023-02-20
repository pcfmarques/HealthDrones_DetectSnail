/* 
V7
Wrapper
A wrapper implements separation of concerns (separation of naming space).

Changes the translation of inputs. 

A local variable has the same name as the name that appears in the model. And the environment variable that is sampled in the registerRead() receives a different name from the model.

This will make the translation easier to implement as all occurrences of inputs are translated easily.
*/
 
/* 
V7
Translation strategy: 1-to-1 mapping of RoboSim states and junctions to Arduino states. A flag exec controls whether the Arduino loop must iterate.
Serialization of Communication and Navigation
Call the actuators during exec. 
No registerWrite implemented. 
Communication is the main function
checkSnailAndUpdate() is an internal service.
*/

//#include <Time.h>
#include <Connection.h> //mudar nome da biblioteca para comunication.h
// STATE. RoboSim states and junctions, where csau means CheckSnailAndUpdate and 
// joints are numbered as they appear in the model from left to right. 
enum STATE {
  SInit_snail, SConnecting, SConnected, i_snail, SMove_entry, SMove_exit, i_csau, SInit_csau, HasSnail, NoSnail, joint1, joint2, joint3, joint4, final_snail, final_csau 
};

// ST. State at a location.
enum ST {
  FREE, VISITED, OBSTACLE, DRONE, SNAIL_st
};

// SNE. Snail - No Snail - Error
enum SNE {
  SNAIL_sne, NOSNAIL, ERROR_sne, NULL_sne
}; //tive que mudar ERROR e NULL

// DIR. Direction
enum DIR {
  WEST, EAST, NORTH, SOUTH
};

// EVENT. Represents an event (like connectAck).
struct EVENT {
  bool comm;
};

// MYLOCATION. The myLocation channel
struct MYLOCATION {
  bool comm;
  int line;
  int column;
};

// COORDINATESFIWARE. The coordinatesFiware channel
struct COORDINATESFIWARE {
  bool comm;
  int *neighs; // neighs: St * St * St * St
};

// CHECKSNAILSV. The checkSnailSv channel
struct CHECKSNAILSV {
  bool comm;
  int id;
};

// CHECKEDSNAILSVACK. The checkedSnailSvAck channel.
struct CHECKEDSNAILSVACK {
  bool comm;
  bool hasSnail;
};

// UPDATEFIWARE. The updateFiware channel
struct UPDATEFIWARE {
  bool comm;
  int x;
  int y;
  ST state;
};

// SENDURL. The sendURL channel
struct SENDURL {
  bool comm;
  String urlCamera;
};

bool exec;
STATE state;

EVENT connectAck, localConnectAck, updateAck, localUpdateAck;
COORDINATESFIWARE coordinatesFiware, localCoordinatesFiware;
//IMAGE getImage, localGetImage;
MYLOCATION myLocation; // myLocation is output only; no need of a localMyLocation.
UPDATEFIWARE updateFiware; // updateFiware is output only; no need of a localUpdateFiware.
CHECKEDSNAILSVACK checkedSnailSvAck, localCheckedSnailSvAck;
CHECKSNAILSV checkSnailSv;
SENDURL sendURL;
Connection connection("ERALUJA","83231749","192.168.1.80","1026"); //ssid(ERAJUJA) and password (83231749) for testing

// Events use 2 variables each. One is connected to a port, and the other is a local variable 
// assigned (sampled) at registerRead() time and used later during run_state_machine().

// A transition [e]/a, where e is an event and a is an action, is translated to:
// void registerRead() {
//   ...
//   localE.comm = e.comm;
//   ...
// }
// ...
// }
// void run_state_machine() {
//   ...
//   if (localE.comm) {
//     ... // translation of a
//   }
// }

// A transition [g]/e, where g is a guard and e is an event, is translated to:
// void run_state_machine() {
//   ...
//   if (g) {
//     ...
//     e.comm = true;
//     ...
//   }
// }


// A transition [c?x]/a, where c?x is an event and a is an action, is translated to:
// void registerRead() {
//   ...
//   localC.comm = c.comm;
//   if (localC.comm) {
//      localC.x = c.x;
//   }
//   ...
// }
// void run_state_machine() {
//   ...
//   if (localC.comm) {
//     ... localC.x ... // translation of a with use of x
//   }
// }

// A transition [g]/c!v, where g is a guard and c!v is an event, is translated to:
// void run_state_machine() {
//   ...
//   if (g) {
//     ...
//     c.comm = true;
//     c.x = v;
//     ...
//   }
// }

//Debug
int DEBUG_LEVEL = 2;
void debug(int level, String msg);
bool verificaBotao();

unsigned long baseTime = 5000; 

long startCycleTime;
long endOperCycleTime; 
long cycleTime = 1*baseTime;

// local variables
int line = 1;
int column = 1;
int timeout = 15; // Raj::original: unsoigned long timeout = 3
int maxAttempt = 3;
bool cfT = true;
bool caT = true;
bool csT = true;
bool giT = true;
bool upT = true;
bool op = false;
int count = 1;
unsigned long Clk;
String urlFiware = "http:"; // jmi: to be defined
String urlCamera = "http:"; // jmi: to be defined
DIR dir;
long vel = 0; // jmi: to be defined 
long flight_time = 0; // jmi: to be defined
int id = 0; // jmi: to be defined

int x_csau; // x_csau and y_csau are parameters of CheckSnailAndUpdate
int y_csau;
SNE res; // result from CheckSnailAndUpdate, where res \in {SNAIL_sne, NOSNAIL, ERROR}


// operations backHome() and getSnail()
void backHome() {
  debug(2,"backHome()");
  // send drone back home
}

void getSnail() {
  debug(2,"getSnail(), initiate the gripper");
  // initiate the gripper
}

// set ports and define the initial state
void setup(){
  delay(4000);
  pinMode(5, INPUT_PULLUP);
  Serial.begin(115200);
  //Serial.printf("timeout: %i", timeout);
  //Serial.println();
  initState();
}

void initState() {
   state = i_snail;
}

EVENT connectAckWrapper() {
  EVENT wrapper;
  wrapper.comm = connection.connectedWifi();
  return wrapper;
}

COORDINATESFIWARE coordinatesFiwareWrapper() {
  COORDINATESFIWARE wrapper;
  wrapper.comm = connection.coordinatesFiwareAck();
  if (wrapper.comm) {
    wrapper.neighs = connection.coordinatesFiware();
  }
  return wrapper;
}

CHECKEDSNAILSVACK checkedSnailSvAckWrapper() {
  CHECKEDSNAILSVACK wrapper;
  wrapper.comm = verificaBotao();
  if (wrapper.comm) {
  wrapper.hasSnail = verificaBotao();
  }
  return wrapper;
}

EVENT updateAckWrapper() {
  EVENT wrapper;
  wrapper.comm = connection.updateAck();
  return wrapper;
}

// registerRead() samples the input ports and assigns their values to local variables.
void registerRead() {
  debug(2,"registerRead");
  
  localConnectAck = connectAckWrapper();
  localCoordinatesFiware = coordinatesFiwareWrapper();
  localCheckedSnailSvAck = checkedSnailSvAckWrapper();
  localUpdateAck = updateAckWrapper();
  dump();
}

void i_snail_f() {
  debug(2,"i_snail_f(), state -> SInit_snail");
  state = SInit_snail;
  exec = false;
  return;
}

void connect_c() {
  debug(2,"connect_c(), call connect function");
  // call Sidney+Raj’s function
  connection.connectWifi();
}

void SInit_snail_f() {
  debug(2,"SInit -> SConnecting, /$connect; #Clk; count=1");
  connect_c(); // $connect
  Clk = millis(); // #Clk
  count = 1;
  state = SConnecting;
  exec = false;
  return;
}

bool connectAck_c() {
  return localConnectAck.comm;
}

int since_Clk() { // Raj:: originalmente estava retornando um boleano
  //return (millis() - Clk); // return (now - Clk)
  return ((millis() - Clk)/1000); 
}

void reset_Clk() {
  Clk = millis(); // Clk = now
}

void sendURL_out_c(String urlCamera) {
  sendURL.comm = true;
  sendURL.urlCamera = urlCamera;
}

void SConnecting_f() {
  if (!((connectAck_c() && caT) || (since_Clk() > timeout && count <= maxAttempt) || (count > maxAttempt))) {
    debug(2,"SConnecting -> SConnecting, [not(($connectAck/\\caT)\\/(since(Clk)>timeout/\\count<=maxAttempt)\\/(count>maxAttempt))]/caT=true; cft=true; csT=true; exec");
    caT = true;
    cfT = true;
    csT = true;
    state = SConnecting;
    exec = true;
    return; 
  }
  if (since_Clk() > timeout && count <= maxAttempt) {
    debug(2, "Sconnecting -> SConnecting, [since(Clk)>timeout/\\count<=maxAttempt]/#Clk; count=count+1");
    reset_Clk(); // #Clk
    count = count + 1;
    state = SConnecting;
    exec = false; 
    return; 
  }
  if (connectAck_c()) {
    debug(2,"SConnecting -> Sconnected, [$connectAck]/caT = false; sendURL!urlCamera");
    caT = false;
    sendURL_out_c(urlCamera); // $sendURL!urlCamera
    state = SConnected;
    exec = false;
    return; 
  }
  if (count > maxAttempt) {
    debug(2,"Sconnecting -> F, [count > maxAttempt]/$backHome()");
    backHome();
    state = final_snail;
    exec = false;
    return; 
  }
}

void myLocation_c(int l, int c) { // jmi: revisar se o envio de myLocation!(line,column) é 
                                  // feito assim, gravando em portas.
  myLocation.comm = true;
  myLocation.line = l;
  myLocation.column = c;
  connection.myLocation(l,c); //connection.getNeighbours(l,c)
}

void SConnected_f() {
  debug(2,"SConnected -> joint1, entry $myLocation(|line,column|), /count=1; #Clk");
  myLocation_c(line,column); // entry myLocation(|line,column|)
  reset_Clk();
  count = 1;
  state = joint1;
  exec = false;
  return;
}

bool coordinatesFiware_c() { // $coordinatesFiware
  return localCoordinatesFiware.comm;
}

bool coordinatesFiware_inp_neighs_c() { // $coordinatesFiware?neighs
  return coordinatesFiware_c();
}


void joint1_f() {
  if (!((coordinatesFiware_c() && cfT) || (since_Clk() > timeout && count <= maxAttempt) || (count > maxAttempt))) {
    debug(2, "joint1 -> joint1, [not(($coordinatesFiware/\\cfT)\\/((since(Clk)>timeout/\\count<=maxAttempt)\\/(count>maxAttempt)))]/caT=true; cfT=true; csT=true; exec");
    caT = true;
    cfT = true;
    csT = true;
    state = joint1;
    exec = true;
    return;
  }
  if (since_Clk() > timeout && count <= maxAttempt) {
    debug(2, "joint1 -> joint1, [since(Clk)>timeout/\\count<=maxAttempt]/#Clk; count=count+1");
    reset_Clk(); // #Clk
    count = count + 1;
    state = joint1;
    exec = false;
    return;
  }
  if (coordinatesFiware_inp_neighs_c() && cfT) { // $coordinatesFiware?neighs && cfT
    debug(2,"joint1 -> SMove, [($coordinatesFiware?neighs and cfT)]/cfT = false");
    cfT = false;
    state = SMove_entry;
    exec = false;
    return;
  }
  if (count > maxAttempt) {
    debug(2, "joint1 -> F, [count > maxAttempt]/$backHome()");
    backHome();
    state = final_snail;
    exec = false;
    return;
  }
}

void move(long vel, long flight_time, DIR dir) {
   ;  // jmi: body of move() to be defined
}

void SMove_entry_f() {
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
  if(op) { // let’s call CheckSnailAndUpdate()
    debug(2, "SMove -> checkSnailAndUpdate(line,column)");
    move(vel,flight_time,dir); // $move(vel,time,dir)
    x_csau = line; // bind the arguments to the parameters
    y_csau = column;
    state = i_csau; // go to the CheckSnailAndUpdate() state machine
    exec = false;      
    return;
  }
  else { // res = NOSNAIL. No call to CheckSnailAndUpdate() must be done.
    res = NOSNAIL;
    state = SMove_exit;
    exec = false; 
    return;
  }
}// end of SMove_entry_f()

void SMove_exit_f() { // State reached after returning from CheckSnailAndUpdate and 
                      // right before going out of SMove 
  if (res == SNAIL_sne) {
    debug(2, "SMove -> F, [res==SNAIL_sne]/$backHome()");
    backHome();
    state = final_snail;
    exec = false;
    return;
  }
  if (res == ERROR_sne) {
    debug(2, "SMove -> F, [res==ERROR_sne]/$backHome()");
    backHome();
    state = final_snail;
    exec = false;
    return;
  }
  if (res == NOSNAIL) {
    debug(2, "SMove -> SConnected, [res==NOSNAIL]/caT=true; cfT=true; csT=true; exec");
    caT = true;
    cfT = true;
    csT = true;
    state = SConnected;
    exec = true;
    return; 
  }
}

void i_csau_f() {
  debug(2, "i -> SInit_csau, /count = 1; #Clk");
  count = 1;
  reset_Clk();
  state = SInit_csau;
  exec = false;
}

void checkSnailSv_out_c(int id) { // checkSnailSv!id
  checkSnailSv.comm = true; // jmi: rever isso
  checkSnailSv.id = id;
}


void SInit_csau_f() {
  debug(2, "SInit_csau -> joint2, $checkSnailSv!id");
  checkSnailSv_out_c(id);
  state = joint2;
  exec = false;
}

bool checkedSnailSvAck_c() {
  return localCheckedSnailSvAck.comm;
}


bool checkedSnailSvAck_inp_hasSnail_c() {
  debug(2, "chamou checkdSnail_inp");
  return checkedSnailSvAck_c();
}

void joint2_f() {
  if (!((checkedSnailSvAck_c() && csT) || (since_Clk() > timeout && count <= maxAttempt) || (count > maxAttempt))) {
    debug(2, "joint2 -> joint2, [not(($checkedSnailSvAck/\\cfT)\\/((since(Clk)>timeout/\\count<=maxAttempt)\\/(count>maxAttempt)))]/caT=true; cfT=true; csT=true; exec");
    csT = true;
    giT = true;
    upT = true;
    state = joint2;
    exec = true;
    return;
  }
  if (since_Clk() > timeout && count <= maxAttempt) {
    debug(2, "joint2 -> joint2, [since(Clk)>timeout/\\count<=maxAttempt]/#Clk; count=count+1");
    reset_Clk();
    count = count + 1;
    state = joint2;
    exec = false;
    return;
  }
  if (checkedSnailSvAck_inp_hasSnail_c() && csT) { // $checkedSnailSvAck?hasSnail
    debug(2, "joint2 ->joint3, [$checkedSnailSvAck?hasSnail/\\csT]/csT=false; count = 1; #Clk");
    csT = false;
    count = 1;
    reset_Clk();
    state = joint3;
    exec = false;
    return;
  }
  if (count > maxAttempt) {
    debug(2, "joint2 -> Final_csau, [count>maxAttempt]/res=SNE::ERROR");
    res = ERROR_sne;
    state = final_csau;
    exec = false;
    return;
  }
}

void updateFiware_out_c(int x, int y, ST state) { // updateFiware!(|x,y,SNAIL_sne|) // jmi: rever isso
  connection.updateFiware(x,y,state);
  updateFiware.comm = true;
  updateFiware.x = x;
  updateFiware.y = y;
  updateFiware.state = state;
}

void joint3_f() {
  if (localCheckedSnailSvAck.hasSnail) { // [hasSnail]
    debug(2, "joint3 -> HasSnail, [hasSnail]/$getSnail();updateFiware!(|x,y,ST::SNAIL|)");
    getSnail();
    updateFiware_out_c(x_csau,y_csau,SNAIL_st); // updateFiware!(|x,y,St::SNAIL|)
    state = HasSnail;
    exec = false;
    return;
  }
  if (!(localCheckedSnailSvAck.hasSnail)) { // [not hasSnail]
    debug(2, "joint3 -> HasSnail, [not hasSnail]/updateFiware!(|x,y,ST::VISITED|)");
    updateFiware_out_c(x_csau,y_csau,VISITED); // updateFiware!(|x,y,St::VISITED|)
    state = NoSnail;
    exec = false;
    return;
  }
}

bool updateAck_c() {
  return updateAck.comm;
}

void HasSnail_f() {
  if (!((updateAck_c() && upT) || (since_Clk() > timeout && count <= maxAttempt) || (count > maxAttempt))) {
    debug(2, "HasSnail -> HasSnail, [not(($coordinatesFiware/\\cfT)\\/((since(Clk)>timeout/\\count<=maxAttempt)\\/(count>maxAttempt)))]/caT=true; cfT=true; csT=true; exec");
    csT = true;
    giT = true;
    upT  = true;
    state = HasSnail;
    exec = true;
    return;
  }
  if (since_Clk() > timeout && count <= maxAttempt) {
    debug(2, "HasSnail -> HasSnail, [since(Clk)>timeout/\\count<=maxAttempt]/#Clk; count=count+1");
    reset_Clk();
    count = count + 1;
    state = HasSnail;
    exec = false;
    return;
  }
  if (count > maxAttempt) {
    debug(2, "HasSnail -> Final_csau, [count>maxAttempt]/res=SNE::ERROR");
    res = ERROR_sne;
    state = final_csau;
    exec = false;
    return;
  }
  if (updateAck_c() && upT) {
    debug(2, "HasSnail -> Final_csau, [updateAck/\\upT]/upT = false; res = SNE:SNAIL");
    upT = false;
    res = SNAIL_sne;
    state = final_csau;
    exec = false;
    return;
  }
}

void NoSnail_f() {
  if (!((updateAck_c() && upT) || (since_Clk() > timeout && count <= maxAttempt) || (count > maxAttempt))) {
    debug(2, "NoSnail -> NoSnail, [not(($coordinatesFiware/\\cfT)\\/((since(Clk)>timeout/\\count<=maxAttempt)\\/(count>maxAttempt)))]/caT=true; cfT=true; csT=true; exec");
    csT = true;
    giT = true;
    upT  = true;
    state = NoSnail;
    exec = true;
    return;
  }
  if (since_Clk() > timeout && count <= maxAttempt) {
    debug(2, "NoSnail -> NoSnail, [since(Clk)>timeout/\\count<=maxAttempt]/#Clk; count=count+1");
    reset_Clk(); // #Clk
    count = count + 1;
    state = NoSnail;
    exec = false;
    return;
  }
  if (count > maxAttempt) {
    debug(2, "NoSnail -> Final_csau, [count>maxAttempt]/res=SNE::ERROR");
    res = ERROR_sne;
    state = final_csau;
    exec = false;
    return;
  }
  if (updateAck_c() && upT) {
    debug(2, "NoSnail -> Final_csau, [updateAck/\\upT]/upt = false; res = SNE:SNAIL");
    upT = false;
    res = NOSNAIL;
    state = final_csau;
    exec = false;
    return;
  }
}

void final_csau_f() {
  debug(2,"Final_csau -> SMove_exit");
  state = SMove_exit;
  exec = false;
}

void final_snail_f() {
  // jmi: é isso?
  while(true){
    debug(2,"FIM DA MAQUINA");
    delay(2000);  
  }
  state = final_snail;
  exec = false;
}

void run_state_machine() {
  switch(state) {
    case i_snail: 
      i_snail_f();
      break;
    case SInit_snail:
      SInit_snail_f();
      break;
    case SConnecting:
      SConnecting_f();
      break;
    case SConnected:
      SConnected_f();
      break;
    case joint1:
      joint1_f();
      break;
    case SMove_entry:
      SMove_entry_f();
      break;
    case i_csau: // state i from CheckSnailAndUpdate()
      i_csau_f();
      break;
    case SInit_csau:
      SInit_csau_f();
      break;
    case joint2:
      joint2_f();
      break;
    case joint3:
      joint3_f();
      break;
    case HasSnail:
      HasSnail_f();
      break;
    case NoSnail:
      NoSnail_f();
      break;
    case final_csau:
      final_csau_f();
      break;
    case SMove_exit:
      SMove_exit_f();
      break;
    case final_snail:
      final_snail_f();
      break;
  } // end of switch(state)
} // end of void run_state_machine()

void run_state_machine_cycle() {
  exec = false;
  while (!exec) {
    run_state_machine();
  }
}

// void loop() {P}  =  void main() {while(true) {P}}
void loop() {
  startCycleTime = millis();
  registerRead();
  run_state_machine_cycle();
  endOperCycleTime = millis();
  //Serial.printf("delay ao final do ciclo: %i", cycleTime - (endOperCycleTime - startCycleTime));
  //Serial.println();
  delay(cycleTime - (endOperCycleTime - startCycleTime));
  //Serial.println("saiu do delay");
}

bool verificaBotao(){
  if(digitalRead(5) == LOW){
    return true;
  }
  return false;
}

void debug(int level, String msg){
  if(DEBUG_LEVEL == level){
    Serial.println(msg); 
  }
}

void dump(){
  Serial.println();
  Serial.println("DUMP");
  Serial.printf("cfT: %i\n",cfT);
  Serial.printf("caT: %i\n", caT);
  Serial.printf("csT: %i\n", csT);
  //Serial.printf("checkedSnailSvAck.comm: %i\n", checkedSnailSvAck.comm);
  //Serial.printf("coordinatesFiware.comm: %i\n", coordinatesFiware.comm);
  Serial.printf("Clk: %i\n", since_Clk());
  Serial.printf("count: %i\n", count);
  Serial.printf("\n\n");
}

