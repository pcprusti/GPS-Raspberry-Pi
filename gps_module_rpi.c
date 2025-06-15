// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

char latitude[15] = {'\0'};
char lat_dir[2] = {'\0'};
char longitude[15] = {'\0'};
char lon_dir[2] = {'\0'};

int main() 
{
  // Allocate memory for read buffer, set size according to your needs
  char read_buf [256];	
  // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  int serial_port = open("/dev/ttyACM0", O_RDWR);

  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if(tcgetattr(serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  
  while(1)
  {
  //  set everything to 0 so we can
  memset(&read_buf, '\0', sizeof(read_buf));

  // Read bytes. The behaviour of read() (e.g. does it block?,
  int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
  //memcpy(read_buf, "$GPGLL,5109.0262317,N,11401.8407304,W,202725.00,A,D*79", sizeof("$GPGLL,5109.0262317,N,11401.8407304,W,202725.00,A,D*79"));
  //int num_bytes = sizeof(read_buf);
  // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
  if (num_bytes < 0) {
      printf("Error reading: %s", strerror(errno));
      return 1;
  }

  // Here we assume we received ASCII data
  //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
  int value = strncmp(read_buf, "$GPGLL", 6);
  if (!value)  
  {
        printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
        char* token = strtok(read_buf, ",");
        char i =0;
        while (token != NULL) 
          {
             //printf(" %s\n", token);
             token = strtok(NULL, ",");
             i++;
             //printf("\n%d\n", i);
             int value_tok = strncmp(token, "V", 1);
             
             if (value_tok && (i < 5))
             {
		     if(i == 1)
		     {
		        memcpy(latitude, token, sizeof(token));
		        printf("\nlatitude %s\n", latitude);
		     }
		     
		     if(i == 2)
		     {
		        memcpy(lat_dir, token, sizeof(token));
		        printf("lat_dir %s\n", lat_dir);
		     }
		     
		     if(i == 3)
		     {
		        memcpy(longitude, token, sizeof(token));
		        printf("longitude %s\n", longitude);
		     }
		     
		     if(i == 4)
		     {
		        memcpy(lon_dir, token, sizeof(token));
		        printf("lon_dir %s\n", lon_dir);
		     }
             }
             else
             {
             	break;
             }
          }
    }
  }
  close(serial_port);
  return 0; // success
};
