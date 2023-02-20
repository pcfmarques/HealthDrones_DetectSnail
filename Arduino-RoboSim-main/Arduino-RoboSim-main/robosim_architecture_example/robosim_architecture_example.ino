#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define DEBUG_LOG   1
#define DEBUG_stm0  2
#define DEBUG_stm1  3
#define DEBUG_ctrl0 4
#define DEBUG_stm2  5
#define DEBUG_ctrl1 6
#define DEBUG_mod   7
int DEBUG_LEVEL = 2;

SemaphoreHandle_t semphr; // Semaphore to control the sending of simultaneous notifications to the module 
SemaphoreHandle_t semphr_ctrl0; // Semaphore to control sending simultaneous notifications to the C0 controller
SemaphoreHandle_t serialMutex; // Semaphore to control access to serial

TaskHandle_t task_H;
TaskHandle_t task_ctrl0_H;
TaskHandle_t task_ctrl1_H;
TaskHandle_t task_ctrl0_stm0_H;
TaskHandle_t task_ctrl0_stm1_H;
TaskHandle_t task_ctrl1_stm2_H;


enum ctrl0_stm0_STATE {
  ctrl0_stm0_i, ctrl0_stm0_s0, ctrl0_stm0_ds0, ctrl0_stm0_joint1  
};

enum ctrl0_stm1_STATE {
  ctrl0_stm1_i, ctrl0_stm1_s0, ctrl0_stm1_ds0, ctrl0_stm1_joint1  
};

enum ctrl1_stm2_STATE {
  ctrl1_stm2_i, ctrl1_stm2_s0, ctrl1_stm2_ds0, ctrl1_stm2_joint1  
};


ctrl0_stm0_STATE ctrl0_stm0_state;
ctrl0_stm1_STATE ctrl0_stm1_state;
ctrl1_stm2_STATE ctrl1_stm2_state;
bool ctrl0_stm0_exec = false;
bool ctrl0_stm1_exec = false;
bool ctrl1_stm2_exec = false;


// constant
const int c = 2;

// EVENT. Represents an event.
struct EVENT {
  bool comm;
};


struct operation_op1 {
  bool flag;
  int y; // parameter
};

struct operation_op2 {
  bool flag;
}; 

struct ctrl0_stm0_mem {
  // locally declared variable
  int w;
  // input and output variables
  int x; 
  int z;
  // output event
  EVENT ev0;
  operation_op1 op1;
  operation_op2 op2;
};

struct ctrl0_stm1_mem {
  // input and output variables
  int x; // does not have a flag because x is declared in the controller
  int z;
  // input event
  EVENT ev5;
  // output event
  EVENT ev2;
};

struct ctrl0_mem {
  int x;
  bool x_flag;// does not have comm field so a flag is required to record whether it has been written in the cycle
  int z;
  bool z_flag;
  EVENT ev0;//evento já tem flag na estrutura, lembrar de resetar flag
  EVENT ev2;
  EVENT ev3;
  //operations
  operation_op1 op1;
  operation_op2 op2;
  TaskHandle_t writting_machine; //current writting machine (e.g mod0_ctrl0_stm1)
  uint32_t output;
  // state machines
  ctrl0_stm0_mem stm0;
  ctrl0_stm1_mem stm1;
};
bool ctrl0_endExec;

struct ctrl1_stm2_mem {
  // input events
  EVENT ev1, ev2;
  // output operation
  operation_op1 op1;
  // input and output variable
  int x;
};

struct ctrl1_mem {
  EVENT ev1, ev2;
  int x;
  bool x_flag;
  operation_op1 op1;
  operation_op2 op2;
  TaskHandle_t writting_machine;
  uint32_t output;
  ctrl1_stm2_mem stm2;
};

struct mod_mem {
  EVENT ev0;
  EVENT ev1;
  EVENT ev2;
  EVENT ev3;
  int x;
  bool x_flag;
  int z;
  bool z_flag;
  ctrl0_mem ctrl0;
  ctrl1_mem ctrl1;
  uint32_t output_var;
  TaskHandle_t writting_controller;
  uint32_t output;
  operation_op1 op1;
  operation_op2 op2;
};

