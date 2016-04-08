#define A_SENSE         A0
#define BFLAT_SENSE     A1
#define B_SENSE         A2
#define C_SENSE         A3
#define CSHARP_SENSE    A4
#define D_SENSE         A5
#define DSHARP_SENSE    A6
#define E_SENSE         A7
#define F_SENSE         A8
#define FSHARP_SENSE    A9
#define G_SENSE         A10
#define GSHARP_SENSE    A11

void setup() {
    pinMode(A_SENSE, INPUT);
    pinMode(BFLAT_SENSE, INPUT);
    pinMode(B_SENSE, INPUT);
    pinMode(C_SENSE, INPUT);
    pinMode(CSHARP_SENSE, INPUT);
    pinMode(D_SENSE, INPUT);
    pinMode(DSHARP_SENSE, INPUT);
    pinMode(E_SENSE, INPUT);
    pinMode(F_SENSE, INPUT);
    pinMode(FSHARP_SENSE, INPUT);
    pinMode(G_SENSE, INPUT);
    pinMode(GSHARP_SENSE, INPUT);
}

void loop() {

}


volatile boolean l;

void TC0_Handler()
{
    long dummy=REG_TC0_SR0; // vital - reading this clears some flag
    // otherwise you get infinite interrupts
    l= !l;
}

void setup(){
    pinMode(13,OUTPUT);
    pinMode(2,OUTPUT);    // port B pin 25  
    analogWrite(2,255);   // sets up some other registers I haven't worked out yet
    REG_PIOB_PDR = 1<<25; // disable PIO, enable peripheral
    REG_PIOB_ABSR= 1<<25; // select peripheral B
    REG_TC0_WPMR=0x54494D00; // enable write to registers
    REG_TC0_CMR0=0b00000000000010011100010000000000; // set channel mode register (see datasheet)
    REG_TC0_RC0=100000000; // counter period
    REG_TC0_RA0=30000000;  // PWM value
    REG_TC0_CCR0=0b101;    // start counter
    REG_TC0_IER0=0b00010000; // enable interrupt on counter=rc
    REG_TC0_IDR0=0b11101111; // disable other interrupts

    NVIC_EnableIRQ(TC0_IRQn); // enable TC0 interrupts

}

void loop(){
    digitalWrite(13,l);
}
