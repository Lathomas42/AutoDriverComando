#include <AutoDriver.h>
#include<comando.h>
//#define DEBUG 1
Comando com = Comando(Serial);
EchoProtocol echo = EchoProtocol(com);
CommandProtocol cmd =  CommandProtocol(com);

class AutoReel: public AutoDriver {
  private:
    int max_speed;
    int acc;
    int dec;
    int k;
    int ms;
    boolean low_speed;
    int _bp;
    
  public:
    AutoReel(int CSPin, int resetPin, int busyPin)
    : AutoDriver(CSPin, resetPin, busyPin),
      max_speed(500),
      acc(500),
      dec(500),
      k(41),
      ms(7),
      low_speed(false),
      _bp(busyPin)
      {}
    void configure () {
 
      resetDev();
      configSyncPin(_bp, 0);
      configStepMode(ms);
      setMaxSpeed(max_speed);
      //setFullSpeed(600);
      setLoSpdOpt(low_speed);
      setAcc(acc);
      setDec(dec);
      setSlewRate(SR_530V_us);
      setOCThreshold(OC_6000mA);//OC_750mA
      setPWMFreq(PWM_DIV_2, PWM_MUL_2);
      setOCShutdown(OC_SD_DISABLE);
      setVoltageComp(VS_COMP_DISABLE);
      setSwitchMode(SW_USER);
      setOscMode(INT_16MHZ_OSCOUT_16MHZ);
      setAccKVAL(k);
      setDecKVAL(k);
      setRunKVAL(k);
      setHoldKVAL(k);
      #ifdef DEBUG
        Serial.println(getDecKVAL());
        Serial.println(getRunKVAL());
        Serial.println(getHoldKVAL());
        Serial.println(getMaxSpeed());
        Serial.println(getFullSpeed());
        Serial.println(getAcc());
        Serial.println(getMinSpeed());
        Serial.println(getStepMode());
        Serial.println(getLoSpdOpt());
        Serial.println(getOCThreshold());
        Serial.println(getPWMFreqDivisor());
        Serial.println(getPWMFreqMultiplier());
        Serial.println(getVoltageComp());
        Serial.println(getSwitchMode());
      #endif
    }
    
    void set_max_speed(int v) {
      #ifdef DEBUG
        Serial.print("~set max_speed to ");
        Serial.println(v, DEC);
      #endif
      max_speed = v;
      //configure();
    }
    
    int get_max_speed() {
      return max_speed;
    }
    
    void set_acc(int v) {
      #ifdef DEBUG
        Serial.print("~set acc to ");
        Serial.println(v, DEC);
      #endif
      acc = v;
      //configure();
    }
    
    int get_acc() {
      return acc;
    }
    
    void set_dec(int v) {
      #ifdef DEBUG
        Serial.print("~set dec to ");
        Serial.println(v, DEC);
      #endif
      dec = v;
      //configure();
    }
    
    int get_dec() {
      return dec;
    }
    
    void set_k(int v) {
      #ifdef DEBUG
        Serial.print("~set k to ");
        Serial.println(v, DEC);
      #endif
      k = v;
      //configure();
    }
    
    int get_k() {
      return k;
    }
    
    void set_ms(int v) {
      #ifdef DEBUG
        Serial.print("~set ms to ");
        Serial.println(v, DEC);
      #endif
      ms = v;
      //configure();
    };
    
    int get_ms() {
      return ms;
    };
    
    void set_low_speed(boolean v) {
      #ifdef DEBUG
        Serial.print("~set low_speed to ");
        Serial.println(v, DEC);
      #endif
      low_speed = v;
      //configure();
    };
    
    boolean get_low_speed() {
      return low_speed;
    };
};

AutoReel boards[] = {
  AutoReel(10,6,5),
  AutoReel(9,6,5),
  AutoReel(8,6,5),
  AutoReel(7,6,5),
};

byte nboards = 0;
byte board_index = 0;

void conf(CommandProtocol* cmd){ //configure
  int board = cmd->get_arg<int>();
  boards[board].configure();
};//0

void soft_stop(CommandProtocol* cmd){ //stop
  int board = cmd->get_arg<int>();
  boards[board].softStop();
};//1

