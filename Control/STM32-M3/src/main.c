/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
//bmp  --> MSB == 1
//bmp write--> MSB == 0


#include "stm32f10x_conf.h"

#define chip_ID 0x00
#define IF_CONFIG 0x1A
#define PWR_CTRL 0x1B //B00110011 --> 0x33
#define ODR 0x1D
#define OSR 0x1C
#define	PRESSURE 0x04
#define	TEMPERATURE 0x07

#define GPIO_Remap_SWJ_NoJTRST      ((uint32_t)0x00300100)  /*!< Full SWJ Enabled (JTAG-DP + SW-DP) but without JTRST */
#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled */
#define GPIO_Remap_SWJ_Disable      ((uint32_t)0x00300400)  /*!< Full SWJ Disabled (JTAG-DP + SW-DP) */

unsigned long result=0;

unsigned long msb=0;
unsigned long lsb=0;
unsigned long xsb=0;
unsigned long buff[30];

char buf[4];

int Rx_flag = 0;
int Tx_flag = 0;
int Rx_cnt = 0;

int Rx_start_cnt = 0;
int Rx_finish_cnt = 0;

char init_reg_1;
char init_reg_2;
char init_reg_3;

int adc_P_val_1  = 0;
int adc_P_val_2  = 0;
int adc_P_val_3  = 0;

int init_adc_P_val_1 = 0;
int delta_P_val_1 = 0;

int init_adc_P_val_2 = 0;
int delta_P_val_2 = 0;

int init_adc_P_val_3 = 0;
int delta_P_val_3 = 0;

int initial_flag = 0;

int threshold_flag_1 = 0;
int threshold_flag_2 = 0;
int threshold_flag_3 = 0;
int threshold_val = 450;

int thumb_case = 0;
int Index_case = 0;
int middle_case = 0;


void UART_2_InitStructure()
{   //use only Rx pin

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2,ENABLE);
}


void TIM3_Configuration()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    TIM_TimeBaseInitTypeDef TIM3_BaseInitStructure;

    TIM3_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM3_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM3_BaseInitStructure.TIM_Prescaler = 72 -1;
    TIM3_BaseInitStructure.TIM_Period = 1000 -1;  //500HZ
    TIM3_BaseInitStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM3, &TIM3_BaseInitStructure);
    TIM_CounterModeConfig(TIM3, TIM_CounterMode_Up);

    TIM_Cmd(TIM3, ENABLE);
    TIM_InternalClockConfig(TIM3);
}

void UART_2_Rx_for_hand_pose()
{

    while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
           buff[Rx_cnt] = USART_ReceiveData(USART2);

           if(buff[Rx_cnt] == 0xfe)
           { Rx_finish_cnt++; }
           else
           { Rx_finish_cnt = 0;}

           if(Rx_cnt >= 2)
           {
               if(buff[1] != 0xff || buff[1] != 0xff)
               { Rx_cnt = 0; }
               else
               { Rx_cnt++;   }
           }
           else
           { Rx_cnt++; }

           if(Rx_finish_cnt >= 2)
           { Rx_cnt = 0;
             Rx_flag = 0;
           }
}

void hand_pose_step_update()
{
    thumb_case = buff[3];
    Index_case = buff[5];
    middle_case = buff[7];

}

void PeripheralInit_SPI1_Master()
{
    GPIO_InitTypeDef GPIO_InitDef;
    SPI_InitTypeDef SPI_InitDef;

    GPIO_StructInit(&GPIO_InitDef);
    SPI_StructInit(&SPI_InitDef);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    //SPI1_SCK pin Operate
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_AF_PP; //alternative function push-pull
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitDef);

    //SPI1_MISO --> Master input
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_IPU; //Input push-pull  mode
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitDef);

    //SPI1_MOSI --> Master Output
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitDef);

    //initialize SPI Master --> if Slave mode no need to define SPI_BaudRatePrescaler
    SPI_InitDef.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitDef.SPI_Mode = SPI_Mode_Master;
    SPI_InitDef.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitDef.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitDef.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitDef.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitDef.SPI_NSS = SPI_NSS_Soft;
    SPI_InitDef.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //--> APB2 72/64 = 1.125MHz
    SPI_InitDef.SPI_CRCPolynomial = 7;  //�� �𸣰ڴµ� �׳� ���� �������� 7 ���� CRC �� �� �ʱⰪ���� �ִ� value
    SPI_Init(SPI1, &SPI_InitDef);

    SPI_Cmd(SPI1, ENABLE);
}