mod_mem mod;

void setup() {
  Serial.begin(115200);
  
  semphr = xSemaphoreCreateMutex();
  semphr_ctrl0 = xSemaphoreCreateMutex();
  serialMutex = xSemaphoreCreateMutex();
  
  // The creation of tasks followed a bottom up order
  debug(DEBUG_stm0, "set_mod0_ctrl0_stm0_c.2,");
  debug(DEBUG_stm1, "set_mod0_ctrl0_stm1_c.2,");
  xTaskCreatePinnedToCore(ctrl0_stm0_task, "M0", 4096, NULL, 4, &task_ctrl0_stm0_H, tskNO_AFFINITY);
  //xTaskCreatePinnedToCore(ctrl0_stm1_task, "M1", 4096, NULL, 4, &task_ctrl0_stm1_H, tskNO_AFFINITY);
  //xTaskCreatePinnedToCore(ctrl1_stm2_task, "M2", 4096, NULL, 4, &task_ctrl1_stm2_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ctrl0_task, "C0", 4096, NULL, 4, &task_ctrl0_H, tskNO_AFFINITY);
  // xTaskCreatePinnedToCore(ctrl1_task, "C1", 4096, NULL, 4, &task_ctrl1_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(task, "module", 4096, NULL, 4, &task_H, tskNO_AFFINITY);
  // randomSeed(analogRead(0));
}

void ctrl0_stm0_registerRead(){
  mod.ctrl0.stm0.x = mod.ctrl0.x;
  debug(DEBUG_stm0,"registerRead.imod0_ctrl0_stm0_x." + String(mod.ctrl0.stm0.x) + ",");
  mod.ctrl0.stm0.z = mod.ctrl0.z;
  debug(DEBUG_stm0,"registerRead.imod0_ctrl0_stm0_z." + String(mod.ctrl0.stm0.z) + ",");
}

void ctrl0_stm1_registerRead(){
  mod.ctrl0.stm1.ev5.comm = mod.ctrl0.ev3.comm;
  debug(DEBUG_stm1,"registerRead.imod0_ctrl0_stm1_ev5." + String(mod.ctrl0.stm1.ev5.comm?"true" : "false") + ",");
  mod.ctrl0.stm1.x = 2; //mod.ctrl0.x;
  debug(DEBUG_stm1,"registerRead.imod0_ctrl0_stm1_x." + String(mod.ctrl0.stm1.x) + ",");
  mod.ctrl0.stm1.z = 2; //mod.ctrl0.z;
  debug(DEBUG_stm1,"registerRead.imod0_ctrl0_stm1_z." + String(mod.ctrl0.stm1.z) + ",");
}

void ctrl0_registerRead(){
  mod.ctrl0.ev3.comm = mod.ev3.comm;
  debug(DEBUG_ctrl0, "registerRead.imod0_ctrl0_ev3." + String(mod.ctrl0.ev3.comm));
  mod.ctrl0.x = mod.x;
  debug(DEBUG_ctrl0, "registerRead.imod0_ctrl0_x." + String(mod.ctrl0.x));
  mod.ctrl0.z = mod.z;
}

void ctrl1_stm2_registerRead(){
  mod.ctrl1.stm2.x = mod.ctrl1.x;
  mod.ctrl1.stm2.ev1.comm = mod.ctrl1.ev1.comm;
  mod.ctrl1.stm2.ev2.comm = mod.ctrl1.ev2.comm;
}

void ctrl1_registerRead(){
  mod.ctrl1.ev1.comm = mod.ev1.comm;
  mod.ctrl1.ev2.comm = mod.ev2.comm;
  mod.ctrl1.x = mod.x;
}

void registerRead(){
  mod.ev1.comm = !mod.ev1.comm;
  mod.ev3.comm = 1;
  //mod.x = random(0,4);
}

