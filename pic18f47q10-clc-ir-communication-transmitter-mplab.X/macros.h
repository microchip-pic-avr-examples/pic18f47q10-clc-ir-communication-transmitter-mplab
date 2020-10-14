#ifndef MACROS_H
#define	MACROS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
//Enable CLC1,2,3 and 4, set CLC1 D-FF, Reset CLC2, CLC3 D-FFs
#define CLC_logic_1_Initialize() CLC1CONbits.EN = 1;\
                    CLC2CONbits.EN = 1;\
                    CLC3CONbits.EN = 1;\
                    CLC4CONbits.EN = 1;\
                    CLC1POLbits.G4POL = 1;\
                    CLC1POLbits.G4POL = 0;\
                    CLC2POLbits.G3POL = 1;\
                    CLC2POLbits.G3POL = 0;\
                    CLC3POLbits.G3POL = 1;\
                    CLC3POLbits.G3POL = 0

//Enable CLC5
#define CLC_logic_0_Initialize() CLC5CONbits.EN = 1;\

//Disable CLC1, 2, 3 and 4
#define CLC_logic_1_Stop() CLC1CONbits.EN = 0;\
                    CLC2CONbits.EN = 0;\
                    CLC3CONbits.EN = 0;\
                    CLC4CONbits.EN = 0

//Disable CLC5
#define CLC_logic_0_Stop() CLC5CONbits.EN = 0

#define DSM_SRC_Set(x) MDSRC = x
    
#define TIMER0_PR_4_5ms() T0CON1bits.T0CKPS= 0b1000 //clock prescaler 1:256 for period of 4.5 ms with TMRH 0x8C   
#define TIMER0_PR_562_5us() T0CON1bits.T0CKPS= 0b0101 //clock prescaler 1:32 for period of 562.5 us with TMRH 0x8C   
#define DisableTMR0Intr() PIE0bits.TMR0IE = 0
#define EnableTMR0Intr() PIE0bits.TMR0IE = 1
    
#ifdef	__cplusplus
}
#endif

#endif	/* MACROS_H */