void PeripheralInit_GPIO()
{
    GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_InitTypeDef GPIO_InitDef;
    GPIO_StructInit(&GPIO_InitDef);

    GPIO_InitDef.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2; //SENSOR_SS pin
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitDef);

    GPIO_InitTypeDef GPIOB_InitDef;
    GPIO_StructInit(&GPIOB_InitDef);

    GPIOB_InitDef.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10; //bmp_threshold generate
    GPIOB_InitDef.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOB_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOB_InitDef);

    GPIO_InitTypeDef Thumb_GPIO;
    GPIO_StructInit(&Thumb_GPIO);

    Thumb_GPIO.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    Thumb_GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    Thumb_GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &Thumb_GPIO);

    GPIO_InitTypeDef Index_GPIO;
    GPIO_StructInit(&Index_GPIO);

    Index_GPIO.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15 | GPIO_Pin_8; //A8 --> middle
    Index_GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    Index_GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &Index_GPIO);

    GPIO_InitTypeDef Middle_GPIO;
    GPIO_StructInit(&Middle_GPIO);

    Middle_GPIO.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; //bmp_threshold generate
    Middle_GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    Middle_GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &Middle_GPIO);
}

void write_at_register_for_setting(unsigned char reg, unsigned char data)
{
    for(int i = 0; i<=10 ; i++);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, reg);   // send
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, data);
    for(int i = 0; i<=10 ; i++); // --> �ᱹ ��������� delay�� �ʿ��ϴ�.
}

void PWR_CTRL_setting()
{
    unsigned char data;
    //data = 0x33;
    data = 0x31; //--> only pressure

    //Sensor_1
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    write_at_register_for_setting(PWR_CTRL, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_3);

    //Sensor_2
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    write_at_register_for_setting(PWR_CTRL, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_2);

    //Sensor_3
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    write_at_register_for_setting(PWR_CTRL, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

void OSR_setting()
{
    unsigned char data;
    data = 0x00;

    //Sensor_1
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    write_at_register_for_setting(OSR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_3);

    //Sensor_2
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    write_at_register_for_setting(OSR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_2);

    //Sensor_3
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    write_at_register_for_setting(OSR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);

    //Sensor_4
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
    write_at_register_for_setting(OSR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_0);

    //Sensor_5
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    write_at_register_for_setting(OSR, data);
    GPIO_SetBits(GPIOB, GPIO_Pin_9);

    //Sensor_6
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    write_at_register_for_setting(OSR, data);
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

void ODR_setting()
{
    unsigned char data;
    data = 0x00; //���߿� �̰� �ѹ� �ٲ㺸�� ������ ���� �� �缭

    //Sensor_1
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    write_at_register_for_setting(ODR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_3);

    //Sensor_2
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    write_at_register_for_setting(ODR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_2);

    //Sensor_3
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    write_at_register_for_setting(ODR, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

void IF_CONF_setting() //Init���� OSR�̳� ODR ������ �ؾ���(reset�ż�)
{
    unsigned char data;
    data = 0x00;

    //Sensor_1
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    write_at_register_for_setting(IF_CONFIG, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_3);

    //Sensor_2
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    write_at_register_for_setting(IF_CONFIG, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_2);

    //Sensor_3
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    write_at_register_for_setting(IF_CONFIG, data);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);

}
void bmp388_Init()
{
    //GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    ODR_setting();
    OSR_setting();
    IF_CONF_setting();
    PWR_CTRL_setting(); //power mode on 00 --> 11
    //GPIO_SetBits(GPIOA, GPIO_Pin_3);
}

int get_pressure_val(int SS)
{
    //read pressure adc value from register(raw_data)

    int adc_val = 0;

    switch(SS)
    {
    case 1:
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        SPI_Read_val();
        GPIO_SetBits(GPIOB, GPIO_Pin_0);
        adc_val = ((init_reg_1 - buf[3]) << 8) + buf[2];
        break;
    }
    case 2:
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        SPI_Read_val();
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
        adc_val = ((init_reg_2 - buf[3]) << 8) + buf[2];
        break;
    }
    case 3:
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_10);
        SPI_Read_val();
        GPIO_SetBits(GPIOB, GPIO_Pin_10);
        adc_val = ((init_reg_3 - buf[3]) << 8) + buf[2];
        break;

    }
    }

     return adc_val;
}