void hard_stop(CommandProtocol* cmd){ //hard stop
  int board = cmd->get_arg<int>();
  boards[board].hardStop();
};//2

void rel(CommandProtocol* cmd){ //release
  int board = cmd->get_arg<int>();
  boards[board].softHiZ();
};//3

void max_sp(CommandProtocol* cmd){ //set max speed
  int board = cmd->get_arg<int>();
  int sp = cmd->get_arg<int>();
  boards[board].set_max_speed(sp);
};//4

void set_accel(CommandProtocol* cmd){ //set acc/dec
  int board = cmd->get_arg<int>();
  int ac = cmd->get_arg<int>();
  boards[board].set_acc(ac);
  boards[board].set_dec(ac);
};//5

void set_current(CommandProtocol* cmd){ //set k value [0-255]
  int board = cmd->get_arg<int>();
  int new_k = cmd->get_arg<int>();
  boards[board].set_k(new_k);
};//6

void set_micro(CommandProtocol* cmd){ //set microstepping vals [0-7]
  int board = cmd->get_arg<int>();
  int new_ms = cmd->get_arg<int>();
  boards[board].set_ms(new_ms);
};//7

void low_speed(CommandProtocol* cmd){ //set low speed mode true or false
  int board = cmd->get_arg<int>();
  bool is_on = cmd->get_arg<bool>();
  boards[board].set_low_speed(is_on);
  boards[board].configure();
};//8

void is_moving(CommandProtocol* cmd){ //returns a bool if the board is moving
  int board = cmd->get_arg<int>();
  int is_moving = boards[board].busyCheck();
  cmd->start_command(0);
  cmd->add_arg(board);
  cmd->add_arg(is_moving!=0);
  cmd->finish_command();
  //echo.send_message(is_moving);
};//9

void wait(CommandProtocol* cmd){ //holds until the board is done moving
  int board = cmd->get_arg<int>();
  while(boards[board].busyCheck());
};//10

void rot(CommandProtocol* cmd){ //rotates int dir int steps_per_sec
  int board = cmd->get_arg<int>();
  int dir = cmd->get_arg<int>();
  int sps = cmd->get_arg<int>();
  boards[board].run(dir,sps);
};//11

void move_steps(CommandProtocol* cmd){ //move int dir int num_steps
  int board = cmd->get_arg<int>();
  int dir = cmd->get_arg<int>();
  int num_steps = cmd->get_arg<int>();
  boards[board].move(dir,num_steps);
};//12

void query(CommandProtocol* cmd){//returns all vital values
  int board = cmd->get_arg<int>();
  int max_spd = boards[board].get_max_speed();
  int acc = boards[board].get_acc();
  int dec = boards[board].get_dec();
  int k = boards[board].get_k();
  int ms = boards[board].get_ms();
  bool low = boards[board].get_low_speed();
  cmd->start_command(1);
  cmd->add_arg(board);
  cmd->add_arg(max_spd);
  cmd->add_arg(acc);
  cmd->add_arg(dec);
  cmd->add_arg(k);
  cmd->add_arg(ms);
  cmd->add_arg(low);
  cmd->finish_command();
};//13

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  nboards = sizeof(boards) / sizeof(AutoReel);
  for (int i=0; i < nboards; i++) {
    boards[i].configure();
  };
  // register protocols
  com.register_protocol(0,echo);
  com.register_protocol(1,cmd);
  // register command callbacks
  cmd.register_callback(0,conf);
  cmd.register_callback(1,soft_stop);
  cmd.register_callback(2,hard_stop);
  cmd.register_callback(3,rel);
  cmd.register_callback(4,max_sp);
  cmd.register_callback(5,set_accel);
  cmd.register_callback(6,set_current);
  cmd.register_callback(7,set_micro);
  cmd.register_callback(8,low_speed);
  cmd.register_callback(9,is_moving);
  cmd.register_callback(10,wait);
  cmd.register_callback(11,rot);
  cmd.register_callback(12,move_steps);
  cmd.register_callback(13,query);
}

void loop() {
  // put your main code here, to run repeatedly:
  com.handle_stream();
}