void ctrl0_stm0_task(void *arg){
  ctrl0_stm0_initState();
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl0_stm0_registerRead();
    // warns the controller that has finished registerRead
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 0, eNoAction);
    }
    debug(DEBUG_LOG, "M0 - roda maquina ate encontrar um exec");
    ctrl0_stm0_run_state_machine_cycle();
    // warns de controller that has finished your cycle
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 0, eNoAction);
    }
  }
}

void ctrl0_stm1_task(void *arg){
  ctrl0_stm1_initState();
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl0_stm1_registerRead();
    // warns the controller that has finished registerRead
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 0, eNoAction);
    }
    debug(DEBUG_LOG, "M1 - roda maquina ate encontrar um exec e atualiza vars. localmente");
    ctrl0_stm1_run_state_machine_cycle();
    // warns de controller that has finished your cycle
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      debug(DEBUG_stm1, "238 finalizando ciclo stm1");
      xTaskNotify(task_ctrl0_H, 0, eNoAction);
    }
  }
}

void ctrl0_task(void *arg) {
  while(true){
    xTaskNotifyWait (0x00, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl0_endExec = false;
    debug(DEBUG_LOG, "inicio de ciclo: ");
    debug(DEBUG_LOG, "C0 - cria uma copia dos valores para cada maquina");
    ctrl0_registerRead();
    debug(DEBUG_LOG, "saiu register read");
    // notifies the module that registerRead has already been completed 
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
    debug(DEBUG_LOG, "fez notify");
    // releases state machines to run
    xTaskNotify(task_ctrl0_stm0_H, 0, eNoAction);
    //xTaskNotify(task_ctrl0_stm1_H, 0, eNoAction);
    // awaits state machines to complete registerRead
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr_ctrl0);
    //xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    //xSemaphoreGive(semphr_ctrl0);
    // reset event values​ communicated between machines
    ctrl0_reset_output_flags();
    // collects the outputs communicated by the machines
    ctrl0_collect_out_task();
    // awaits state machines complete your cycles
    // notify the module 
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      ctrl0_endExec = true;
      debug(DEBUG_LOG, "269 fim ciclo do controlador");
      xTaskNotify(task_H, 12, eSetValueWithOverwrite);
    }  
  }
}


void ctrl1_stm2_task(void *arg){
  while(true){
    xTaskNotifyWait (0x00 ,ULONG_MAX, NULL, portMAX_DELAY);
    ctrl1_stm2_registerRead();
    // warns the controller that has finished registerRead
    xTaskNotify(task_ctrl1_H, 0, eNoAction);
    ctrl1_stm2_run_state_machine_cycle();
    debug(DEBUG_LOG, "M2 - roda maquina ate encontrar um exec e atualiza vars. localmente");
    debug(DEBUG_LOG, "M2 - vars. do modulo/controlador atualizadas passadas para C1");
    xTaskNotify(task_ctrl1_H, 0, eNoAction);
  }
}

void ctrl1_task(void *arg) {
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl1_registerRead();
    // notifies the module that registerRead has already been completed
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
    debug(DEBUG_LOG, "C1 - cria uma copia dos valores para cada maquina");
    // releases state machine to run
    xTaskNotify(task_ctrl1_stm2_H, 0, eNoAction);
    // waits for state machine to finalize to registerRead
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    // reset event values​communicated between machines
    ctrl1_reset_output_flags();
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    // notifies the module that the cycle has already benn completed
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
  }
}

void task(void *arg) {
  uint32_t output;
  while(true){
    debug(DEBUG_LOG, "Modulo le entradas da plataforma");
    debug(DEBUG_LOG, "Cria uma copia dos valores para cada controlador");
    registerRead();
    // releases controllers to run
    xTaskNotify(task_ctrl0_H, 0, eNoAction);
    //xTaskNotify(task_ctrl1_H, 0, eNoAction);
    // awaits completion of controller registerRead
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr);
    //xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    //xSemaphoreGive(semphr);
    // reset event values​for communication between controllers
    reset_output_flags();
    // expects completion of the controllers' cycle
    collect_out_task();
  
//    xTaskNotifyWait (pdFALSE, ULONG_MAX, &output, portMAX_DELAY);
//    xSemaphoreGive(semphr);
    debug(DEBUG_LOG, "Modulo escreve na plataforma as variaveis atualizadas");
    debug(DEBUG_LOG, "Passa o tempo");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}


