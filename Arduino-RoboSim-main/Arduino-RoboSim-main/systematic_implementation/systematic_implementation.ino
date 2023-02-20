// falta incluir regra para imports
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>  

// falta criar regras referentes a instrumentação para debug
#define DEBUG_LOG   1
#define DEBUG_stm0  2
#define DEBUG_stm1  3
#define DEBUG_ctrl0 4
#define DEBUG_stm2  5
#define DEBUG_ctrl1 6
#define DEBUG_mod   7
int DEBUG_LEVEL = 2;

// function 1
TaskHandle_t task_H;
SemaphoreHandle_t semphr;  
SemaphoreHandle_t semphr_collect_out;

// function 7
TaskHandle_t task_ctrl0_H;
SemaphoreHandle_t semphr_ctrl0; 
SemaphoreHandle_t semphr_collect_out_ctrl0;

// incluir regra para definição de tipos
struct Int {
  int value;
  bool flag;
}

struct Event {
  bool comm;
};

struct operation_op1 {
  int y; // parameter
  bool flag;
};

struct operation_op2 {
  bool flag;
};

const int c = 2;

/////////////////////////////////////////////////////
// function 12
askHandle_t task_ctrl0_stm0_H;

// function 13
struct ctrl0_stm0_mem {
  Int w;
  Int x; 
  Int z;
  Event ev0;
  operation_op1 op1;
  operation_op2 op2;
};

// function 14
enum ctrl0_stm0_STATE {
  ctrl0_stm0_i, ctrl0_stm0_s0, ctrl0_stm0_ds0, ctrl0_stm0_joint1  
};

// function 15
void ctrl0_stm0_registerRead(){
  mod.ctrl0.stm0.x = mod.ctrl0.x;
  debug(DEBUG_stm0,"registerRead.imod0_ctrl0_stm0_x." + String(mod.ctrl0.stm0.x) + ",");
  mod.ctrl0.stm0.z = mod.ctrl0.z;
  debug(DEBUG_stm0,"registerRead.imod0_ctrl0_stm0_z." + String(mod.ctrl0.stm0.z) + ",");
}

// function 23
void ctrl0_stm0_i_f() {
  // mod.ctrl0.stm0.w = 1; como distinguir a atribuição de uma variável local de uma variável provida?
  ctrl0_stm0_state = ctrl0_stm0_s0;
  return;
}

void ctrl0_stm0_s0_f() {
  // ctrl0_stm0_write(&mod.ctrl0.stm0.x, mod.ctrl0.stm0.x + mod.ctrl0.stm0.w); assign
  operation_op1 op1_call;
  op1_call.y = 1;
  ctrl0_stm0_write(&mod.ctrl0.stm0.op1, op1_call);
  ctrl0_stm0_state = ctrl0_stm0_ds0;
  return;
}

void ctrl0_stm0_ds0_f() {
  ctrl0_stm0_state = ctrl0_stm0_joint1;
  ctrl0_stm0_exec = true;
  return;
}

void ctrl0_stm0_joint1_f() {
  if (mod.ctrl0.stm0.x >= c) {
    ctrl0_stm0_state = ctrl0_stm0_ds0;
    return;  
  }
  if (mod.ctrl0.stm0.x < c) {
    EVENT ev;
    ev.comm = true;
    ctrl0_stm0_write(&mod.ctrl0.stm0.ev0, ev);
    operation_op2 op;
    op.flag = true;
    ctrl0_stm0_write(&mod.ctrl0.stm0.op2, op);
    // ctrl0_stm0_write(&mod.ctrl0.stm0.z, mod.ctrl0.stm0.x); assign
    ctrl0_stm0_state = ctrl0_stm0_s0;
    return;
  }
}

// function 16
ctrl0_stm0_STATE ctrl0_stm0_state = ctrl0_stm0_i;
bool ctrl0_stm0_exec = false;

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
  ctrl0_stm0_exec = false;
  while (!ctrl0_stm0_exec) {
    ctrl0_stm0_run_state_machine();
  }
}

// function 17
template <typename T1,
void ctrl0_stm0_write(T1 var_stm) {
  if( xSemaphoreTake(semphr_collect_out_ctrl0, portMAX_DELAY) == pdTRUE){
    mod.ctrl0.writting_machine = task_ctrl0_stm0_H;
    xTaskNotify(task_ctrl0_collect_out_H, (uint32_t)var_stm, eSetValueWithOverwrite); //notify ctrl0's collect out
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);//waits ctrl0's collect out
    xSemaphoreGive(semphr_collect_out_ctrl0);
  } 
}

