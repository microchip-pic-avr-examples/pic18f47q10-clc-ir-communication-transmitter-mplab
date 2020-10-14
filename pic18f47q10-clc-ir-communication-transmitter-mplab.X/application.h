#ifndef APPLICATION_H
#define	APPLICATION_H

#ifdef	__cplusplus
extern "C" {
#endif

//IR receiver Address
#define IR_RX_ADDRESS   (uint8_t)0xAA
    
//used for generating commands    
#define COMMAND1MAX (uint8_t)8
#define COMMAND2MAX (uint8_t)8
#define COMMAND2MIN (uint8_t)0x80

//number of data bits for sending command using NEC IR protocol    
#define MAXCOMMANDBITS (uint8_t)32
#define MAXCOMMANDBITS_1 (uint8_t)31
    
//used in reversing the bits from MSb to LSb 
#define UPPER_NIBBLE    (uint8_t)0xF0
#define LOWER_NIBBLE    (uint8_t)0x0F
#define BIT7632         (uint8_t)0xCC
#define BIT5410         (uint8_t)0x33
#define BIT7531         (uint8_t)0xAA
#define BIT6420         (uint8_t)0x55 

#define BIT31_HIGH 0x80000000UL  
#define ZERO (uint8_t)0

#define MARK_DURATION 2U// used for counting timer 0 overflows for sending MARK of 9 ms duration= 2 timer 0 overflows of 4.5ms each 
    
// DSM modulation source
#define SRC_SW_BIT      (uint8_t)0x01
#define SRC_CLC1_OUT    (uint8_t)0x10
#define SRC_CLC5_OUT    (uint8_t)0x14
 
    
//IR data transmission states
typedef enum {
START_TMR0,
START_BIT_MARK,
START_BIT_SPACE,
LOGIC_1,
LOGIC_0,
STOP_BIT,
STOP_FRAME       
}e_IrTxState;
    
//Application task states
typedef enum {
    GET_EVENT,
    PREPARE_CMD,
    PREPARE_IR_NEC_DATA,                
    SEND_DATA_OVER_IR,
    COMM_ERR    
}app_state;

typedef enum {
  ERROR = 0xFF,
  SUCCESS = 1        
}e_status;
    
typedef enum {
    KEY_NO_ACTION,
    KEY_PRESSED,
    KEY_RELEASED,                
    MAX_KEY_STATE
}e_key_state;

typedef enum {
    NO_KEY,
    SW1,
    MAX_KEYS
}e_key;

typedef struct {
    e_key key;
    e_key_state keyStatus;
}key_t;

/**
 * @Param
    none
 * @Returns
    none
 * @Description
     main application function being executed in while loop
 * check for button press event, prepare command according to the button press event,
 * prepare 32 bit data as per NEC IR protocol, send IR frame for the command using DSM
 * @Example
    ApplicationTask();
 */    
void ApplicationTask(void);

/**
 * @Param
    None
 * @Returns
    None
 * @Description
 Timer 0 User interrupt handler function
 */
void TMR0_UserInterruptHandler(void);

/**
 * @Param
    None
 * @Returns
    None
 * @Description
 Timer 4 User interrupt handler function
 */
void TMR4_UserInterruptHandler(void);

#ifdef	__cplusplus
}
#endif

#endif	/* APPLICATION_H */