//////////////////////////////////////////////////////////
// state machine stm0 implementation

void ctrl0_stm0_initState() {
   ctrl0_stm0_state = ctrl0_stm0_i;
}

void ctrl0_stm0_i_stm0_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm0: i -> s0 w=1");
  mod.ctrl0.stm0.w = 1;
  ctrl0_stm0_state = ctrl0_stm0_s0;
  return;
}

void ctrl0_stm0_s0_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm0: s0 -> ds0 entry $x = x + w; $op1(1)");
  mod.ctrl0.stm0.x = mod.ctrl0.stm0.x + mod.ctrl0.stm0.w; 
  ctrl0_stm0_write(&mod.ctrl0.stm0.x); // entry $x = x + w
  debug(DEBUG_stm0,"registerWrite.omod0_ctrl0_stm0_x." + String(mod.ctrl0.stm0.x) + ",");
  mod.ctrl0.stm0.op1.y = 1;
  mod.ctrl0.stm0.op1.flag = true;
  ctrl0_stm0_write(&mod.ctrl0.stm0.op1);
  debug(DEBUG_stm0,"registerWrite.omod0_ctrl0_stm0_op1." + String(mod.ctrl0.stm0.op1.y) + ",");
  ctrl0_stm0_state = ctrl0_stm0_ds0;
  return;
}

void ctrl0_stm0_ds0_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm0: ds0 -> joint1 exec");
  ctrl0_stm0_state = ctrl0_stm0_joint1;
  ctrl0_stm0_exec = true;
  debug(DEBUG_stm0,"mod0_ctrl0_stm0_endexec.\"mod0_ctrl0_stm0_ds0_to_j0\",");
  return;
}

void ctrl0_stm0_joint1_f() {
  if (mod.ctrl0.stm0.x >= c) {
    debug(DEBUG_LOG, "mod_ctrl0_stm0: joint1 -> ds0 [x >= c]");
    ctrl0_stm0_state = ctrl0_stm0_ds0;
    return;  
  }
  if (mod.ctrl0.stm0.x < c) {
    debug(DEBUG_LOG, "mod_ctrl0_stm0: joint1 -> s0 [x < 0]/$ev0;op2();$z=x");
    mod.ctrl0.stm0.ev0.comm = true;
    ctrl0_stm0_write(&mod.ctrl0.stm0.ev0);
    debug(DEBUG_stm0,"registerWrite.omod0_ctrl0_stm0_ev0,");
    mod.ctrl0.stm0.op2.flag = true;
    ctrl0_stm0_write(&mod.ctrl0.stm0.op2);
    debug(DEBUG_stm0,"registerWrite.omod0_ctrl0_stm0_op2,");
    mod.ctrl0.stm0.z = mod.ctrl0.stm0.x;
    ctrl0_stm0_write(&mod.ctrl0.stm0.z);
    debug(DEBUG_stm0,"registerWrite.omod0_ctrl0_stm0_z." + String(mod.ctrl0.stm0.z) + ",");
    ctrl0_stm0_state = ctrl0_stm0_s0;
    return;
  }
}

void ctrl0_stm0_run_state_machine() {
  switch(ctrl0_stm0_state){
    case ctrl0_stm0_i:
      ctrl0_stm0_i_stm0_f();
      break;
    case ctrl0_stm0_s0:
      ctrl0_stm0_s0_f();
      break;
    case ctrl0_stm0_ds0:
      ctrl0_stm0_ds0_f();
      break;
    case ctrl0_stm0_joint1:
      ctrl0_stm0_joint1_f();
      break;
  }
}

void ctrl0_stm0_run_state_machine_cycle(){
  debug(DEBUG_LOG, "iniciou stm0");
  ctrl0_stm0_exec = false;
  while (!ctrl0_stm0_exec) {
    ctrl0_stm0_run_state_machine();
  }
}
//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
// state machine stm1 implementation