//function 18
void ctrl0_stm0_task(void *arg){
  ctrl0_stm0_initState();
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl0_stm0_registerRead();
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 0, eNoAction);
    }
    ctrl0_stm0_run_state_machine_cycle();
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 0, eNoAction);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
TaskHandle_t task_ctrl0_stm1_H;

struct ctrl0_stm1_mem {
  int x;
  int z;
  EVENT ev5;
  EVENT ev2;
};

enum ctrl0_stm1_STATE {
  ctrl0_stm1_i, ctrl0_stm1_s0, ctrl0_stm1_ds0, ctrl0_stm1_joint1  
};
ctrl0_stm1_STATE ctrl0_stm1_state = ctrl0_stm1_i;
bool ctrl0_stm1_exec = false;

// void ctrl0_stm1_initState() {
//   ctrl0_stm1_state = ctrl0_stm1_i;
// }

void ctrl0_stm1_i_stm1_f() {
  // ctrl0_stm1_write(&mod.ctrl0.stm1.z, 1); assign
  ctrl0_stm1_state = ctrl0_stm1_s0;
  ctrl0_stm1_exec = false;
  return;
}

void ctrl0_stm1_s0_f() {
  ctrl0_stm1_state = ctrl0_stm1_ds0;
  ctrl0_stm1_exec = false;
  return;
}

void ctrl0_stm1_ds0_f() {
  ctrl0_stm1_state = ctrl0_stm1_joint1;
  ctrl0_stm1_exec = true;
  return;
}

bool ctrl0_stm1_ev5(){
  return mod.ctrl0.stm1.ev5.comm; 
}

void ctrl0_stm1_joint1_f() {
  if (!(ctrl0_stm1_ev5() && mod.ctrl0.stm1.x >= c && mod.ctrl0.stm1.z == mod.ctrl0.stm1.x)) {
    ctrl0_stm1_state = ctrl0_stm1_ds0;
    ctrl0_stm1_exec = false;
    return;  
  }
  if (ctrl0_stm1_ev5() && mod.ctrl0.stm1.x >= c && mod.ctrl0.stm1.z == mod.ctrl0.stm1.x) {
    mod.ctrl0.stm1.ev2.comm = true;
    ctrl0_stm1_write(&mod.ctrl0.stm1.ev2);
    // ctrl0_stm1_write(&mod.ctrl0.stm1.x, 0); assign
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

// function 17
template <typename T1>
void ctrl0_stm1_write(T1 var_stm) {
  if( xSemaphoreTake(semphr_collect_out_ctrl0, portMAX_DELAY) == pdTRUE){
    mod.ctrl0.writting_machine = task_ctrl0_stm1_H;
    xTaskNotify(task_ctrl0_collect_out_H, (uint32_t)var_stm, eSetValueWithOverwrite);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr_collect_out_ctrl0);
  } 
}

void ctrl0_stm1_task(void *arg){
  ctrl0_stm1_initState();
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl0_stm1_registerRead();
    // warns the controller that has finished registerRead
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 3, eSetValueWithOverwrite);
    }
    debug(DEBUG_LOG, "M1 - roda maquina ate encontrar um exec e atualiza vars. localmente");
    ctrl0_stm1_run_state_machine_cycle();
    // warns de controller that has finished your cycle
    if( xSemaphoreTake( semphr_ctrl0, portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_ctrl0_H, 3, eSetValueWithOverwrite);
    }
  }
}

//////////////////////////////////////////////////////////////////////////


struct ctrl0_mem {  
  int x;
  int z;
  EVENT ev0;
  EVENT ev2;
  EVENT ev3;
  operation_op1 op1;
  operation_op2 op2;
  TaskHandle_t writting_machine;
  uint32_t output;
  ctrl0_stm0_mem stm0;
  ctrl0_stm1_mem stm1;
};

// TODO : falta regra
void ctrl0_registerRead(){
  mod.ctrl0.ev3.comm = mod.ev3.comm;
  mod.ctrl0.x = mod.x;
  mod.ctrl0.z = mod.z;
}

void ctrl0_reset_output_flags() {
  mod.ctrl0.x_flag = false;
  mod.ctrl0.z_flag = false;
  mod.ctrl0.ev0.comm = false;
  mod.ctrl0.op1.flag = false;
  mod.ctrl0.op2.flag = false;
}

