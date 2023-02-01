/**
 ****************************************************************************
 * @file    buffered_uart.c
 * @author  nicolas.brunner@heig-vd.ch
 * @date    07-August-2018
 * @brief   Driver for UART using circular buffer
 ****************************************************************************
 * @copyright HEIG-VD
 *
 * License information
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>			// Used for UART
#include <fcntl.h>			// Used for UART
#include <termios.h>		// Used for UART
#include <sys/signal.h>     // Interrupt
#include <sys/types.h>

#include "buffered_uart.h"
#include "hardware.h"

/* Private define ------------------------------------------------------------*/
#define RX_BUFFER_SIZE     512
#define TX_BUFFER_SIZE    1024
#define N_BAUD_SETTINGS   22


typedef struct{
  parity_t parity;
  uint32_t word_length;
  flowControl_t flow_control;
  stopBits_t stopBits;
  uint32_t baudrate;
}uartParam_s;

/* Private variables----------------------------------------------------------*/
uartParam_s uConfig;




/** Buffer for the RX */
static uint8_t rx_buffer[RX_BUFFER_SIZE];
 // Index up to which data in buffer was already read (can be overwritten)
static uint16_t rx_buffer_read_index;
 // Index up to which buffer is filled and unread (read_available)
static uint16_t rx_buffer_fill_count;
// Current write index
static uint16_t rx_buffer_write_index;

/** Buffer for the TX */
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint16_t tx_buffer_read_index;
static uint16_t tx_buffer_write_index;
static volatile uint16_t tx_buffer_fill_count;
static uint16_t tx_dma_size;


static int uart_fd;

/* Private functions declaration ---------------------------------------------*/

static size_t update_rx_buffer(void);
static void update_rx_buffer_fill_count(void);
static void start_tx_dma(void);
static size_t start_tx_linux(uint8_t * buffer, uint16_t size);
static void linux_uart_init(void); ///< Linux UART initialization
static void uart_clearRxBuffer(void);
static void tx_linux_complete(void);

/* Public functions implementation-----------------------------------------*/
void buffered_uart_init(uint32_t baudrate, uint32_t word_length, parity_t parity, flowControl_t flow_control){

  rx_buffer_read_index = 0;
  rx_buffer_fill_count = 0;
  rx_buffer_write_index = 0;
  tx_buffer_read_index = 0;
  tx_buffer_write_index = 0;
  tx_buffer_fill_count = 0;

  uConfig.parity = parity;
  uConfig.word_length = word_length;
  uConfig.flow_control = flow_control;
  uConfig.baudrate = baudrate;
  uConfig.stopBits = STOP_BITS_1;

  uart_fd = open(UART_DEV, O_RDWR | O_NOCTTY/* | O_NDELAY*/);
  if(uart_fd<0)
    printf("Can't open serial port\n");
/*
  // SET INTERRUPT
  struct sigaction saio;
  saio.sa_handler = newData;
  saio.sa_flags = 0;
  saio.sa_mask = 0;
  saio.sa_restorer = NULL;
  sigaction(SIGIO,&saio,NULL);

  // Flag description at :
  // https://www.mkssoftware.com/docs/man3/fcntl.3.asp
  // Various interrupt settings
  //fcntl(uart_fd, F_SETFL, O_NDELAY);
  // Set running process as target of SIGIO signal interrupt
  fcntl(uart_fd, F_SETOWN, getpid());
  fcntl(uart_fd, F_SETFL,  O_ASYNC );
  //fcntl(uart_fd, F_SETFL,  O_NONBLOCK );
*/
  linux_uart_init();


}


void buffered_uart_deinit(void)
{
  close(uart_fd);
  return;
}


void buffered_uart_start(void){
  //do nothing, uart is started on init
  return;
}


void buffered_uart_set_baudrate(uint32_t baudrate){
  uConfig.baudrate = baudrate;
  linux_uart_init();
  return;
}


static void update_rx_buffer_fill_count(void)
{
    // update rx buffer, which will update its fill count (simulate a DMA)
    update_rx_buffer();
    return;
}