void get_initalizing_adc_val()
{
    init_adc_P_val_1 = get_pressure_val(1);
    init_reg_1 = buf[3];
    init_adc_P_val_2 = get_pressure_val(2);
    init_reg_2 = buf[3];
    init_adc_P_val_3 = get_pressure_val(3);
    init_reg_3 = buf[3];

    init_adc_P_val_1 = get_pressure_val(1);
    init_adc_P_val_2 = get_pressure_val(2);
    init_adc_P_val_3 = get_pressure_val(3);
}

void SPI_Read_val()
{
    for(int i = 0; i<=10 ; i++);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, 0x84);   // send
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, 0x00);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    buf[0] = SPI_I2S_ReceiveData(SPI1); //dummy
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, 0x00);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    buf[1] = SPI_I2S_ReceiveData(SPI1);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, 0x00);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    buf[2] = SPI_I2S_ReceiveData(SPI1);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, 0x00);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    buf[3] = SPI_I2S_ReceiveData(SPI1);
}

void sensor_val_update()
{
    adc_P_val_1 = get_pressure_val(1);
    adc_P_val_2 = get_pressure_val(2);
    adc_P_val_3 = get_pressure_val(3);

    delta_P_val_1 = adc_P_val_1 - init_adc_P_val_1;
    delta_P_val_2 = adc_P_val_2 - init_adc_P_val_2;
    delta_P_val_3 = adc_P_val_3 - init_adc_P_val_3;
}

void check_threshold()
{
    sensor_val_update();

    if (delta_P_val_1 >= threshold_val)
    { threshold_flag_1 = 1; }
    if (delta_P_val_2 >= threshold_val)
    { threshold_flag_2 = 1; }
    if (delta_P_val_3 >= threshold_val)
    { threshold_flag_3 = 1; }
    if (threshold_flag_1 == 1 && delta_P_val_1 <= 450)
    {
        threshold_flag_1 = 0;
    }
    if (threshold_flag_2 == 1 && delta_P_val_2 <= 1200)
    {
        threshold_flag_2 = 0;
    }
    if (threshold_flag_3 == 1 && delta_P_val_3 <= 1200)
    {
        threshold_flag_3 = 0;
    }

    threshold_to_pulse();
}
void threshold_to_pulse()
{
    if(threshold_flag_1 == 1)
    { make_pulse_threshold(1); }
    else
    { make_pulse_threshold(4); }
    if(threshold_flag_2 == 1)
    { make_pulse_threshold(2); }
    else
    { make_pulse_threshold(5); }
    if(threshold_flag_3 == 1)
    { make_pulse_threshold(3); }
    else
    { make_pulse_threshold(6); }


}

void make_pulse_threshold(int num)
{
    switch (num)
    {
    case 1: {GPIO_SetBits(GPIOA, GPIO_Pin_0); break;}
    case 2: {GPIO_SetBits(GPIOA, GPIO_Pin_1); break;}
    case 3: {GPIO_SetBits(GPIOA, GPIO_Pin_2); break;}
    case 4: {GPIO_ResetBits(GPIOA, GPIO_Pin_0); break;}
    case 5: {GPIO_ResetBits(GPIOA, GPIO_Pin_1); break;}
    case 6: {GPIO_ResetBits(GPIOA, GPIO_Pin_2); break;}

    }
}

