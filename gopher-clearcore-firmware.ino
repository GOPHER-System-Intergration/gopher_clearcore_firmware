#include "drive.h"
#include <arduino-timer.h>

// Select the baud rate to match the target serial device
#define baudRate 115200

// Other variables
auto logger_timer = timer_create_default();
auto flush_timer = timer_create_default();
bool logger = false;

bool serial_flush(void *)
{
  Serial.flush();
  // Serial.begin(baudRate);
  return true;
}

void setup() 
{
    attachInterrupt(digitalPinToInterrupt(lowerEndstopPin), lowerEndstopInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(upperEndstopPin), upperEndstopInterrupt, FALLING);

    interrupts();

    // Put your setup code here, it will only run once:
    // Activate break
    pinMode(brakePin, OUTPUT);

    // Sets up serial communication and waits up to 5 seconds for a port to open.
    // Serial communication is not required for this example to run.
    Serial.begin(baudRate);
    uint32_t timeout = 5000;
    uint32_t startTime = millis();
    while (!Serial && millis() - startTime < timeout) 
    {
        continue;
    }

    driveSetup();

    // Clear the buffer each 1000ms
    flush_timer.every(1000, serial_flush);
}

void loop() 
{
  // Auto-enable brake after a movement
  moveCompleted();

  flush_timer.tick();
  
  if(logger == true)
  {
    logger_timer.tick();
  }
  

  if(Serial.available() != 0)
  {
   
    String command = Serial.readStringUntil('_');

    // Blank command
    if(command == 0)
    {

    }

    // Homing command
    else if(command == "hm")
    {
      homing(positionUpperLimit);
    }

    // Absolute movement (mm)
    else if(command == "am")
    {
      double position = Serial.readStringUntil('_').toDouble();
      double vel_frac = Serial.readStringUntil('_').toDouble();
      moveAbsolutePosition(position, vel_frac, false, false);
    }

    // Relative movement (mm)
    else if(command == "rm")
    {
      double position = Serial.readStringUntil('_').toDouble();
      double vel_frac = Serial.readStringUntil('_').toDouble();
      moveRelativePosition(position, vel_frac, false, false);
    }

    // Velocity movement (fraction of the max velocity)
    else if(command == "vm")
    {
      double vel_frac = Serial.readStringUntil('_').toDouble();
      moveAtVelocity(vel_frac, false);     
    }

    // Enable the drive
    else if(command == "ed")
    {
      drivePower("on");
    }

    // Disable the drive
    else if(command == "dd")
    {
      drivePower("off");
    }

    // Enable the brake
    else if(command == "eb")
    {
      brakeControl("on", true);
    }
   
    // Disable the brake
    else if(command == "db")
    {
      brakeControl("off", true);
    }

    // Debug mode
    else if(command == "debug")
    {
      String sub_command = Serial.readStringUntil('_');

      if(sub_command == "on") debugMode(true);
      else if(sub_command == "off") debugMode(false);
    }

    // Get position (mm)
    else if(command == "gp")
    {
      Serial.print("Current position is ");
      Serial.print(getPosition());
      Serial.println(" mm");
    }

    // Get velocity (mm/s)
    else if(command == "gv")
    {
      Serial.print("Current velocity is ");
      Serial.print(getVelocity());
      Serial.println(" mm/s");
    }

    else if(command == "logger")
    {
      String sub_command = Serial.readStringUntil('_');

      if(sub_command == "on")
      {
        logger = true;
  
        // Period in ms
        double period = Serial.readStringUntil('_').toDouble();
        logger_timer.cancel();
        logger_timer.every(period, driveStatus);
      }

      else if(sub_command == "off")
      {
        logger = false;
      }
    }
  }    
}
