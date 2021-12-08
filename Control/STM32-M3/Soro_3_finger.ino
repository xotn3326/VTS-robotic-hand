#include <DueTimer.h>
#include <Dynamixel2Arduino.h>

#define DXL_SERIAL   Serial
#define DEBUG_SERIAL SerialUSB
const uint8_t DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN
const uint8_t DXL_ID_0 = 0;
const uint8_t DXL_ID_1 = 1;
const uint8_t DXL_ID_2 = 2;

const float DXL_PROTOCOL_VERSION = 1.0;
int velocity[3];
int current_position[3];
int goal_position[3];
int delta_position[3];

float P_val = 0.6;
float I_val = 0.4;
int I_cnt[3];
int button_flag = 0;
int case_num = 0;
int thumb_case = 0;
int index_case = 0;
int middle_case = 0;
int bmp_flag[3];


#define BMP_pulse_1 46
#define BMP_pulse_2 48
#define BMP_pulse_3 50

#define Thumb_1 51
#define Thumb_2 49
#define Thumb_3 47
#define Thumb_4 45
#define Thumb_5 43

#define Index_1 41
#define Index_2 39
#define Index_3 37
#define Index_4 35
#define Index_5 33

#define Middle_1 31
#define Middle_2 29
#define Middle_3 27
#define Middle_4 25
#define Middle_5 23


Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

//This namespace is required to use Control table item names
using namespace ControlTableItem;

void Thumb_pinMode()
{
  pinMode(Thumb_1,INPUT);
  pinMode(Thumb_2,INPUT);
  pinMode(Thumb_3,INPUT);
  pinMode(Thumb_4,INPUT);
  pinMode(Thumb_5,INPUT);
}

void Index_pinMode()
{
  pinMode(Index_1,INPUT);
  pinMode(Index_2,INPUT);
  pinMode(Index_3,INPUT);
  pinMode(Index_4,INPUT);
  pinMode(Index_5,INPUT);
}

void Middle_pinMode()
{
  pinMode(Middle_1,INPUT);
  pinMode(Middle_2,INPUT);
  pinMode(Middle_3,INPUT);
  pinMode(Middle_4,INPUT);
  pinMode(Middle_5,INPUT);
}


void setup() {
  pinMode(BMP_pulse_1,INPUT);
  pinMode(BMP_pulse_2,INPUT);
  pinMode(BMP_pulse_3,INPUT);

  Thumb_pinMode();
  Index_pinMode();
  Middle_pinMode();
  
  // put your setup code here, to run once:
  
  // Use UART port of DYNAMIXEL Shield to debug.`
  DEBUG_SERIAL.begin(115200);
  
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(115200);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(DXL_ID_0);
  dxl.ping(DXL_ID_1);
  dxl.ping(DXL_ID_2);
  

  // Turn off torque when configuring items in EEPROM area

  dxl.torqueOff(DXL_ID_0);
  dxl.torqueOff(DXL_ID_1);
  dxl.torqueOff(DXL_ID_2);
  dxl.setOperatingMode(DXL_ID_0, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_1, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_2, OP_VELOCITY);
  dxl.torqueOn(DXL_ID_0);
  dxl.torqueOn(DXL_ID_1);
  dxl.torqueOn(DXL_ID_2);
  dxl.setGoalVelocity(DXL_ID_0, 500);
  dxl.setGoalVelocity(DXL_ID_1, 500);
  dxl.setGoalVelocity(DXL_ID_2, 500);
  
  dxl.torqueOff(DXL_ID_0);
  dxl.torqueOff(DXL_ID_1);
  dxl.torqueOff(DXL_ID_2);
  dxl.setOperatingMode(DXL_ID_0, OP_POSITION);
  dxl.setOperatingMode(DXL_ID_1, OP_POSITION);
  dxl.setOperatingMode(DXL_ID_2, OP_POSITION);
  dxl.torqueOn(DXL_ID_0);
  dxl.torqueOn(DXL_ID_1);
  dxl.torqueOn(DXL_ID_2);
  dxl.setGoalPosition(DXL_ID_0, 300); //datum
  dxl.setGoalPosition(DXL_ID_1, 300); //datum
  dxl.setGoalPosition(DXL_ID_2, 300); //datum
  delay(1000);
  
  dxl.torqueOff(DXL_ID_0);
  dxl.torqueOff(DXL_ID_1);
  dxl.torqueOff(DXL_ID_2);
  dxl.setOperatingMode(DXL_ID_0, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_1, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_2, OP_VELOCITY);
  dxl.torqueOn(DXL_ID_0);
  dxl.torqueOn(DXL_ID_1);
  dxl.torqueOn(DXL_ID_2);
  Timer.getAvailable().attachInterrupt(velocity_upgrade).start(25000);

  for(int i = 0; i<= 2;i++)
  {
    velocity[i] = 0; 
    current_position[i] = 0;
    goal_position[i] = 0;
    delta_position[i] = 0;
    I_cnt[i] = 0;  
    bmp_flag[i] = 0;
  }
  
}

