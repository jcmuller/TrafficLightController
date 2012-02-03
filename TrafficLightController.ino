/********************************************************************************
 * Copyright (c) 2012 Juan C. Muller <jcmuller@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 ********************************************************************************/

#include "WiServer.h"
#include "Streaming.h"

#include "wireless_connection.h"

enum Lights {
  OFF,
  RED,
  YELLOW,
  GREEN,
  NUMBER_OF_LIGHTS
};

int lightPins[NUMBER_OF_LIGHTS];
int activeLight = 0;
long turnedOnAtMillis = 0;
long turnOffAfter = 60000 * 15; // 15 minutes
long currentMillis;
boolean debug = true;
boolean turnGreenOff = false;

boolean processRequest(char* URL)
{
  boolean found = false;

  if (strcmp(URL, "/off") == 0)
  {
    WiServer.print("<html>off</html>\n");
    activeLight = OFF;
    found = true;
  }
  else if (strcmp(URL, "/red") == 0)
  {
    WiServer.print("<html>red</html>\n");
    activeLight = RED;
    found = true;
  }
  else if (strcmp(URL, "/yellow") == 0)
  {
    WiServer.print("<html>yellow</html>\n");
    activeLight = YELLOW;
    found = true;
  }
  else if (strcmp(URL, "/green") == 0)
  {
    WiServer.print("<html>green</html>\n");
    activeLight = GREEN;
    turnedOnAtMillis = millis();
    found = true;
  }
  else if (strcmp(URL, "/timeLeft") == 0)
  {
    WiServer.print("<html>");

    currentMillis = millis();
    int timeRunning = currentMillis - turnedOnAtMillis;

    if (timeRunning < turnOffAfter)
      WiServer << "green will turn off in " << ((turnOffAfter - timeRunning) / 60000) << " minutes";
    else
      WiServer.print("green light turned off already");

    WiServer.print("</html>");
    return true;
  }
  else if (
      (strcmp(URL, "/status") == 0) ||
      (strcmp(URL, "/active") == 0)
      )
  {
    WiServer.print("<html>");

    switch(activeLight)
    {
      case RED:
        WiServer.print("red");
        break;
      case GREEN:
        WiServer.print("green");
        break;
      case YELLOW:
        WiServer.print("yellow");
        break;
      case OFF:
        WiServer.print("off");
        break;
    }

    WiServer.print("</html>");
    return true;
  }
  else if (strcmp(URL, "/turnGreenOff") == 0)
  {
    turnGreenOff = !turnGreenOff;
    WiServer.print("<html>");

    if (turnGreenOff)
      WiServer << "green will turn off after " << (turnOffAfter / 60000) << " minutes";
    else
      WiServer.print("green stays on");

    WiServer.print("</html>");
    return true;
  }
  else
  {
    WiServer.print("<html>Invalid</html>");
  }

  if (found)
  {
    for (int i = 0; i <= NUMBER_OF_LIGHTS; i++)
    {
      if (i != OFF)
      {
        digitalWrite(lightPins[i], LOW);
      }
    }

    if (debug)
      Serial.println("Turning all off");

    if (activeLight != OFF)
    {
      digitalWrite(lightPins[activeLight], HIGH);

      if (debug)
        Serial << "Turning " << activeLight << " on";
    }
  }

  return found;
}

void setup()
{
  lightPins[RED]    = 5;
  lightPins[YELLOW] = 6;
  lightPins[GREEN]  = 7;

  for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
    if (i != OFF)
      pinMode(lightPins[i], OUTPUT);

  if (debug)
    Serial.begin(9600);

  WiServer.init(processRequest);
  //WiServer.enableVerboseMode(true);

  digitalWrite(lightPins[RED], HIGH);
}

void loop()
{
  WiServer.server_task();

  // Turn off green light after turnOffAfter milliseconds passed.
  if (activeLight == GREEN && turnGreenOff)
  {
    currentMillis = millis();

    if (currentMillis - turnedOnAtMillis > turnOffAfter)
    {
      activeLight = OFF;
      digitalWrite(lightPins[GREEN], LOW);

      if (debug)
        Serial.println("Turning green off");
    }
  }

  delay(10);
}
