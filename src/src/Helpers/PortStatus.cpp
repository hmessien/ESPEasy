#include "PortStatus.h"

#include "../DataStructs/PinMode.h"
#include "../Globals/GlobalMapPortStatus.h"

#include "../../ESPEasy-Globals.h"


/**********************************************************
*                                                         *
* Helper Functions for managing the status data structure *
*                                                         *
**********************************************************/
void savePortStatus(uint32_t key, struct portStatusStruct& tempStatus) {
  // FIXME TD-er: task and monitor are unsigned, should we only check for == ????
  if ((tempStatus.task <= 0) && (tempStatus.monitor <= 0) && (tempStatus.command <= 0)) {
    globalMapPortStatus.erase(key);
  }
  else {
    globalMapPortStatus[key] = tempStatus;
  }
}

bool existPortStatus(uint32_t key) {
  return globalMapPortStatus.find(key) != globalMapPortStatus.end();
}

void removeTaskFromPort(uint32_t key) {
  const auto it = globalMapPortStatus.find(key);
  if (it != globalMapPortStatus.end()) {
    (it->second.task > 0) ? it->second.task-- : it->second.task = 0;

    if ((it->second.task <= 0) && (it->second.monitor <= 0) && (it->second.command <= 0) &&
        (it->second.init <= 0)) {
      // erase using the key, so the iterator can be const
      globalMapPortStatus.erase(key);
    }
  }
}

void removeMonitorFromPort(uint32_t key) {
  const auto it = globalMapPortStatus.find(key);
  if (it != globalMapPortStatus.end()) {
    it->second.monitor = 0;

    if ((it->second.task <= 0) && (it->second.monitor <= 0) && (it->second.command <= 0) &&
        (it->second.init <= 0)) {
      // erase using the key, so the iterator can be const
      globalMapPortStatus.erase(key);
    }
  }
}

void addMonitorToPort(uint32_t key) {
  globalMapPortStatus[key].monitor = 1;
}

uint32_t createKey(uint16_t pluginNumber, uint16_t portNumber) {
  return (uint32_t)pluginNumber << 16 | portNumber;
}

pluginID_t getPluginFromKey(uint32_t key) {
  return static_cast<pluginID_t>((key >> 16) & 0xFFFF);
}

uint16_t getPortFromKey(uint32_t key) {
  return static_cast<uint16_t>(key & 0xFFFF);
}




/*********************************************************************************************\
   set pin mode & state (info table)
\*********************************************************************************************/
/*
   void setPinState(byte plugin, byte index, byte mode, uint16_t value)
   {
   // plugin number and index form a unique key
   // first check if this pin is already known
   bool reUse = false;
   for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      pinStates[x].mode = mode;
      pinStates[x].value = value;
      reUse = true;
      break;
    }

   if (!reUse)
   {
    for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if (pinStates[x].plugin == 0)
      {
        pinStates[x].plugin = plugin;
        pinStates[x].index = index;
        pinStates[x].mode = mode;
        pinStates[x].value = value;
        break;
      }
   }
   }
 */

/*********************************************************************************************\
   get pin mode & state (info table)
\*********************************************************************************************/

/*
   bool getPinState(byte plugin, byte index, byte *mode, uint16_t *value)
   {
   for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
 * mode = pinStates[x].mode;
 * value = pinStates[x].value;
      return true;
    }
   return false;
   }

 */
/*********************************************************************************************\
   check if pin mode & state is known (info table)
\*********************************************************************************************/
/*
   bool hasPinState(byte plugin, byte index)
   {
   for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      return true;
    }
   return false;
   }

 */


/*********************************************************************************************\
   report pin mode & state (info table) using json
\*********************************************************************************************/
String getPinStateJSON(bool search, uint32_t key, const String& log, int16_t noSearchValue)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("getPinStateJSON"));
  #endif
  printToWebJSON = true;
  byte mode     = PIN_MODE_INPUT;
  int16_t value = noSearchValue;
  bool    found = false;

  if (search) {
    const auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      mode  = it->second.mode;
      value = it->second.getValue();
      found = true;
    }
  }

  if (!search || (search && found))
  {
    String reply;
    reply.reserve(128);
    reply += F("{\n\"log\": \"");
    {
      // truncate to 25 chars, max MQTT message size = 128 including header...
      int colonPos = log.indexOf(':');
      if (colonPos == -1) {
        colonPos = 0;
      }
      String tmp = log.substring(colonPos, colonPos + 25);
      tmp.trim();      
      reply += tmp;
    }
    reply += F("\",\n\"plugin\": ");
    reply += getPluginFromKey(key);
    reply += F(",\n\"pin\": ");
    reply += getPortFromKey(key);
    reply += F(",\n\"mode\": \"");
    reply += getPinModeString(mode);
    reply += F("\",\n\"state\": ");
    reply += value;
    reply += F("\n}\n");
    return reply;
  }
  return "";
}

const __FlashStringHelper * getPinModeString(byte mode) {
  switch (mode)
  {
    case PIN_MODE_UNDEFINED:    return F("undefined");
    case PIN_MODE_INPUT:        return F("input");
    case PIN_MODE_INPUT_PULLUP: return F("input pullup");
    case PIN_MODE_INPUT_PULLDOWN: return F("input pulldown");
    case PIN_MODE_OFFLINE:      return F("offline");
    case PIN_MODE_OUTPUT:       return F("output");
    case PIN_MODE_PWM:          return F("PWM");
    case PIN_MODE_SERVO:        return F("servo");
    default:
      break;
  }
  return F("ERROR: Not Defined");
}