void loop() {

  Thumb_check();
  Index_check();
  Middle_check();
  Thumb_set();
  Index_set();
  Middle_set();
  current_position_feedback();
  velocity_update(); 
}
void Thumb_check()
{
  if(digitalRead(Thumb_1) == 1)
  {
    thumb_case = 1;
  }
  if(digitalRead(Thumb_2) == 1)
  {
    thumb_case = 2;
  }
  if(digitalRead(Thumb_3) == 1)
  {
    thumb_case = 3;
  }
  if(digitalRead(Thumb_4) == 1)
  {
    thumb_case = 4;
  }if(digitalRead(Thumb_5) == 1)
  {
    thumb_case = 5;
  }
}
void Index_check()
{
  if(digitalRead(Index_1) == 1)
  {
    index_case = 1;
  }
  if(digitalRead(Index_2) == 1)
  {
    index_case = 2;
  }
  if(digitalRead(Index_3) == 1)
  {
    index_case = 3;
  }
  if(digitalRead(Index_4) == 1)
  {
    index_case = 4;
  }if(digitalRead(Index_5) == 1)
  {
    index_case = 5;
  }
}
void Middle_check()
{
  if(digitalRead(Middle_1) == 1)
  {
    middle_case = 1;
  }
  if(digitalRead(Middle_2) == 1)
  {
    middle_case = 2;
  }
  if(digitalRead(Middle_3) == 1)
  {
    middle_case = 3;
  }
  if(digitalRead(Middle_4) == 1)
  {
    middle_case = 4;
  }if(digitalRead(Middle_5) == 1)
  {
    middle_case = 5;
  }
}
void bmp_flag_check()
{
  if(digitalRead(BMP_pulse_1))
  {
    bmp_flag[0] = 1;
  }
  if(digitalRead(BMP_pulse_2))
  {
    bmp_flag[1] = 1;
  }
  if(digitalRead(BMP_pulse_3))
  {
    bmp_flag[2] = 1;
  }
}


void Thumb_set()
{
  switch(thumb_case)
  {
    case 0:
    {
      break;
    }
    case 1: 
    {
      goal_position[0] = 200;
      break;
    }
    case 2:
    {
      goal_position[0] = 450;
      break;
    }
    case 3:
    {
      goal_position[0] = 600;
      break;
    }
    case 4:
    {
      goal_position[0] = 850;
      break;
    }
    case 5:
    {
      goal_position[0] = 1010;
      break;
    }   
  }
}

void Index_set()
{
  switch(index_case)
  {
    case 0:
    {
      break;
    }
    case 1: 
    {
      goal_position[1] = 200;
      break;
    }
    case 2:
    {
      goal_position[1] = 450;
      break;
    }
    case 3:
    {
      goal_position[1] = 600;
      break;
    }
    case 4:
    {
      goal_position[1] = 850;
      break;
    }
    case 5:
    {
      goal_position[1] = 1010;
      break;
    }   
  }
}


void Middle_set()
{
  switch(middle_case)
  {
    case 0:
    {
      break;
    }
    case 1: 
    {
      goal_position[2] = 200;
      break;
    }
    case 2:
    {
      goal_position[2] = 450;
      break;
    }
    case 3:
    {
      goal_position[2] = 600;
      break;
    }
    case 4:
    {
      goal_position[2] = 850;
      break;
    }
    case 5:
    {
      goal_position[2] = 1010;
      break;
    }   
  }
}
void velocity_update()
{
  dxl.setGoalVelocity(DXL_ID_0, velocity[0]);
  dxl.setGoalVelocity(DXL_ID_1, velocity[1]);
  dxl.setGoalVelocity(DXL_ID_2, velocity[2]);
}

void current_position_feedback()
{
    current_position[0] = dxl.getPresentPosition(DXL_ID_0);
    current_position[1] = dxl.getPresentPosition(DXL_ID_1);
    current_position[2] = dxl.getPresentPosition(DXL_ID_2);
    for(int i = 0; i<= 2; i++)
    {
      delta_position[i] = goal_position[i] - current_position[i];
    }
}

void velocity_upgrade()
{
  for(int i = 0; i<= 2; i++)
  {
      if(delta_position[i] >= 0)
    {
        velocity[i] = (P_val * delta_position[i]) + I_val*I_cnt[i];
        if(velocity[i] >= 1017)
        {
          velocity[i] = 1017;
        }
    }
    else
    {
        velocity[i] = abs(P_val * delta_position[i]) + 1023 + I_val*I_cnt[i];
        if(velocity[i] >= 2040)
        {
          velocity[i] = 2040;
        }  
    }
  }

  for(int i = 0; i<=2; i++)
  {
    if(delta_position[i] > 2 && velocity[i] < 1017)
    {
      I_cnt[i]++;
    }
    else if(delta_position[i] < -2 && velocity[i] < 2040)
    {
      I_cnt[i]++;
    }
    else 
    {
      I_cnt[i] = 0;
      velocity[i] = 0;
    }
  }
  for(int i = 0 ;i <=2; i++)
  {
    bmp_flag[i] = 0;
  }

  //bmp_flag_check();
  for(int i = 0; i<=2; i++)
  {
    if(bmp_flag[i] == 1)
    {
      velocity[i] = 0;
      I_cnt[i] = 0;
    }
  } 
}