void ctrl0_collect_out_task(void *arg){
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX,&mod.ctrl0.output, portMAX_DELAY);
    
    //stm0 outputs

    if((int *)mod.ctrl0.output == &mod.ctrl0.stm0.x && !mod.ctrl0.x_flag){
      mod.ctrl0.x_flag = true;
      mod.ctrl0.x = mod.ctrl0.stm0.x;
      ctrl0_release_and_notify_module(&mod.ctrl0.x);
    }
    
    if((int *)mod.ctrl0.output == &mod.ctrl0.stm0.z && !mod.ctrl0.z_flag){
      mod.ctrl0.z_flag = true;
      mod.ctrl0.z = mod.ctrl0.stm0.z;
      ctrl0_release_and_notify_module(&mod.ctrl0.z);
    }

    if((EVENT *)mod.ctrl0.output == &mod.ctrl0.stm0.ev0 && !mod.ctrl0.ev0.comm){
      mod.ctrl0.ev0 = mod.ctrl0.stm0.ev0;
      ctrl0_release_and_notify_module(&mod.ctrl0.ev0);
    }
    
    if((operation_op1 *)mod.ctrl0.output == &mod.ctrl0.stm0.op1 && !mod.ctrl0.op1.flag){
      mod.ctrl0.op1 = mod.ctrl0.stm0.op1;
      ctrl0_release_and_notify_module(&mod.ctrl0.op1);
    }

    if((operation_op2 *)mod.ctrl0.output == &mod.ctrl0.stm0.op2 && !mod.ctrl0.op2.flag){
      mod.ctrl0.op2 = mod.ctrl0.stm0.op2;
      ctrl0_release_and_notify_module(&mod.ctrl0.op2);
    }
    
    //stm1 outputs

    if((int *)mod.ctrl0.output == &mod.ctrl0.stm1.x && !mod.ctrl0.x_flag){
      mod.ctrl0.x_flag = true;
      mod.ctrl0.x = mod.ctrl0.stm1.x;
      ctrl0_release_and_notify_module(&mod.ctrl0.x);
    }
    if((int *)mod.ctrl0.output == &mod.ctrl0.stm1.z && !mod.ctrl0.z_flag){
      mod.ctrl0.z_flag = true;
      mod.ctrl0.z = mod.ctrl0.stm1.z;
      ctrl0_release_and_notify_module(&mod.ctrl0.z);
    }
    if((EVENT *)mod.ctrl0.output == &mod.ctrl0.stm1.ev2 && !mod.ctrl0.ev2.comm){
      mod.ctrl0.ev2.comm = true;
      mod.ctrl0.ev2 = mod.ctrl0.stm1.ev2;
      ctrl0_release_and_notify_module(&mod.ctrl0.ev2);
    }
  }
}


void ctrl0_task(void *arg) {
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl0_registerRead();
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xTaskNotify(task_ctrl0_stm0_H, 0, eNoAction);
    xTaskNotify(task_ctrl0_stm1_H, 0, eNoAction);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr_ctrl0);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr_ctrl0);
    ctrl0_reset_output_flags();
    xTaskNotify(task_ctrl0_stm0_H, 0, eNoAction);
    xTaskNotify(task_ctrl0_stm1_H, 0, eNoAction);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr_ctrl0);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr_ctrl0);
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
  }
}

//////////////////////////////////////////////////////////////////////////

TaskHandle_t task_ctrl1_H;
TaskHandle_t task_ctrl1_collect_out_H;

TaskHandle_t task_ctrl1_stm2_H;

struct ctrl1_stm2_mem {
  EVENT ev1, ev2;
  operation_op1 op1;
  int x;
};

enum ctrl1_stm2_STATE {
  ctrl1_stm2_i, ctrl1_stm2_s0, ctrl1_stm2_ds0, ctrl1_stm2_joint1  
};
ctrl1_stm2_STATE ctrl1_stm2_state = ctrl1_stm2_i;
bool ctrl1_stm2_exec = false;

// TODO : falta regra
void ctrl1_stm2_registerRead(){
  mod.ctrl1.stm2.x = mod.ctrl1.x;
  mod.ctrl1.stm2.ev1.comm = mod.ctrl1.ev1.comm;
  mod.ctrl1.stm2.ev2.comm = mod.ctrl1.ev2.comm;
}

void ctrl1_stm2_i_stm2_f() {
  ctrl1_stm2_state = ctrl1_stm2_s0;
  return;
}

