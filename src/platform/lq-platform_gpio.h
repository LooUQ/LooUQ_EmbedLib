/** ***************************************************************************
  @file 
  @brief LTEm GPIO abstraction declarations.

  @author Greg Terrell/Jensen Miller, LooUQ Incorporated

  \loouq

  @warning Internal dependencies, changes only as directed by LooUQ staff.

-------------------------------------------------------------------------------

LooUQ-LTEmC // Software driver for the LooUQ LTEm series cellular modems.
Copyright (C) 2017-2023 LooUQ Incorporated

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
Also add information on how to contact you by electronic and paper mail.

**************************************************************************** */


#ifndef __PLATFORM_GPIO_H__
#define __PLATFORM_GPIO_H__

// #ifdef __cplusplus
// #include <cstdint>
// #else
// #endif // __cplusplus

#include <stdint.h>
#include <stdbool.h>

#ifndef LOW
#define LOW                     0x0                         ///< GPIO low/0 value
#define HIGH                    0x1                         ///< GPIO high/1 value
#endif

//GPIO FUNCTIONS
#ifndef INPUT
    #define INPUT               0x01                        ///< Digital GPIO pin behavior: input

    // Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT) 
    // where you can read the state of pin even when it is set as OUTPUT

    #define OUTPUT              0x03                        ///< Digital GPIO pin behavior: output 
    #define PULLUP              0x04                        ///< Digital GPIO pin behavior: output pullup
    #define INPUT_PULLUP        0x05                        ///< Digital GPIO pin behavior: input pullup
    #define PULLDOWN            0x08                        ///< Digital GPIO pin behavior: output pulldown
    #define INPUT_PULLDOWN      0x09                        ///< Digital GPIO pin behavior: input pulldown
    #define OPEN_DRAIN          0x10                        ///< Digital GPIO pin behavior: input open-drain
    #define OUTPUT_OPEN_DRAIN   0x13                        ///< Digital GPIO pin behavior: output open-drain
    #define ANALOG              0xC0                        ///< Digital GPIO pin behavior: analog input
#endif


//Interrupt Modes
#ifndef RISING
    #define DISABLED            0x00                        ///< Interrup pin event: none/disabled
    #define RISING              0x01                        ///< Interrup pin event: rising 0>1
    #define FALLING             0x02                        ///< Interrup pin event: falling 1>0
    #define CHANGE              0x03                        ///< Interrup pin event: any change
    #define ONLOW               0x04                        ///< Interrup pin event: on LOW
    #define ONHIGH              0x05                        ///< Interrup pin event: on HIGH
    #define ONLOW_WE            0x0C                        ///< Interrup pin event: on LOW (WE)
    #define ONHIGH_WE           0x0D                        ///< Interrup pin event: on HIGH (WE)
#endif


/**
 * @brief GPIO pin (I/O) values
 */
typedef enum {
    gpioValue_low = LOW,
    gpioValue_high = HIGH
} gpioPinValue_t;


/**
 * @brief GPIO pin modes
 */
typedef enum {
    gpioMode_input = INPUT,
    gpioMode_output = OUTPUT,
    gpioMode_inputPullUp = INPUT_PULLUP,
    gpioMode_inputPullDown = INPUT_PULLDOWN
} gpioPinMode_t;


/**
 * @brief IRQ trigger conditions
 */
typedef enum {
    gpioIrqTriggerOn_low = LOW,
    gpioIrqTriggerOn_high = HIGH,
    gpioIrqTriggerOn_change = CHANGE,
    gpioIrqTriggerOn_falling = FALLING,
    gpioIrqTriggerOn_rising = RISING
} gpioIrqTrigger_t;


/**
 * @brief Prototype IRQ callback (ISR)
 * 
 */
typedef void(*platformGpioPinIrqCallback)(void);


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * @brief 
 * 
 * @param pinNum 
 * @param pinMode 
 */
void lqGpio_openPin(uint8_t pinNum, uint8_t pinMode);

/**
 * @brief 
 * 
 * @param pinNum 
 */
void lqGpio_closePin(uint8_t pinNum);


/**
 * @brief 
 * 
 * @param pinNum 
 * @return uint8_t 
 */
uint8_t lqGpio_readPin(uint8_t pinNum);

/**
 * @brief 
 * 
 * @param pinNum 
 * @param val 
 */
void lqGpio_writePin(uint8_t pinNum, uint8_t val);

/**
 * @brief 
 * 
 * @param pinNum 
 * @param enabled 
 * @param triggerMode 
 * @param isrCallback 
 */
void lqGpio_attachIsr(uint8_t pinNum, bool enabled, uint8_t triggerMode, platformGpioPinIrqCallback isrCallback);

/**
 * @brief 
 * 
 * @param pinNum 
 */
void lqGpio_detachIsr(uint8_t pinNum);


/* The functions below are optional
 * They are intended to be used during development to help create your attach\detach ISR functions. */

/**
 * @brief 
 * 
 * @return uint32_t 
 */
uint32_t lqGpio_getIntFlags();

/**
 * @brief 
 * 
 * @param pin 
 * @return uint32_t 
 */
uint32_t lqGpio_getPinInterrupt(uint32_t pin);


/* DEPRECATED - To be removed in embedLib v2.1.0 
 ------------------------------------------------------- */
// void platform_openPin(uint8_t pinNum, uint8_t pinMode);
// void platform_closePin(uint8_t pinNum);

// uint8_t platform_readPin(uint8_t pinNum);
// void platform_writePin(uint8_t pinNum, uint8_t val);

// void platform_attachIsr(uint8_t pinNum, bool enabled, uint8_t triggerMode, platformGpioPinIrqCallback isrCallback);
// void platform_detachIsr(uint8_t pinNum);

// /* The functions below are optional
//  * They are intended to be used during development to help create your attach\detach ISR functions. */
// uint32_t platform_getIntFlags();
// uint32_t platform_getPinInterrupt(uint32_t pin);
/* --------------------------------------------------------
 */


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  /* !__PLATFORM_GPIO_H__ */