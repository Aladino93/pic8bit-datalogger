#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config PLLDIV = 1       // PLL Prescaler Selection bits (No prescale (4 MHz oscillator input drives PLL directly))
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset  (Disabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide)
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)
#pragma config OSC = INTOSC     // Oscillator (INTOSC)
#pragma config T1DIG = OFF      // T1OSCEN Enforcement (Secondary Oscillator clock source may not be selected)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator (High-power operation)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)
#pragma config RTCOSC = INTOSCREF// RTCC Clock Select (RTCC uses INTRC)

#include <xc.h>
#include <i2c.h>



unsigned short somma_indirizzi = 0x0000;

void EE_byte_write_singolo( unsigned char address_byte1 , unsigned char address_byte2 , unsigned char data1  )
//NB IL LINKER DEVE AVER CHECKATO il link in peripheral
{
unsigned char device_adress_byte ;                         // byte address  l'ottavo bit è a 0 per operazioni di write  

device_adress_byte = 0b10100110 ;  // indirizzo del nostro specifico I2C

SSPSTAT=0x80;
SSPADD=0x18;                                     //100khz baud rate
SSPCON1=0x28;                                    // Master mode in I2c.
SSPCON2=0x00;
TRISBbits.TRISB4=1;
TRISBbits.TRISB5=1;
OpenI2C(MASTER, SLEW_ON);// Initialize I2C module
SSPADD = 9; //400kHz Baud clock(9) @16MHz
SSP1CON1= 0x00;
SSP1CON1bits.SSPM01=0;
SSP1CON1bits.SSPM11=0;
SSP1CON1bits.SSPM21=0;
SSP1CON1bits.SSPM31=0;
SSP1STATbits.SMP=0;
SSPEN1= 1;

SSP1CON2bits.SEN = 1;
    
//OpenI2C1( MASTER,  SLEW_OFF );
//StartI2C1();      // start I2C
if(WriteI2C1(device_adress_byte)<0);  // Send address with R/W set for write - Error in WriteI2C() documentation!
IdleI2C1();
if(WriteI2C1(address_byte1)<0);
IdleI2C1();
if(WriteI2C1(address_byte2)<0);
IdleI2C1();
if(WriteI2C1(data1)<0);
IdleI2C1();
StopI2C1();                                          // stops I2C
CloseI2C1();

return;                                    
}

void main(void) {
    
    OSCCONbits.IRCF  = 0b111; // setto il clock interno di sistema 111=8Mhz
    PADCFG1 = 0x05; //come fare le due cose commentate sotto   
    // RTSECSEL1 = 1; 
    // RTSECSEL0 = 0; 
    EECON2 = 0x55; // queste letture alla flash sono obbligatorie prima di accedere a RTCWREN
    EECON2 = 0xAA;
    RTCCFGbits.RTCWREN = 1; // NB VA FATTO IL CICLO DI CLOCK IMMEDIATO DOPO EECON2 = 0xAA;
    // RTCC 
    // si protegge la scrittura
    RTCCFGbits.RTCPTR1 = 1;
    RTCCFGbits.RTCPTR0 = 1;
                               // RTCVALH | RTCVALL
    RTCVALL = 0x0B;            // reserved | year
    RTCVALH = 0xFF;

    RTCVALL = 0x01;            // month | day
    RTCVALH = 0x01;

    RTCVALL = 0x12;            // weekday | hours
    RTCVALH = 0x01;

    RTCVALL = 0x00;            // minutes | seconds
    RTCVALH = 0x00;
                  
    RTCCFGbits.RTCEN = 0;
    RTCCFGbits.RTCEN = 1;   

    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    RTCCFGbits.RTCWREN = 0;
    
    ALRMCFGbits.ALRMEN=0;  
    
    ALRMPTR1 = 1;
    ALRMPTR0 = 0;

    ALRMVALL = 0x01;            // month | day
    ALRMVALH = 0x01;

    ALRMVALL= 0x12;            // weekday | hours
    ALRMVALH= 0x01;

    ALRMVALL = 0x02;            // minutes | seconds
    ALRMVALH = 0x00;
    
    ALRMRPTbits.ARPT  = 0b00000000; // contatore degli allarmi (che tanto vanno all'infinito)
    ALRMCFGbits.CHIME = 1;   // continua a ripetere l'allarme all'infinito
    ALRMCFGbits.AMASK = 0b0011; // allarme ogni minuto
    ALRMCFGbits.ALRMEN= 1; // enable definitivo dell'allarme
    RTCCIF=0; // azzero flag dell'interrupt del RTC
    RTCCIE=1; //enable di interrupt del RTC
    INTCONbits.GIEL = 1; // Enables all low priority interrupts
    INTCONbits.GIEH = 1; // Enables all low priority interrupts   
   
    TRISB = 1; // configuro porte dell'I2C come input 
    
    while(1){    
     Sleep();                 //EE_byte_write_singolo( 0x00 , 0x05 , 0xFF  );        
    };
   
    return;
}


void interrupt interruzione()          
{
    
    if (PIR3bits.RTCCIF == 1)
    {   

        unsigned char lettura_a_vuoto;
        //unsigned char indirizzo1=(char)(somma_indirizzi>>8);
        //unsigned char indirizzo2=(char)somma_indirizzi;    
        RTCCFGbits.RTCPTR1 = 1;
        RTCCFGbits.RTCPTR0 = 1;
        //EE_byte_write_singolo( indirizzo1 , indirizzo2 , RTCVALL );//year
        lettura_a_vuoto=RTCVALH;
        EE_byte_write_singolo( (char)(somma_indirizzi>>8) , (char)somma_indirizzi , RTCVALL );//day
        lettura_a_vuoto=RTCVALH;
        somma_indirizzi=somma_indirizzi++;
        EE_byte_write_singolo( (char)(somma_indirizzi>>8) , (char)somma_indirizzi , lettura_a_vuoto );//month
        somma_indirizzi=somma_indirizzi++;
        EE_byte_write_singolo( (char)(somma_indirizzi>>8) , (char)somma_indirizzi , RTCVALL );//ora
        somma_indirizzi=somma_indirizzi++;
        lettura_a_vuoto=RTCVALH;
        EE_byte_write_singolo( (char)(somma_indirizzi>>8) , (char)somma_indirizzi , RTCVALH ); //minuti
        somma_indirizzi=somma_indirizzi++;
        //aggiungere scrittura di temperatura
        // EE_byte_write_singolo( (char)(somma_indirizzi>>8) , (char)somma_indirizzi , VALORE_TEMPERATURA );    
        PIR3bits.RTCCIF = 0;
    }
}