void ctrl0_stm1_initState() {
  ctrl0_stm1_state = ctrl0_stm1_i;
}

void ctrl0_stm1_i_stm1_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm1: i -> s0 z=1");
  //T1 var_stm, T2 val
  ctrl0_stm1_write(&mod.ctrl0.stm1.z);
  debug(DEBUG_stm1,"registerWrite.omod0_ctrl0_stm1_z." + String(mod.ctrl0.stm1.z) + ",");
  ctrl0_stm1_state = ctrl0_stm1_s0;
  ctrl0_stm1_exec = false;
  return;
}

void ctrl0_stm1_s0_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm1: s0 -> ds0");
  ctrl0_stm1_state = ctrl0_stm1_ds0;
  ctrl0_stm1_exec = false;
  return;
}

void ctrl0_stm1_ds0_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm1: ds0 -> joint1 exec");
  ctrl0_stm1_state = ctrl0_stm1_joint1;
  ctrl0_stm1_exec = true;
  debug(DEBUG_stm1,"mod0_ctrl0_stm1_endexec.\"mod0_ctrl0_stm1_ds0_to_j0\",");
  return;
}

bool ctrl0_stm1_ev5(){
  return mod.ctrl0.stm1.ev5.comm; 
}

void ctrl0_stm1_joint1_f() {
  if (!(ctrl0_stm1_ev5() && mod.ctrl0.stm1.x >= c && mod.ctrl0.stm1.z == mod.ctrl0.stm1.x)) {
    debug(DEBUG_LOG, "mod_ctrl0_stm1: joint1 -> ds0 [not($ev5/\\x>=c/\\z==x)]");
    ctrl0_stm1_state = ctrl0_stm1_ds0;
    ctrl0_stm1_exec = false;
    return;  
  }
  if (ctrl0_stm1_ev5() && mod.ctrl0.stm1.x >= c && mod.ctrl0.stm1.z == mod.ctrl0.stm1.x) {
    debug(DEBUG_stm1,"471");
    debug(DEBUG_LOG, "mod_ctrl0_stm1: joint1 -> s0 [($ev5/\\x>=c/\\z==x)]/$ev2;$x=0");
    mod.ctrl0.stm1.ev2.comm = true;
    ctrl0_stm1_write(&mod.ctrl0.stm1.ev2);
    debug(DEBUG_stm1,"registerWrite.omod0_ctrl0_stm1_ev2,");
    mod.ctrl0.stm1.x = 0;
    ctrl0_stm1_write(&mod.ctrl0.stm1.x);
    debug(DEBUG_stm1,"registerWrite.omod0_ctrl0_stm1_x." + String(mod.ctrl0.stm1.x) + ",");
    ctrl0_stm1_state = ctrl0_stm1_s0;
    ctrl0_stm1_exec = false;
    return;
  }
}

void ctrl0_stm1_run_state_machine() {
  switch(ctrl0_stm1_state){
    case ctrl0_stm1_i:
      ctrl0_stm1_i_stm1_f();
      break;
    case ctrl0_stm1_s0:
      ctrl0_stm1_s0_f();
      break;
    case ctrl0_stm1_ds0:
      ctrl0_stm1_ds0_f();
      break;
    case ctrl0_stm1_joint1:
      ctrl0_stm1_joint1_f();
      break;
  }
}

void ctrl0_stm1_run_state_machine_cycle(){
  ctrl0_stm1_exec = false;
  while (!ctrl0_stm1_exec) {
    ctrl0_stm1_run_state_machine();
  }
}
//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
// state machine stm2 implementation

void ctrl1_stm2_initState() {
   ctrl1_stm2_state = ctrl1_stm2_i;
}

void ctrl1_stm2_i_stm2_f() {
  //debug(DEBUG_LOG, "mod_ctrl1_stm2: i -> s0");
  ctrl1_stm2_state = ctrl1_stm2_s0;
  return;
}

