/**
  Section: Included Files
*/

#include "mcc_generated_files/mcc.h"
#include "application.h"
#include "macros.h"

extern volatile uint8_t timer0overflowFlag;// flag is set and incremented in timer0.c file at timer0 overflow event
extern volatile bool clc1FirstRisingEdge;// flag is set at initialization of CLC1 in CLC1.c
extern volatile bool clc1risingEdgeFlag;// flag is set in CLC1.c file when there is rising edge on CLC1 output 
extern volatile bool clc5RisingEdgeFlag;// flag is set in CLC5.c file when there is rising edge on CLC5 output 

key_t event;

static uint8_t Reverse(uint8_t byteVar);   
static uint8_t PrepareNewCMD (e_key swNum);
static uint32_t Prepare_IR_NEC_Data(uint8_t devAddress, uint8_t cmd);    
static e_status SendIR_Frame (uint32_t IR_Data, uint8_t numOfBitsToSend); 
static e_key_state GetEvent(void);
static void ClearEvent(void);
static e_key GetButton(void);
static void ClearButton(void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
     main application function being executed in while loop
 * check for button press and release event, prepare command according to the button press event,
 * prepare 32 bit data as per NEC IR protocol, send IR frame for the command using DSM
 * @Example
    ApplicationTask();
 */
void ApplicationTask(void) {
    static app_state state=GET_EVENT;
    static e_key button;
    static uint32_t data;
    static uint8_t command;   
    static uint8_t rxAddress; 
    uint8_t retStatus = 0;
    
    switch(state) {
    case GET_EVENT:
        if(GetEvent())//check if there is button press and release event
        {
            button = GetButton();  
            if(button < MAX_KEYS)
            {
                state = PREPARE_CMD;//prepare command for button press and release event
            }
            ClearButton();
            ClearEvent();
        }
        else
        {
          
        }
        break;
    case PREPARE_CMD:
        if(button == SW1)
        {
          LED_SetLow();
          __delay_ms(200);
          LED_SetHigh(); 
        }
        else 
        {
            // Nothing
        }
        command = PrepareNewCMD(button);//prepare new command for button press and release events
        if(command != 0)
        {
            //Reverse the bits in the command to be transmitted from LSb to MSb
            //as per NEC protocol LSb is transmitted first
            command = Reverse (command);
            state = PREPARE_IR_NEC_DATA;
        }
        else
        {
            state = GET_EVENT;
        }
        button = NO_KEY;
    break;
    case PREPARE_IR_NEC_DATA:
        //Reverse the bits in the Receiver address to be transmitted from LSb to MSb
        //as per NEC protocol LSb is transmitted first
        rxAddress=Reverse((uint8_t)IR_RX_ADDRESS);
        data = Prepare_IR_NEC_Data(rxAddress,command);//prepare 32 bit data to be sent in the IR frame
        if(data != 0)
        {
            state = SEND_DATA_OVER_IR;
        }
        else
        {
            state = GET_EVENT;
        }       
    break;
    case SEND_DATA_OVER_IR:
        retStatus = SendIR_Frame(data,MAXCOMMANDBITS_1);// send IR frame
        if(retStatus == ERROR)
        {
            state = COMM_ERR;
        }
        else if(retStatus == SUCCESS)//if IR frame is sent successfully then go to idle state
        {
            state = GET_EVENT;
        }
    break;
    case COMM_ERR:
        // error event can be added
        state = GET_EVENT;
    break;
    default:
        state = GET_EVENT;
        break;
    }
}

/**
 * @Param
    1 byte value
 * @Returns
    reverse of the byte with LSb as MSb
 * @Description
    In NEC protocol LSb is transmitted first. 
    This function is used to reverse the bits in the data byte and decode the byte received with MSb first.
 * @Example
    Reverse();
 */
static uint8_t Reverse(uint8_t byteVar) 
{
   byteVar = (byteVar & UPPER_NIBBLE) >> 4 | (byteVar & LOWER_NIBBLE) << 4;
   byteVar = (byteVar & BIT7632) >> 2 | (byteVar & BIT5410) << 2;
   byteVar = (byteVar & BIT7531) >> 1 | (byteVar & BIT6420) << 1;
   return byteVar;
}

/**
 * @Param
   e_key swNum: value of the switch pressed 
 * @Returns
   8 bit command
 * @Description
    This function is used to prepare an 8 bit command corresponding to the button press event.
 * @Example
    command= PrepareNewCMD(SW1);
 */
static uint8_t PrepareNewCMD (e_key swNum)
{
   static uint8_t sw1PressedIndex;
   
   uint8_t newCommand = 0;
   
        if (swNum == SW1)
        { 
            sw1PressedIndex++;
            //if switch S2 is pressed command is transmitted as switch press count+0x80 from 0x81 to 0x88
            newCommand = sw1PressedIndex+COMMAND2MIN;
            if(sw1PressedIndex>=COMMAND1MAX)
            {
                sw1PressedIndex=0;
            }
        }
   return newCommand;
}

/**
 * @Param
   8 bit IR Receiver Address , 8 bit command
 * @Returns
   None
 * @Description
    This function is used to prepare 32 bits command to be transmitted after button press event. 
 * 32 bit IR Data as per NEC protocol for 8 bit command transmission: 
    address byte, inverse of address byte, command byte, inverse of command byte
 * @Example
    IR_DATA=Prepare_IR_NEC_Data(RxAddress, command);
 */
static uint32_t Prepare_IR_NEC_Data(uint8_t devAddress, uint8_t cmd)
{
    uint32_t retData = 0;
    
    retData = devAddress;// receiver address
    retData <<= 8;
    retData |= (uint8_t)~devAddress;//inverse of receiver address
    retData <<= 8;
    retData |= (uint8_t)cmd;//command
    retData <<= 8;
    retData |= (uint8_t)~cmd;//inverse of command
    return retData;   
}
/**
 * @Param
   IR_Data to be transmitted , numOfBitsToSend
 * @Returns
    status=1 if frame transmitted successfully
 *  status=0xFF if error in transmission
 * @Description
    This function is used to send command using NEC IR protocol. 
 *  NEC protocol
    //Logical '0' ? a 562.5?s pulse burst followed by a 562.5?s space, with a total transmit time of 1.125ms
    //Logical '1' ? a 562.5?s pulse burst followed by a 1.6875ms space, with a total transmit time of 2.25ms
    //Complete IR frame for 1byte command transmission
    //9ms leading pulse burst (16 times the pulse burst length used for a logical data bit)
    //4.5ms space
    //the 8-bit address for the receiving device
    //the 8-bit logical inverse of the address
    //the 8-bit command
    //the 8-bit logical inverse of the command
    //a final 562.5?s pulse burst to signify the end of message transmission.
 * @Example
    frameSentFlag= SendIR_Frame(IR_Data, numOfBitsToSend);
 */
static e_status SendIR_Frame (uint32_t IR_Data, uint8_t numOfBitsToSend)
{
    static e_IrTxState logicState = START_TMR0;
    static uint8_t bitCounter = 0;
    static uint32_t tempData = 0;
    
    e_status retValue;
    
    switch (logicState)
    {
    case START_TMR0:
        TIMER0_PR_4_5ms();//Set the Timer overflow to 4.5 ms for start of transmission
        TMR0_WriteTimer(ZERO);
        timer0overflowFlag = 0;
        EnableTMR0Intr();//enable timer 0 overflow interrupt
        TMR0_StartTimer();//start timer 0
        DSM_ManualModulationSet();//set DSM modulation source to transmit MARK in the start of IR frame
        DSM_ModulationStart();
        logicState = START_BIT_MARK;
        break;
    case START_BIT_MARK: //send 9ms leading pulse burst (MARK)   
        if (timer0overflowFlag == MARK_DURATION)//check if timer has overflowed 2 times for 4.5*2=9 ms duration
        {    
            DSM_ManualModulationClear();
            timer0overflowFlag = 0;
            logicState = START_BIT_SPACE;
        }      
        break; 
    case START_BIT_SPACE://send 4.5ms SPACE//9ms MARK + 4.5ms SPACE = Start sequence in NEC IR Frame
        if(timer0overflowFlag)
            {
                tempData = IR_Data;
                bitCounter = numOfBitsToSend;
                if(tempData & BIT31_HIGH) 
                    {       
                        //logic '1'transmit "1000" a 562.5µs pulse burst followed by a 1.6875ms space
                        CLC_logic_1_Initialize();      
                         //DSM source is CLC1 output
                        DSM_SRC_Set(SRC_CLC1_OUT);
                        logicState = LOGIC_1;
                    }
                else 
                    { 
                        //logic '0'transmit "10"  a 562.5µs pulse burst followed by a 562.5µs space  
                        CLC_logic_0_Initialize();  
                        //DSM source is CLC5 output
                        DSM_SRC_Set(SRC_CLC5_OUT);
                        logicState = LOGIC_0;
                    }
                tempData <<= 1;            
                TIMER0_PR_562_5us();//change the timer period value to transmit 32 bit data in IR frame at 562.5 us period 
                                    //for transmitting Logic '0' or Logic '1'
                timer0overflowFlag = 0;
                // Disabling TMR0 interrupt. All interrupts are handled in CLC1 and CLC5 ISR
                DisableTMR0Intr();
            }
        break;
    case LOGIC_1:
       if(clc1risingEdgeFlag)
        {    
            if(bitCounter == 0) //check if all bits are transmitted
            {  
                DSM_ManualModulationSet();//for 562.5 us stop bit
                DSM_SRC_Set(SRC_SW_BIT);// DSM source is software bit;
                logicState = STOP_BIT;
                CLC_logic_1_Stop();
                clc1FirstRisingEdge = false;
                EnableTMR0Intr();// enabling TMR0 interrupt to service stop bit
            } 
            else 
            {        
                if(!(tempData & BIT31_HIGH)) //check if the next data bit to be transmitted is logic 0
                {
                    //enable logic 0 CLC                  
                    CLC_logic_0_Initialize(); 
                    //Set DSM source to CLC5 output
                    DSM_SRC_Set(SRC_CLC5_OUT);//switch to logic 0
                    CLC_logic_1_Stop();  
                    clc1FirstRisingEdge = false;
                    logicState = LOGIC_0;
                }
                bitCounter--;
                tempData <<= 1;
            }
            clc1risingEdgeFlag = 0;
        }
    break;
    case LOGIC_0:
        if(clc5RisingEdgeFlag)
        {    
            if(bitCounter == 0)//check if all bits are transmitted 
            {  
                DSM_ManualModulationSet();//for 562.5 us stop bit
                DSM_SRC_Set(SRC_SW_BIT);// DSM source is software bit;   
                logicState = STOP_BIT;
                CLC_logic_0_Stop();
                EnableTMR0Intr();// enabling TMR0 interrupt to service stop bit 
            } 
            else 
            {        
                if(tempData & BIT31_HIGH)//check if the next data bit to be transmitted is logic 1
                {
                    //enable logic 1 CLCs 
                    CLC_logic_1_Initialize();
                    //Set DSM source to CLC1 output
                    DSM_SRC_Set(SRC_CLC1_OUT);//switch to logic 1
                    CLC_logic_0_Stop();           
                    logicState = LOGIC_1;
                }
                bitCounter--;
                tempData <<= 1;
            }
            clc5RisingEdgeFlag = 0;
        }
    break;
    case STOP_BIT:// All the bits are transmitted, transmit the end sequence a pulse burst of 562.5 us
        if(timer0overflowFlag)
        {
            logicState = STOP_FRAME;
            timer0overflowFlag = 0;
        }        
        break;
    case STOP_FRAME:  // IR frame is transmitted reinitialize get ready for next transmission  
        if(timer0overflowFlag)
        {
            TMR0_StopTimer();
            TIMER0_PR_4_5ms();//reload timer for overflow at every 4.5 ms for start of transmission
            DSM_ManualModulationClear();
            DSM_ModulationStop();
            logicState = START_TMR0;
            timer0overflowFlag = 0;
            DisableTMR0Intr();
            retValue = SUCCESS;
        }
    break;
    default://error state
        retValue = ERROR;
        logicState = START_TMR0;
        break;  
    }
    return retValue;
    
}

/**
 * @Param
    None
 * @Returns
    Status of any switch press event
 * @Description
    This function is used to get information whether there is any event for generating command.
 * @Example
    event_status =GetEvent();
 */
static e_key_state GetEvent(void)
{
    return event.keyStatus;//key status is set in tmr4.c if switch S2 is pressed and released
                           //key status is set in tmr6.c if switch S1 is pressed and released
}

/**
 * @Param
    None
 * @Returns
    None
 * @Description
    This function is used to clear previous switch press event after 
 *  corresponding  action is taken for the event.
 * @Example
    ClearEvent();
 */
static void ClearEvent(void)
{
    event.keyStatus = KEY_NO_ACTION;
}

/**
 * @Param
    None
 * @Returns
    Which switch is pressed
 * @Description
    This function is used to get information about which switch is pressed for generating command.
 * @Example
    button=GetButton();
 */
static e_key GetButton(void)
{
    return event.key;
}

/**
 * @Param
    None
 * @Returns
    None
 * @Description
    This function is used to clear previous switch button presses after 
 *  corresponding  action is taken for the button press event.
 * @Example
    ClearButton();
 */
static void ClearButton(void)
{
    event.key = NO_KEY;
}

/**
 * @Param
    None
 * @Returns
    None
 * @Description
 Timer 0 User interrupt handler function
 */
void TMR0_UserInterruptHandler(void)
{
      timer0overflowFlag++;
}

/**
 * @Param
    None
 * @Returns
    None
 * @Description
 Timer 4 User interrupt handler function
 */
void TMR4_UserInterruptHandler(void)
{
    event.key = SW1;//switch S2 is pressed and released
    event.keyStatus = KEY_PRESSED;
}