uint16_t buffered_uart_read(uint8_t* data, uint16_t size)
{
    uint16_t effectiveSize; // how many bytes will be copied to argument data

    // calculate the effective read size by updating local rx buffer
    update_rx_buffer_fill_count();
    if (size < rx_buffer_fill_count) {
        effectiveSize = size;
    } else {
        effectiveSize = rx_buffer_fill_count;
    }
    // if data is available and we have room to copy them
    if (effectiveSize > 0) {
      // if unread data and new data are too big for local rx buffer
      // Circular write (two-fold)
        if (rx_buffer_read_index + effectiveSize > RX_BUFFER_SIZE) {
            uint16_t firstPartSize = RX_BUFFER_SIZE - rx_buffer_read_index;

            //memcpy(void * dest,const void * src,size_t size)
            // copy up to buffer length
            memcpy(data, &rx_buffer[rx_buffer_read_index], firstPartSize);
            // copy rest on buffer start
            memcpy(&data[firstPartSize], rx_buffer, effectiveSize - firstPartSize);
            rx_buffer_read_index = effectiveSize - firstPartSize;
        } else {
          // Standard copy if buffer is big enough
            memcpy(data, &rx_buffer[rx_buffer_read_index], effectiveSize);
            rx_buffer_read_index += effectiveSize;
            if (rx_buffer_read_index == RX_BUFFER_SIZE) rx_buffer_read_index = 0;
        }
        // Update data count in local buffer
        rx_buffer_fill_count -= effectiveSize;
    }
    return effectiveSize;
}

uint16_t buffered_uart_write(const uint8_t* data, uint16_t size)
{
  uint16_t effectiveSize;

  effectiveSize = TX_BUFFER_SIZE - tx_buffer_fill_count;
  // if there is enough room in TX buffer for all input bytes
  if (size < effectiveSize) effectiveSize = size;

  // if TX buffer is not full
  if (effectiveSize > 0) {
    // if TX buffer has to be looped back (circular write)
    if (tx_buffer_write_index + effectiveSize > TX_BUFFER_SIZE) {
      uint16_t firstPartSize = TX_BUFFER_SIZE - tx_buffer_write_index;
      // Copy up to last TX buffer index
      memcpy(&tx_buffer[tx_buffer_write_index], data, firstPartSize);
      // Copy on buffer start (loopback)
      memcpy(tx_buffer, &data[firstPartSize], effectiveSize - firstPartSize);
      tx_buffer_write_index = effectiveSize - firstPartSize;
    } else {
      // Standard copy
      memcpy(&tx_buffer[tx_buffer_write_index], data, effectiveSize);
      tx_buffer_write_index += effectiveSize;
      if (tx_buffer_write_index == TX_BUFFER_SIZE) tx_buffer_write_index = 0;
    }

    uint16_t fillCount;

    // Replaced with
    fillCount = tx_buffer_fill_count;
    tx_buffer_fill_count = fillCount + effectiveSize;
    if (fillCount == 0) start_tx_dma();
  }
  // This function simulates the TX complete callback on STM32
  // Without it, transmission will not take place
  tx_linux_complete();

  return effectiveSize;
}



uint16_t buffered_uart_get_read_available(void)
{
    update_rx_buffer_fill_count();
    return rx_buffer_fill_count;
}



uint16_t buffered_uart_get_write_available(void)
{
    return TX_BUFFER_SIZE - tx_buffer_fill_count;
}


// Probably a useless function, here for code compatibility. Arduino UART should
// never be busy since TX is blocking (fake DMA)
bool buffered_uart_is_busy(void)
{
    return (tx_buffer_fill_count > 0);
}



/* Private functions implementation ------------------------------------------*/

/* Updates RX buffer array contents to reflect hardware arduino DMA buffer
 * Implements a circular buffer with pointers:
 * rx_buffer_write_index -> current address to write new byte
 * rx_buffer_fill_count -> number of unread bytes (see buffered_uart_read)
 *
 */
static size_t update_rx_buffer(void){
  uint8_t rx_tmp[RX_BUFFER_SIZE] = {0};

  //printf("update_rx_buffer\n");
  ssize_t rx_length = read(uart_fd, rx_tmp, RX_BUFFER_SIZE);
  if(rx_length > 0){
    //printf("rx_length = %zd\n", rx_length);
    if (rx_buffer_write_index+rx_length > RX_BUFFER_SIZE){
      // Copy block in two parts if buffer not big enough
      uint16_t firstPartSize = RX_BUFFER_SIZE - rx_buffer_write_index;
      memcpy(&rx_buffer[rx_buffer_write_index], rx_tmp, firstPartSize);
      memcpy(rx_buffer, &rx_tmp[firstPartSize], rx_length-firstPartSize);
      rx_buffer_write_index=rx_length-firstPartSize;
    }else{
      // Copy block
      memcpy(&rx_buffer[rx_buffer_write_index], rx_tmp, rx_length);
      rx_buffer_write_index+=rx_length;
      // Wrap write index if out of buffer
      if (rx_buffer_write_index==RX_BUFFER_SIZE)
        rx_buffer_write_index=0;
    }
    rx_buffer_fill_count+=rx_length;
    //printf("rx_buffer_fill_count = %u\n", rx_buffer_fill_count);
  }
  // return nb of written bytes
  return (size_t) rx_buffer_fill_count;
}