void ctrl1_stm2_s0_f() {
  //debug(DEBUG_LOG, "mod_ctrl0_stm0: s0 -> ds0");
  ctrl1_stm2_state = ctrl1_stm2_ds0;
  return;
}

void ctrl1_stm2_ds0_f() {
  debug(DEBUG_LOG, "mod_ctrl0_stm0: ds0 -> joint1 exec");
  ctrl1_stm2_state = ctrl1_stm2_joint1;
  ctrl1_stm2_exec = true;
  return;
}

void ctrl1_stm2_joint1_f() {
  if (!(mod.ctrl1.stm2.x >= c && (mod.ctrl1.stm2.ev1.comm || (mod.ctrl1.stm2.ev1.comm && mod.ctrl1.stm2.x == 1)))) {
    //debug(DEBUG_LOG, "mod_ctrl1_stm2: joint1 -> ds0");
    ctrl1_stm2_state = ctrl1_stm2_ds0;
    return;  
  }
  if (mod.ctrl1.stm2.x >= c && (mod.ctrl1.stm2.ev1.comm || (mod.ctrl1.stm2.ev1.comm && mod.ctrl1.stm2.x == 1))) {
    //debug(DEBUG_LOG, "mod_ctrl1_stm2: joint1 -> s0");
    //mod_ctrl1_stm2_write_op1(1);
    ctrl1_stm2_state = ctrl1_stm2_s0;
    return;
  }
}

void ctrl1_stm2_run_state_machine() {
  switch(ctrl1_stm2_state){
    case ctrl1_stm2_i:
      ctrl1_stm2_i_stm2_f();
      break;
    case ctrl1_stm2_s0:
      ctrl1_stm2_s0_f();
      break;
    case ctrl1_stm2_ds0:
      ctrl1_stm2_ds0_f();
      break;
    case ctrl1_stm2_joint1:
      ctrl1_stm2_joint1_f();
      break;
  }
}

void ctrl1_stm2_run_state_machine_cycle(){
  ctrl1_stm2_exec = false;
  while (!ctrl1_stm2_exec) {
    ctrl1_stm2_run_state_machine();
  }
}
//////////////////////////////////////////////////////////


template <typename T1>
void ctrl0_stm0_write(T1 var_stm) {
  if( xSemaphoreTake(semphr_ctrl0, portMAX_DELAY) == pdTRUE){
    mod.ctrl0.writting_machine = task_ctrl0_stm0_H;
    xTaskNotify(task_ctrl0_H, (uint32_t)var_stm, eSetValueWithOverwrite); //notify ctrl0's collect out
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);//waits ctrl0's collect out
    xSemaphoreGive(semphr_ctrl0);
  } 
}

//Esta função é chamada apenas quando var_stm é do controlador ou do módulo
template <typename T3>
void ctrl0_stm1_write(T3 var_stm) {
  if( xSemaphoreTake(semphr_ctrl0, portMAX_DELAY) == pdTRUE){
    mod.ctrl0.writting_machine = task_ctrl0_stm1_H;
    xTaskNotify(task_ctrl0_H, (uint32_t)var_stm, eSetValueWithOverwrite); //notify ctrl0's collect out
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);//waits ctrl0's collect out
    xSemaphoreGive(semphr_ctrl0);
  } 
}