void thumb_pulse()
{
    switch(thumb_case)
    {

    case 1:
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_3);
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            break;
        }
    case 2:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_3);
            GPIO_SetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            break;
        }
    case 3:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_3);
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_SetBits(GPIOB, GPIO_Pin_5);
            GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            break;
        }
    case 4:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_3);
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            GPIO_SetBits(GPIOB, GPIO_Pin_6);
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            break;
        }

    case 5:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_3);
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            GPIO_SetBits(GPIOB, GPIO_Pin_7);
            break;
        }

    }
}

void index_pulse()
{
    switch(Index_case)
    {

    case 1:
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_9);
            GPIO_ResetBits(GPIOA, GPIO_Pin_10);
            GPIO_ResetBits(GPIOA, GPIO_Pin_11);
            GPIO_ResetBits(GPIOA, GPIO_Pin_12);
            GPIO_ResetBits(GPIOA, GPIO_Pin_15);
            break;
        }
    case 2:
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            GPIO_SetBits(GPIOA, GPIO_Pin_10);
            GPIO_ResetBits(GPIOA, GPIO_Pin_11);
            GPIO_ResetBits(GPIOA, GPIO_Pin_12);
            GPIO_ResetBits(GPIOA, GPIO_Pin_15);
            break;
        }
    case 3:
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            GPIO_ResetBits(GPIOA, GPIO_Pin_10);
            GPIO_SetBits(GPIOA, GPIO_Pin_11);
            GPIO_ResetBits(GPIOA, GPIO_Pin_12);
            GPIO_ResetBits(GPIOA, GPIO_Pin_15);
            break;
        }
    case 4:
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            GPIO_ResetBits(GPIOA, GPIO_Pin_10);
            GPIO_ResetBits(GPIOA, GPIO_Pin_11);
            GPIO_SetBits(GPIOA, GPIO_Pin_12);
            GPIO_ResetBits(GPIOA, GPIO_Pin_15);
            break;
        }

    case 5:
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            GPIO_ResetBits(GPIOA, GPIO_Pin_10);
            GPIO_ResetBits(GPIOA, GPIO_Pin_11);
            GPIO_ResetBits(GPIOA, GPIO_Pin_12);
            GPIO_SetBits(GPIOA, GPIO_Pin_15);
            break;
        }

    }
}

void middle_pulse()
{
    switch(middle_case)
    {

    case 1:
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOA, GPIO_Pin_8);
            break;
        }
    case 2:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
            GPIO_SetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOA, GPIO_Pin_8);
            break;
        }
    case 3:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            GPIO_SetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOA, GPIO_Pin_8);
            break;
        }
    case 4:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOA, GPIO_Pin_8);
            break;
        }

    case 5:
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            GPIO_SetBits(GPIOA, GPIO_Pin_8);
            break;
        }

    }
}

int main(void)
{
    PeripheralInit_SPI1_Master();
    PeripheralInit_GPIO();
    TIM3_Configuration();
    UART_2_InitStructure();

    //Initializing step
    for(int i = 0; i <= 3; i++) {buf[i] = 0;}

    bmp388_Init();
    get_initalizing_adc_val();

  while(1)
  {
      if(Rx_flag == 1)
       { UART_2_Rx_for_hand_pose(); }

      if(TIM_GetFlagStatus(TIM3, TIM_FLAG_Update) == SET)
          {
              sensor_val_update();
              check_threshold();
              hand_pose_step_update();

              if(Rx_flag != 1)
              {
                  thumb_pulse();
                  index_pulse();
                  middle_pulse();
                  Rx_flag = 1;
              }
              TIM_ClearFlag(TIM3, TIM_FLAG_Update);
          }

    }
}
