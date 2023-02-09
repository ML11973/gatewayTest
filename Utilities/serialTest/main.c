#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <sys/signal.h>     // Interrupt

/*
 * UART port for communication with radio module is UART4
 * TX successful, RX TBD
 *
 * adapted from https://raspberry-projects.com/pi/programming-in-c/uart-serial-port/using-the-uart
 */


int uart4_filestream = -1;
unsigned char rx_buffer[256];
int rx_length = 0;

void uart_rx_int(int signum){
    rx_length = read(uart4_filestream, (void*)rx_buffer, 255);
    return;
}

int main(int argc, char *argv[]){

    printf("Serial test utility\n");
    //----- SETUP USART 4 -----
    //At bootup, relevant pins are set by dtb
    //OPEN THE UART
    //The flags (defined in fcntl.h):
    //	Access modes (use 1 of these):
    //		O_RDONLY - Open for reading only.
    //		O_RDWR - Open for reading and writing.
    //		O_WRONLY - Open for writing only.
    //
    //	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode.
    //  When set read requests on the file can return immediately with a failure
    //  status if there is no input immediately available (instead of blocking).
    //  Likewise, write requests can also return immediately with a failure status
    //  if the output can't be written immediately.
    //
    //	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
    uart4_filestream = open("/dev/ttyAMA1", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
    if (uart4_filestream == -1)
    {
        //ERROR - CAN'T OPEN SERIAL PORT
        printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }

    // Flag description at :
    // https://www.mkssoftware.com/docs/man3/fcntl.3.asp
    // Various interrupt settings
    fcntl(uart4_filestream, F_SETFL, O_NDELAY);
    // Set running process as target of SIGIO signal interrupt
    fcntl(uart4_filestream, F_SETOWN, getpid());
    fcntl(uart4_filestream, F_SETFL,  O_ASYNC );

    //CONFIGURE THE UART
    //The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
    //	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
    //	CSIZE:- CS5, CS6, CS7, CS8
    //	CLOCAL - Ignore modem status lines
    //	CREAD - Enable receiver
    //	IGNPAR = Ignore characters with parity errors
    //	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
    //	PARENB - Parity enable
    //	PARODD - Odd parity (else even)
    struct termios options;
    tcgetattr(uart4_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD | ICRNL;		//<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart4_filestream, TCIFLUSH);
    tcsetattr(uart4_filestream, TCSANOW, &options);

    // SET INTERRUPT
    struct sigaction saio;
    saio.sa_handler = uart_rx_int;
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    sigaction(SIGIO,&saio,NULL);



    while(1){
        //----- CHECK FOR ANY RX BYTES -----
        if (uart4_filestream != -1)
        {
            // Read up to 255 characters from the port if they are there
            //unsigned char rx_buffer[256];
            //int rx_length = read(uart4_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
            if (rx_length < 0)
            {
                //An error occured (will occur if there are no bytes)
            }
            else if (rx_length == 0)
            {
                //No data waiting
            }
            else
            {
                //Bytes received
                printf("%i bytes read : %s\n", rx_length, rx_buffer);
                for(int i=0;i<rx_length;i++){
                    if(rx_buffer[i]<='Z' && rx_buffer[i]>='A'){
                        // Convert to lowercase
                        rx_buffer[i] += ('a'-'A');
                    }else if (rx_buffer[i]<='z' && rx_buffer[i]>='a'){
                        rx_buffer[i] -= ('a'-'A');
                    }
                }
                int count = write(uart4_filestream, rx_buffer, rx_length);
                rx_length=0;//Filestream, bytes to write, number of bytes to write
            }
        }


    }
    //----- CLOSE THE UART -----
    close(uart4_filestream);

    return 0;
}