void ctrl1_stm2_s0_f() {
  ctrl1_stm2_state = ctrl1_stm2_ds0;
  return;
}

void ctrl1_stm2_ds0_f() {
  ctrl1_stm2_state = ctrl1_stm2_joint1;
  ctrl1_stm2_exec = true;
  return;
}

void ctrl1_stm2_joint1_f() {
  if (!(mod.ctrl1.stm2.x >= c && (mod.ctrl1.stm2.ev1.comm || (mod.ctrl1.stm2.ev1.comm && mod.ctrl1.stm2.x == 1)))) {
    ctrl1_stm2_state = ctrl1_stm2_ds0;
    return;  
  }
  if (mod.ctrl1.stm2.x >= c && (mod.ctrl1.stm2.ev1.comm || (mod.ctrl1.stm2.ev1.comm && mod.ctrl1.stm2.x == 1))) {
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

template <typename T3>
void ctrl1_stm2_write(T2 var_stm) {
    mod.ctrl0.writting_machine = task_ctrl1_stm2_H;
    xTaskNotify(task_ctrl1_collect_out_H, (uint32_t)var_stm, eSetValueWithOverwrite);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
}

void ctrl1_stm2_task(void *arg){
  while(true){
    xTaskNotifyWait (0x00 ,ULONG_MAX, NULL, portMAX_DELAY);
    ctrl1_stm2_registerRead();
    // warns the controller that has finished registerRead
    xTaskNotify(task_ctrl1_H, 0, eNoAction);
    xTaskNotifyWait (0x00 ,ULONG_MAX, NULL, portMAX_DELAY);
    ctrl1_stm2_run_state_machine_cycle();
    debug(DEBUG_LOG, "M2 - roda maquina ate encontrar um exec e atualiza vars. localmente");
    debug(DEBUG_LOG, "M2 - vars. do modulo/controlador atualizadas passadas para C1");
    xTaskNotify(task_ctrl1_H, 0, eNoAction);
  }
}
//////////////////////////////////////////////////////////////////////////

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

void ctrl1_registerRead(){
  mod.ctrl1.ev1.comm = mod.ev1.comm;
  mod.ctrl1.ev2.comm = mod.ev2.comm;
  mod.ctrl1.x = mod.x;
}

void ctrl1_task(void *arg) {
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl1_registerRead();
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xTaskNotify(task_ctrl1_stm2_H, 0, eNoAction);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    ctrl1_reset_output_flags();
    xTaskNotify(task_ctrl1_stm2_H, 0, eNoAction);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    if( xSemaphoreTake( semphr, ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
      xTaskNotify(task_H, 0, eNoAction);
    }
  }
}

void ctrl1_collect_out_task(void *arg){
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
//////////////////////////////////////////////////////////////////////////

TaskHandle_t task_collect_out_H;


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
  semphr_collect_out_ctrl0 = xSemaphoreCreateMutex();
  semphr_collect_out = xSemaphoreCreateMutex();
  
  // The creation of tasks followed a bottom up order
  xTaskCreatePinnedToCore(collect_out_task, "mod_collect_out", 4096, NULL, 4, &task_collect_out_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ctrl0_collect_out_task, "ctrl0_collect_out", 4096, NULL, 4, &task_ctrl0_collect_out_H, tskNO_AFFINITY);
  debug(DEBUG_stm0, "set_mod0_ctrll0_stm0_c.2,");
  xTaskCreatePinnedToCore(ctrl0_stm0_task, "M0", 4096, NULL, 4, &task_ctrl0_stm0_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ctrl0_stm1_task, "M1", 4096, NULL, 4, &task_ctrl0_stm1_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ctrl1_stm2_task, "M2", 4096, NULL, 4, &task_ctrl1_stm2_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ctrl0_task, "C0", 4096, NULL, 4, &task_ctrl0_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(ctrl1_task, "C1", 4096, NULL, 4, &task_ctrl1_H, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(task, "module", 4096, NULL, 4, &task_H, tskNO_AFFINITY);
  randomSeed(analogRead(0));
}

void ctrl0_stm0_registerRead(){
  mod.ctrl0.stm0.x = mod.ctrl0.x;
  debug(DEBUG_stm0,"registerRead.imod0_ctrll0_stm0_x." + String(mod.ctrl0.stm0.x) + ",");
  mod.ctrl0.stm0.z = mod.ctrl0.z;
  debug(DEBUG_stm0,"registerRead.imod0_ctrll0_stm0_z." + String(mod.ctrl0.stm0.z) + ",");
}

void ctrl0_stm1_registerRead(){
  mod.ctrl0.stm1.ev5.comm = mod.ctrl0.ev3.comm;
  debug(DEBUG_stm1,"registerRead.imod0_ctrll0_stm1_ev5." + String(mod.ctrl0.stm1.ev5.comm) + ",");
  mod.ctrl0.stm1.x = mod.ctrl0.x;
  debug(DEBUG_stm1,"registerRead.imod0_ctrll0_stm1_x." + String(mod.ctrl0.stm1.x) + ",");
  mod.ctrl0.stm1.z = mod.ctrl0.z;
  debug(DEBUG_stm1,"registerRead.imod0_ctrll0_stm1_z." + String(mod.ctrl0.stm1.z) + ",");
}


void registerRead(){
  //mod.ev1.comm = !mod.ev1.comm;
  mod.ev3.comm = 1;
  //mod.x = random(0,4);
}




void task(void *arg) {
  while(true){
    debug(DEBUG_LOG, "Modulo le entradas da plataforma");
    debug(DEBUG_LOG, "Cria uma copia dos valores para cada controlador");
    registerRead();
    // releases controllers to run
    xTaskNotify(task_ctrl0_H, 0, eNoAction);
    xTaskNotify(task_ctrl1_H, 0, eNoAction);
    // awaits completion of controller registerRead
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr);
    // reset event values​for communication between controllers
    reset_output_flags();
    // releases controllers to run
    xTaskNotify(task_ctrl0_H, 0, eNoAction);
    xTaskNotify(task_ctrl1_H, 0, eNoAction);
    // expects completion of the controllers' cycle
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr);
    xTaskNotifyWait (pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);
    xSemaphoreGive(semphr);
    debug(DEBUG_LOG, "Modulo escreve na plataforma as variaveis atualizadas");
    debug(DEBUG_LOG, "Passa o tempo");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}


//////////////////////////////////////////////////////////



void collect_out_task(void *args) {
  while(true){
    xTaskNotifyWait (pdFALSE, ULONG_MAX, &mod.output_var, portMAX_DELAY);
    
    //ctrll0
    if((int *)mod.output_var == &mod.ctrl0.x && !mod.x_flag){
      mod.x_flag = true;
      mod.x = mod.ctrl0.x;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
    if((int *)mod.output_var == &mod.ctrl0.z && !mod.z_flag){
      mod.z_flag = true;
      mod.z = mod.ctrl0.z;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
    if((EVENT *)mod.output_var == &mod.ctrl0.ev0 && !mod.ev0.comm){
      mod.op1 = mod.ctrl0.op1;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
    if((operation_op1 *)mod.output_var == &mod.ctrl0.op1 && !mod.op1.flag){
      mod.op1 = mod.ctrl0.op1;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
    if((operation_op2 *)mod.output_var == &mod.ctrl0.op2 && !mod.op2.flag){
      mod.op2 = mod.ctrl0.op2;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
    
    //ctrll1
    if((int *)mod.output_var == &mod.ctrl1.x && !mod.x_flag){
      mod.x_flag = true;
      mod.x = mod.ctrl1.x;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
    if((operation_op1 *)mod.output_var == &mod.ctrl1.op1 && !mod.op1.flag){
      mod.op1 = mod.ctrl1.op1;
      //xTaskNotify(task_mod_ctrl0_collect_out_H, 0, eNoAction);
      xSemaphoreGive(semphr_collect_out);
    }
  }   
}

template <typename T3>
void ctrl0_release_and_notify_module(T3 ctrl0_var){
  xTaskNotify( mod.ctrl0.writting_machine, 0, eNoAction);
  if(xSemaphoreTake(semphr_collect_out, portMAX_DELAY) == pdTRUE){
    xTaskNotify(task_collect_out_H, (uint32_t) ctrl0_var, eSetValueWithOverwrite);
  }
}

template <typename T3>
void ctrl1_release_and_notify_module(T3 ctrl1_var){
  xTaskNotify( mod.ctrl1.writting_machine, 0, eNoAction);
  if(xSemaphoreTake(semphr_collect_out, portMAX_DELAY) == pdTRUE){
    xTaskNotify(task_collect_out_H, (uint32_t) ctrl1_var, eSetValueWithOverwrite);
  }
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
