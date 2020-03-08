/*
    ChibiOS - Copyright (C) 2006..2016 Nicolas Reinecke
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.hpp"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"
#include "QuadEncoder.hpp"
#include "shell.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
/**
 * PWM Config
 */
// We will be using PB6 for TIM4_CH1
static PWMConfig pwmCFG =
    {
        100000,               /* 10kHz PWM clock frequency */
        10,                   /* PWM period = ticks / PWM clock frequency =1second*/
        // Frequency = 150000/10;
        NULL,                 /* No callback */
        {
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL}
        },
        0,
        0
};

// The interrupts will be too much at about 1000 RPM there will be no time 
// for the processor to do other things.
static QEIConfig qeicfg = {
    QEI_MODE_QUADRATURE,
    QEI_BOTH_EDGES,
    QEI_DIRINV_FALSE,
    QEI_OVERFLOW_WRAP,
    0,
    8192,
    NULL,
    NULL,
  };

// Initialize the Quadrature Encoder
QuadEncoder quadEncoder(&QEID3, &qeicfg);

// Base sequential stream for printing on terminal
BaseSequentialStream *chp = (BaseSequentialStream *)(&SDU1);

// Constants
static constexpr double ONE_BY_TWO_PI =  0.15915494309;

// Variables
bool stopCommandIssued = false;

/**
 * Handles the interrupt. Just a hack to satisfy chibios as it needs
 * a static function for handling interrupt
 */
void handleInterrupt(void *arg)
{
    (void)arg;
    quadEncoder.handlePinAInterrupt();
}

/** 
 * Prints the angle until a stop command is issued.
 */
static void print_angle(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argv;
    // Reset the encoder first
    quadEncoder.reset();
    double currentAngle;
    char* dirString;
    QuadEncoder::Direction direction;
    stopCommandIssued = false;
    while(!stopCommandIssued)
    {
        direction = quadEncoder.getCurrentDirection();
        if(direction == QuadEncoder::Direction::FORWARD)
        {
            dirString = "Forward";
        }
        else if(direction == QuadEncoder::Direction::REVERSE)
        {
            dirString = "Reverse";
        }
        // Calculate angular velocity
        currentAngle = quadEncoder.getCurrentAngleRad();
        chprintf(chp, "Angle : %f, Direction : %s \n", currentAngle, dirString);
        chThdSleepMilliseconds(50);
    }
    chThdExit(0);
}

/**
 * The experiment function.
 * This will set the PWM to 50% of the duty cycle. The maximum voltage of 
 * 6V has been set in the laboratory power supply and with 50% duty cycle
 * it will be about ~3V and the direction is switched about every 250ms as 
 * it has been done in the matlab simulation.
 */
static void start_experiment(BaseSequentialStream *chp, int argc, char* argv[])
{
    (void) argv;
    // last angle
    double lastAngle = 0.;
    double currentAngle = 0.;
    double angularVelocity = 0.;
    double time = 0.;
    QuadEncoder::Direction direction;
    char* dirString;
    uint16_t qei;
    uint16_t directionTime = 0;
    /*
    * Enable channel 0 with 50% duty cycle
    */
    pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 2000));
    // Also create thread for direction.
    // Run for only 10 seconds
    // Reset the encoder first
    quadEncoder.reset();
    while(time < 5.)
    {
        qei = quadEncoder.getPulseCount();
        direction = quadEncoder.getCurrentDirection();
        if(direction == QuadEncoder::Direction::FORWARD)
        {
            dirString = "Forward";
        }
        else if(direction == QuadEncoder::Direction::REVERSE)
        {
            dirString = "Reverse";
        }
        // Calculate angular velocity
        currentAngle = quadEncoder.getCurrentAngleRad();
        angularVelocity = (ONE_BY_TWO_PI * (currentAngle - lastAngle)) / 0.5;
        chprintf(chp, "%f, %f\n", time, quadEncoder.getCurrentAngleRad());
        time += 0.005;
        lastAngle = currentAngle;
        if(directionTime == 250)
        {
            palTogglePad(GPIOA, 5);
            directionTime = 0;
        }
        directionTime += 5;
        chThdSleepMilliseconds(5);
    }
    // Reset the encoder before exiting
    quadEncoder.reset();
    pwmDisableChannel(&PWMD4, 0);
    chThdExit(0);
}

static void start_validation(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argv;
    // Start validation
    // last angle
    double currentAngle = 0.;
    double time = 0.;
    uint16_t totalTime = 0;
    // Also create thread for direction.
    // Run for only 10 seconds
    // Reset the encoder first
    quadEncoder.reset();
    while(totalTime < 5000)
    {
        // Calculate angular velocity
        currentAngle = quadEncoder.getCurrentAngleRad();
        chprintf(chp, "%f, %f\n", time, quadEncoder.getCurrentAngleRad());
        totalTime += 5;
        time += 0.005;
        if(totalTime == 750)
        {
            pwmEnableChannel(&PWMD4, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, 2000));
        }
        if(totalTime == 1000)
        {
            palTogglePad(GPIOA, 5);
        }
        if(totalTime == 1500)
        {
            palTogglePad(GPIOA, 5);
        }
        if(totalTime == 1750)
        {
            pwmDisableChannel(&PWMD4, 0);
        }
        chThdSleepMilliseconds(5);
    }
    // Reset the encoder before exiting
    quadEncoder.reset();
    chThdExit(0);
}

// Shell commands
static const ShellCommand commands[] = {
    {"start", start_experiment},
    {"print", print_angle},
    {"valid", start_validation},
    {NULL, NULL}
};

// Shell config
static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SDU1,
    commands
};


/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chibios_rt::System::init();

  // Initialize Serial-USB driver
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  quadEncoder.init();
  // Initialize Quadrature Encoder
  quadEncoder.setGpioAParams(GPIOA, 6);
  quadEncoder.setGpioBParams(GPIOB, 7);

  // Set the alternate function for PA6, as it uses TIM3_CH1
  palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_PULLUP);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_PULLUP);
  palEnableLineEvent(PAL_LINE(GPIOA, 6U), PAL_EVENT_MODE_RISING_EDGE);
  palEnableLineEvent(PAL_LINE(GPIOA, 7U), PAL_EVENT_MODE_BOTH_EDGES);
  palSetPadCallback(GPIOA, 6, handleInterrupt, NULL);

  // Set alternate function for PB6 for PWM outputs
  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
  pwmStart(&PWMD4, &pwmCFG);

  // Set mode for dir pin
  palSetPadMode(GPIOA, 5, PAL_MODE_OUTPUT_PUSHPULL);

  // Initialize shell
  shellInit();
  AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_NOREMAP;
  
//   chThdCreateStatic(changeDirection, sizeof(changeDirection),
//                     NORMALPRIO+1, dirThread, NULL);
  while (1) {
      if(serusbcfg.usbp->state == USB_ACTIVE)
      {
          thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                              "shell", NORMALPRIO + 1,
                                              shellThread, (void *)&shell_cfg1);
          chThdWait(shelltp); 
      }
     chThdSleepMilliseconds(1000);
  }
}