/* Private functions implementation ------------------------------------------*/

static void start_tx_dma(void)
{
    tx_dma_size = tx_buffer_fill_count;
    if (tx_buffer_read_index + tx_dma_size > TX_BUFFER_SIZE) {
        tx_dma_size = TX_BUFFER_SIZE - tx_buffer_read_index;
    }

    start_tx_linux(&tx_buffer[tx_buffer_read_index], tx_dma_size);
}


/* TX using Arduino library
 * If size is over 64 (ARDUINO_BUFFER_SIZE, same as TX buffer size), function
 * is blocking. Else it proceeds as with a DMA.
 * Waits for control signal to be ok before transmitting
 */
static size_t start_tx_linux(uint8_t * buffer, uint16_t size){
  return write(uart_fd, buffer, size);
}


/* Simulates HAL_UART_TxCpltCallback
 * Originally called when DMA UART is done transmitting
 * Checks for untransmitted bytes with tx_buffer_fill_count and sends them
 */
static void tx_linux_complete(void)
{
    uint16_t fillCount;
    fillCount = tx_buffer_fill_count;
    tx_buffer_fill_count=fillCount-tx_dma_size;

    tx_buffer_read_index = (tx_buffer_read_index + tx_dma_size) % TX_BUFFER_SIZE;

    if (tx_buffer_fill_count > 0) start_tx_dma();
}


// Normally OK
static void linux_uart_init(void){
    // Useful resource for config
    //https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/#cstopb-num-stop-bits
    tcflag_t config = 0;

    struct termios options;
    tcgetattr(uart_fd, &options);


    // Flow control
    if (uConfig.flow_control == FLOW_CONTROL_RTS_CTS){
        // Enable hardware flow control
        #ifdef CRTSCTS
        config |= CRTSCTS;
        #endif
    }

    // Word length selection
    switch (uConfig.word_length){
        case 5:
            config |= CS5;
            break;
        case 6:
            config |= CS6;
            break;
        case 7:
            config |= CS7;
            break;
        default:
            config |= CS8;
            break;
    }

    // Baud rate selection
    uint32_t available_bauds[N_BAUD_SETTINGS] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000, 2500000, 3000000, 3500000, 4000000};
    uint32_t available_bauds_defs[N_BAUD_SETTINGS] = {B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000};
    // Choose closest inferior baudrate
    uint8_t i=0;
    while(uConfig.baudrate>available_bauds[i]){
        i++;
        if(i>N_BAUD_SETTINGS){
            i=3; // default baud 9600
            break;
        }
    }

    config |= available_bauds_defs[i];

    // Default parity is off, if parity is on default is even
    if(uConfig.parity != PARITY_NONE)
        config |= PARENB;

    if (uConfig.parity == PARITY_ODD)
        config |= PARODD;

    // Stop bits
    if (uConfig.stopBits == STOP_BITS_2)
        config |= CSTOPB;

    // Apply config
    options.c_cflag = config | CREAD | CLOCAL; // CREAD enables receiver, CLOCAL ignores RS-232 CD line
    // Disable special handling of RX bytes (raw data)
    options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    options.c_iflag |= IGNPAR; // ignore chars with parity errors
    if (uConfig.flow_control == FLOW_CONTROL_NONE)
        options.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

    // Disable all output options
    options.c_oflag &= ~(OPOST | ONLCR);

    // Local config as per ref link on top of function
    options.c_lflag &= ~(ICANON | ECHOE | ECHO | ECHONL | ISIG);

    // Set up non-blocking read
    options.c_cc[VTIME] = 0; // timeout = 0
    options.c_cc[VMIN]  = 0; // min. number of chars = 0

    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &options);

  return;
}


// Normally OK
static void uart_clearRxBuffer(void){
    tcflush(uart_fd, TCIFLUSH);
  return;
}