void ctrl0_collect_out_task(){
  debug(DEBUG_LOG, "600 iniciou collect out");
  while(true){  
    xTaskNotifyWait (pdFALSE, ULONG_MAX, &mod.ctrl0.output, portMAX_DELAY); //comes from write from some state machine
    debug(DEBUG_stm1, "602 collect out");
    // se a variável não for do contexto do módulo, as máquinas são liberadas sem antes ter notificado o módulo
     debug(DEBUG_LOG, "entrou no collect out");
    /*
    if(mod.ctrl0.output_var == &mod.ctrl0.stm1.z) {
      mod.ctrl0.stm1.z_flag = true;
      mod.ctrl0.z = mod.ctrl0.stm1.z;
      xSemaphoreGive(semphr_collect_out_mod_ctrl0);
    }
    */
    
    //stm0 outputs

    if((int *)mod.ctrl0.output == &mod.ctrl0.stm0.x && !mod.ctrl0.x_flag){
      mod.ctrl0.x_flag = true;
      mod.ctrl0.x = mod.ctrl0.stm0.x;
      debug(DEBUG_ctrl0, "registerWrite.omod0_ctrl0_x." + String(mod.ctrl0.x));
      ctrl0_release_and_notify_module(&mod.ctrl0.x);
    }
    
    if((int *)mod.ctrl0.output == &mod.ctrl0.stm0.z && !mod.ctrl0.z_flag){
      mod.ctrl0.z_flag = true;
      mod.ctrl0.z = mod.ctrl0.stm0.z;
      ctrl0_release_and_notify_module(&mod.ctrl0.z);
    }

    if((EVENT *)mod.ctrl0.output == &mod.ctrl0.stm0.ev0 && !mod.ctrl0.ev0.comm){
      mod.ctrl0.ev0 = mod.ctrl0.stm0.ev0; // the flag and arguments of the event are set on this line
      ctrl0_release_and_notify_module(&mod.ctrl0.ev0);
    }
    
    if((operation_op1 *)mod.ctrl0.output == &mod.ctrl0.stm0.op1 && !mod.ctrl0.op1.flag){
      mod.ctrl0.op1 = mod.ctrl0.stm0.op1; // the flag and arguments of the operation are set on this line
      debug(DEBUG_ctrl0, "registerWrite.omod0_ctrl0_op1." + String(mod.ctrl0.op1.y));
      ctrl0_release_and_notify_module(&mod.ctrl0.op1);
    }

    if((operation_op2 *)mod.ctrl0.output == &mod.ctrl0.stm0.op2 && !mod.ctrl0.op2.flag){
      mod.ctrl0.op2 = mod.ctrl0.stm0.op2; // the flag and arguments of the operation are set on this line
      ctrl0_release_and_notify_module(&mod.ctrl0.op2);
    }
    
    //stm1 outputs
    
    if((int *)mod.ctrl0.output == &mod.ctrl0.stm1.x && !mod.ctrl0.x_flag){
      mod.ctrl0.x_flag = true;
      mod.ctrl0.x = mod.ctrl0.stm1.x;
      debug(DEBUG_ctrl0, "registerWrite.omod0_ctrl0_x." + String(mod.ctrl0.x));
      ctrl0_release_and_notify_module(&mod.ctrl0.x);
     
      //xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY); // waits mod's collect out
    }
    if((int *)mod.ctrl0.output == &mod.ctrl0.stm1.z && !mod.ctrl0.z_flag){
      mod.ctrl0.z_flag = true;
      mod.ctrl0.z = mod.ctrl0.stm1.z;
      ctrl0_release_and_notify_module(&mod.ctrl0.z);
     
      //xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY); //waits mod's collect out
    }
    if((EVENT *)mod.ctrl0.output == &mod.ctrl0.stm1.ev2 && !mod.ctrl0.ev2.comm){
      mod.ctrl0.ev2 = mod.ctrl0.stm1.ev2;
      ctrl0_release_and_notify_module(&mod.ctrl0.ev2);
      
      //xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY); //waits mod's collect out
    }

    if((bool *)mod.ctrl0.output == &ctrl0_stm0_exec){
      xSemaphoreGive(semphr_ctrl0);
    }
    if((bool *)mod.ctrl0.output == &ctrl0_stm1_exec){
      xSemaphoreGive(semphr_ctrl0);
    }
    if(ctrl0_stm0_exec){ //&& ctrl0_stm1_exec
      debug(DEBUG_stm1, "674 finalizou exec");
      xSemaphoreGive(semphr_ctrl0);
      break;
    }
  }
}

