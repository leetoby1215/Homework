#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "mbed_rpc.h"

#define UINT14_MAX 16383
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1)
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1)
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1)
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1)
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);

Thread RPC_thread;
Thread acc_thread;
EventQueue RPC_queue(32 * EVENTS_EVENT_SIZE);
EventQueue acc_queue(32 * EVENTS_EVENT_SIZE);

float acc_value[1000][3];
int counter = 0;
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
uint8_t who_am_i, data[2], res[6];
int16_t acc16;

void acc(void);
void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);
void getValue(Arguments *in, Reply *out);
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);

RPCFunction GETVALUE(&getValue, "getValue");

int main(void) {
    FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data, 2);
    
    pc.baud(9600);

    char xbee_reply[4];

    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();

    if (xbee_reply[0] == 'O' && xbee_reply[1] == 'K') {
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 0x405\r\n");
    reply_messange(xbee_reply, "setting MY : 405");
    xbee.printf("ATDL 0x415\r\n");
    reply_messange(xbee_reply, "setting DL : 415");
    xbee.printf("ATID 0x12\r\n");
    reply_messange(xbee_reply, "setting PAN ID : 12");
    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");
    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");
    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");
    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    xbee.getc();
    pc.printf("start\r\n");
    acc_thread.start(&acc);
    RPC_thread.start(callback(&RPC_queue, &EventQueue::dispatch_forever));
    xbee.attach(xbee_rx_interrupt, Serial::RxIrq);
}

void getValue(Arguments *in, Reply *out) {
    xbee.printf("%d\r\n", counter);
    for (int i = 0; i < counter; i++) {
        xbee.printf("%1.4f\n%1.4f\n%1.4f\r\n", acc_value[i][0], acc_value[i][1], acc_value[i][2]);
    }
    counter = 0;
}

void xbee_rx_interrupt(void) {
    xbee.attach(NULL, Serial::RxIrq);
    RPC_queue.call(&xbee_rx);
}

void xbee_rx(void) {
    char buf[100];
    char outbuf[100] = {0};
    memset(buf, 0, 100);
    
    while (xbee.readable()) {
        for (int i = 0; ; i++) {
            char recv = xbee.getc();
            if (recv == '\r') {
                break;
            }
            buf[i] = pc.putc(recv);
        }
        RPC::call(buf, outbuf);
        pc.printf("%s\r\n", outbuf);
        wait(0.1);
    }
    xbee.attach(xbee_rx_interrupt, Serial::RxIrq);
}

void reply_messange(char *xbee_reply, char *messange) {
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    if (xbee_reply[1] == 'O' && xbee_reply[2] == 'K') {
        pc.printf("%s\r\n", messange);
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
        xbee_reply[2] = '\0';
    }
}

void check_addr(char *xbee_reply, char *messenger) {
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    xbee_reply[3] = xbee.getc();
    pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
}
void acc(void) {
    while (true) {
        FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);
        FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
        acc16 = (res[0] << 6) | (res[1] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        acc_value[counter][0] = ((float)acc16) / 4096.0f;
        acc16 = (res[2] << 6) | (res[3] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        acc_value[counter][1] = ((float)acc16) / 4096.0f;
        acc16 = (res[4] << 6) | (res[5] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        acc_value[counter][2] = ((float)acc16) / 4096.0f;
        counter++;
        if (counter == 1000)
            counter = 0;
        wait(0.5);
    }
}

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}