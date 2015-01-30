#ifndef __FONEASTRA_PINS_H__
#define __FONEASTRA_PINS_H__

/*
this file maps pins on the FoneAstra board to an AtMega328 based Arduino (Uno, mini, pro mini)
Arduino-ATMega pin mapping available at:
http://www.arduino.cc/en/Hacking/PinMapping168
*/

//SS1_PIN

//BUZZER_PIN: an audio buzzer is connected to this pin. 

//BT_PWR_PIN: this controls power to the BT module. driving this high turns the module on, low is off.

//A3_PIN

//I2C SDA, SCL pins already defined in Contents/Resources/Java/hardware/arduino/variants/standard/pins_arduino.h 

//BATT_LVL_PIN: this is a voltage divider and used for an approximate measure of battery charge

//A7_PIN

//INT0_PIN: goes high when a BT device connects (blue LED goes on), low otherwise. 

//INT1_PIN: connected to a tactile switch. Goes high when the switch is pressed.

//BT_RESET_PIN: this pin can reset the BT module.  

//D5_PIN

//ONEWIRE_PIN: 1-wire sensors are connected to this pin. The temperature sensor used in HMB & VAX apps is a 1wb sensor.  

//WALL_PIN: high when a usb charger is connected to the board, low otherwise. 
//
//the BT module is connected to ATMega's h/w UART, so for apps that use the BT module, we have a software UART 
//for the serial console. foneastra's UARTs are accessible on a 2x6 male header. The pins on the inner row are the
//HW UART & used for downloading code on the board. The pins on the outer row are the SW UART & can be used with 
//the serial console if HW UART is being used for BT communications
//
//SW_UART_RX: the rx pin of the s/w uart 

//SW_UART_TX: the tx pin of the h/w uart. 

//SPI SS, MOSI, MISO, SCK pns already defined in pins_arduino.h

#define SS1_PIN A0
#define BUZZER_PIN A1
#define BT_PWR_PIN A2
#define A3_PIN A3

//I2C pins. Already defined in Contents/Resources/Java/hardware/arduino/variants/standard/pins_arduino.h
//#define SDA A4
//#define SCL A5

#define BATT_LVL_PIN A6
#define A7_PIN A7

#define HW_UART_RX_PIN 0
#define HW_UART_TX_PIN 1
#define INT0_PIN 2
#define INT1_PIN 3
#define BT_RESET_PIN 4
#define D5_PIN 5
#define ONEWIRE_PIN 6
#define WALL_PIN 7
#define SW_UART_RX_PIN 8
#define SW_UART_TX_PIN 9

#define RED_LED_PIN D5_PIN
#define GREEN_LED_PIN SCK

//SPI pins. Already defined in Contents/Resources/Java/hardware/arduino/variants/standard/pins_arduino.h
//#define SS 10
//#define MOSI 11
//#define MISO 12
//#define SCK 13

#endif
