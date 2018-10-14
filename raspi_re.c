#include <raspi_re.h>

#include <wiringPi.h>

const int PIN_A[] = {
    PIN_A0, PIN_A1, PIN_A2, PIN_A3
};

const int PIN_D[] = {
    PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7
};

int _re_data_direction;

void _re_pinMode(int, int);

void _re_digitalWrite(int, int);

int _re_digitalRead(int);

/*
 *
 */
int re_init() {

    int rc;
    int i;

    rc = wiringPiSetup();
    if (rc != 0) return 1;

    pinMode(PIN_IRQ, INPUT);
    pullUpDnControl(PIN_IRQ, PUD_UP);

    pinMode(PIN_WR, OUTPUT);
    digitalWrite(PIN_WR, HIGH);

    pinMode(PIN_IC, OUTPUT);
    digitalWrite(PIN_IC, HIGH);
    
    pinMode(PIN_CS0, OUTPUT);
    digitalWrite(PIN_CS0, HIGH);
    
    pinMode(PIN_RD, OUTPUT);
    digitalWrite(PIN_RD, HIGH);

    for (i = 0; i < (sizeof PIN_A / sizeof PIN_A[0]); i++) {
        pinMode(PIN_A[i], OUTPUT);
        digitalWrite(PIN_A[i], LOW);
    }

    for (i = 0; i < (sizeof PIN_D / sizeof PIN_D[0]); i++) {
        pinMode(PIN_D[i], OUTPUT);
        digitalWrite(PIN_D[i], LOW);
    }
    _re_data_direction = OUTPUT;

    return 0;
}

/*
 *
 */
void re_reset() {

    re_ic(LOW);

    delayMicroseconds(1000);

    re_ic(HIGH);
}

/*
 *
 */
void re_address(uint8_t address) {

    int i;
    uint8_t aa = address;

    for (i = 0; i < (sizeof PIN_A / sizeof PIN_A[0]); i++) {
        digitalWrite(PIN_A[i], aa & 1);
        aa = aa >> 1;
    }
}

/*
 *
 */
void re_write_data(uint8_t data) {

    int i;
    uint8_t dd = data;

    for (i = 0; i < (sizeof PIN_D / sizeof PIN_D[0]); i++) {
        if (_re_data_direction == INPUT) {
            pinMode(PIN_D[i], OUTPUT);
        }
        digitalWrite(PIN_D[i], dd & 1);
        dd = dd >> 1;
    }
    _re_data_direction = OUTPUT;
}

/*
 *
 */
uint8_t re_read_data() {

    int i;
    uint8_t dd = 0;

    for (i = (sizeof PIN_D / sizeof PIN_D[0]) - 1; i >= 0; i--) {
        if (_re_data_direction == OUTPUT) {
            pinMode(PIN_D[i], INPUT);
            pullUpDnControl(PIN_D[i], PUD_UP);
        }
        dd = dd << 1;
        dd |= _re_digitalRead(PIN_D[i]) & 1;
    }
    _re_data_direction = INPUT;

    return dd;
}

/*
 *
 */
int re_irq() {
    return digitalRead(PIN_IRQ);
}

/*
 *
 */
void re_wr(int b) {
    digitalWrite(PIN_WR, b);
}

/*
 *
 */
void re_ic(int b) {
    digitalWrite(PIN_IC, b);
}

/*
 *
 */
void re_cs0(int b) {
    digitalWrite(PIN_CS0, b);
}

/*
 *
 */
void re_rd(int b) {
    digitalWrite(PIN_RD, b);
}

void _re_pinMode(int pin, int mode) {

    pinMode(pin, mode);

    if (mode == INPUT) {
        pullUpDnControl(pin, PUD_UP);
    }
}

void _re_digitalWrite(int pin, int value) {

    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);
}

int _re_digitalRead(int pin) {

    pinMode(pin, INPUT);
    pullUpDnControl(pin, PUD_UP);
    return digitalRead(pin);
}