void ctrl1_collect_out_task(){
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, &mod.ctrl1.output, portMAX_DELAY);
    
    if((operation_op1 *)mod.ctrl1.output == &mod.ctrl1.stm2.op1 && !mod.ctrl1.op2.flag){
      mod.ctrl1.op1 = mod.ctrl1.stm2.op1; // the flag and arguments of the operation are set on this line
      ctrl1_release_and_notify_module(&mod.ctrl1.op1);
    }
    
     if((int *)mod.ctrl1.output == &mod.ctrl1.stm2.x && !mod.ctrl1.x_flag){
      mod.ctrl1.x_flag = true;
      mod.ctrl1.x = mod.ctrl1.stm2.x;
      ctrl1_release_and_notify_module(&mod.ctrl1.x);
    }
  }
}

void collect_out_task() {
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, &mod.output_var, portMAX_DELAY);
    debug(DEBUG_LOG, "699 entrou no collectout do modulo");
    
    //ctrl0
    if((int *)mod.output_var == &mod.ctrl0.x && !mod.x_flag){
      mod.x_flag = true;
      mod.x = mod.ctrl0.x;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    if((int *)mod.output_var == &mod.ctrl0.z && !mod.z_flag){
      mod.z_flag = true;
      mod.z = mod.ctrl0.z;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    if((EVENT *)mod.output_var == &mod.ctrl0.ev0 && !mod.ev0.comm){
      mod.ev0 = mod.ctrl0.ev0;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    if((EVENT *)mod.output_var == &mod.ctrl0.ev2 && !mod.ev2.comm){
      mod.ev2 = mod.ctrl0.ev2;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    if((operation_op1 *)mod.output_var == &mod.ctrl0.op1 && !mod.op1.flag){
      mod.op1 = mod.ctrl0.op1;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    if((operation_op2 *)mod.output_var == &mod.ctrl0.op2 && !mod.op2.flag){
      mod.op2 = mod.ctrl0.op2;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    
    //ctrll1
    if((int *)mod.output_var == &mod.ctrl1.x && !mod.x_flag){
      mod.x_flag = true;
      mod.x = mod.ctrl1.x;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
    if((operation_op1 *)mod.output_var == &mod.ctrl1.op1 && !mod.op1.flag){
      mod.op1 = mod.ctrl1.op1;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr);
    }
//    if((ctrl0_endExec *)mod.output_var == &mod.ctrl1.op1){
//      xSemaphoreGive(semphr);
//    }
    if(ctrl0_endExec){
      debug(DEBUG_LOG, "748");
      xSemaphoreGive(semphr);
      break;
    }
    
  }   
}

template <typename T3>
void ctrl0_release_and_notify_module(T3 ctrl0_var){
  xTaskNotify( mod.ctrl0.writting_machine, 0, eNoAction);
  if(xSemaphoreTake(semphr, portMAX_DELAY) == pdTRUE){
    xTaskNotify(task_H, (uint32_t) ctrl0_var, eSetValueWithOverwrite);
  }
}

template <typename T3>
void ctrl1_release_and_notify_module(T3 ctrl1_var){
  xTaskNotify( mod.ctrl1.writting_machine, 0, eNoAction);
  if(xSemaphoreTake(semphr, portMAX_DELAY) == pdTRUE){
    xTaskNotify(task_H, (uint32_t) ctrl1_var, eSetValueWithOverwrite);
  }
}

void ctrl0_reset_output_flags() {
  mod.ctrl0.x_flag = false;
  mod.ctrl0.z_flag = false;
  mod.ctrl0.ev0.comm = false;
  mod.ctrl0.ev2.comm = false;
  mod.ctrl0.op1.flag = false;
  mod.ctrl0.op2.flag = false;
}

void ctrl1_reset_output_flags() {
  mod.ctrl1.x_flag = false;
  mod.ctrl1.op1.flag = false;
}

void reset_output_flags() {
  mod.x_flag = false;
  mod.z_flag = false;
  mod.ev0.comm = false;
  mod.op1.flag = false;
  mod.op2.flag = false;
}


void debug(int lv, String msg) {
  if(lv == DEBUG_LEVEL){
    if( xSemaphoreTake( serialMutex, portMAX_DELAY ) == pdTRUE ){
      Serial.println(msg);
      xSemaphoreGive(serialMutex);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
