//------------------------------------------------------------------
#include <string.h>
#include <TimerOne.h>

#define ONE_WIRE_BUS 8 // Data wire from temp sensor
#define RELAY_COMP_PIN 9 // Relay to control the compressor
#define RELAY_MISC_PIN  10 // Relay for something else
#define RELAY_DISP_PIN  11 // Relay for change machine
#define RELAY_LIGHT_PIN 12 // Raly for light in change machine


#define MAX_COMMAND_LENGTH  64

#define RETURN_VALS // Return-mode define. Controls return messages.

String inputString = "";        // String read in from serial
String lastString = "";         // Last complete string read in
boolean stringComplete = false; // Is the string finished?
boolean debugMode = false;      // Disable debug mode by default

void setup() 
{
  // Initialize the output pins:
  pinMode(RELAY_DISP_PIN, OUTPUT);
  pinMode(RELAY_LIGHT_PIN, OUTPUT);
  digitalWrite(RELAY_DISP_PIN, HIGH);
  digitalWrite(RELAY_LIGHT_PIN, HIGH);
  // Start up the temperature control:
  initTempControl();
  // Start up the serial at 9600 baud:
  Serial.begin(9600);
  // Show the prompt symbol:
  printDebug(">: ");
}

void loop() {
  // Read in any serial values:
  serialEvent();
  // If a full string is read in:
  if (stringComplete) {
    // Try to parse the command:
    parseCommand();
    // Reset the stringComplete flag:
    stringComplete = false;
  }
  // Run the temperature control routine:
  runTempControl();
}

/*
  serialEvent() checks for serial data and adds any new chars
   to inputString. If the stringComplete flag is true, it means
   a newline was found and the string is complete.
*/
void serialEvent() {
  while (Serial.available()) {
    // Read in the next byte (character) via serial:
    char inChar = (char)Serial.read(); 

    // If a newline is found:
    if (inChar == '\n' || inChar == '\0') {
      // Convert the string to lowercase:
      inputString.toLowerCase();
      // The string is fully-formed, set the stringComplete flag:
      stringComplete = true;
      // Set the lastString to the new complete string:
      lastString = inputString;
      // Clear the inputString:
      inputString = "";
      // Start the next terminal line:
      printDebug("\r\n>: ");
    } 
    // Otherwise, keep going:
    else if (!stringComplete){
      // Add the character to the inputString:
      inputString += inChar;
      printDebug((String)inChar);
    }
  }
}

/*
  parseCommand() tries to analyze the string that was read in
   from serial and match it up with the corresponding function
   call.
*/
void parseCommand() {
  // Grab the first keyword from the string:
  String keyword = getNextKeyword();
  // Find the case where the keyword matches the function:
  if (keyword.equals("dispense")) {
    keyword = getNextKeyword();
    if (keyword.equals("") || keyword.equals("dispense")) {
      dispense(1);
      printReturn(1); // Successful dispense
    }
    else if (keywordIsInt(keyword)) {
      dispense(keywordToInt(keyword));
      printReturn(1); // Successful dispense
    }
    else {
      invalidKeyword(keyword);
      printReturn(0); // Unsuccessful dispense
    }
  }
  else if (keyword.equals("light")) {
    keyword = getNextKeyword();
    if (keyword.equals("on")) {
      turnOnLight();
      printReturn(1); // Successful light on
    }    
    else if (keyword.equals("off")) {
      turnOffLight();
      printReturn(1); // Successful light off
    }
    else if (keyword.equals("blink")) {
      keyword = getNextKeyword();
      if (keyword.equals("") || keyword.equals("blink")) {
        blinkLightNumberOfTimes();
        printReturn(1); // Successful blink
      }
      else if (keywordIsInt(keyword)) {
        blinkLightNumberOfTimes(keywordToInt(keyword));
        printReturn(1); // Successful blink
      }
      else {
        invalidKeyword(keyword);
        printReturn(0); // Unsuccessful blink
      }
    }
    else if (keyword.equals("") || keyword.equals("light")) {
      printDebug("Not enough arguments; try one of these:/r/n");
      printDebug("  light on/r/n");
      printDebug("  light off/r/n");
      printDebug("  light blink/r/n");
      printDebug("  light blink 2/r/n");
      printReturn(0); // Unsuccessful light command
    }
    else {
      invalidKeyword(keyword);
      printReturn(0); // Unsuccessful light command
    }
  }
  else if (keyword.equals("relay")) {
    keyword = getNextKeyword();
    if (keyword.equals("on")) {
      turnOnRelay();
      printReturn(1); // Successful relay on
    }    
    else if (keyword.equals("off")) {
      turnOffRelay();
      printReturn(1); // Successful relay off
    }
    else if (keyword.equals("") || keyword.equals("relay")) {
      printDebug("Not enough arguments; try one of these:/r/n");
      printDebug("  relay on/r/n");
      printDebug("  relay off/r/n");
      printReturn(0); // Unsuccessful relay command
    }
    else {
      invalidKeyword(keyword);
      printReturn(0); // Unsuccessful relay command
    }
  }
  else if (keyword.equals("get")) {
    keyword = getNextKeyword();
    if (keyword.equals("temp-center")) {
      printReturn(getTempCenter()); 
    }    
    else if (keyword.equals("temp-delta")) {
      printReturn(getTempDelta());
    }
    else if (keyword.equals("") || keyword.equals("get")) {
      printDebug("Not enough arguments; try one of these:/r/n");
      printDebug("  get temp-center/r/n");
      printDebug("  get temp-delta/r/n");
      printReturn(0); // Unsuccessful get command
    }
    else {
      invalidKeyword(keyword);
      printReturn(0); // Unsuccessful get command
    }
  }
  //------------
  else if (keyword.equals("set")) {
    keyword = getNextKeyword();
    if (keyword.equals("temp-center")) {
      keyword = getNextKeyword();
      if (keywordIsFloat(keyword)) {
        setTempCenter(keywordToFloat(keyword));
        printReturn(1); // Successful set temp-center
      }
      else if (keyword.equals("") || keyword.equals("temp-center")) {
        printDebug("Not enough arguments; try one of these:/r/n");
        printDebug("  set temp-center 2.0/r/n");
        printReturn(0); // Unsuccessful set temp-center
      } 
      else {
        invalidKeyword(keyword);
        printReturn(0); // Unsuccessful set temp-center
      }
    }    
    else if (keyword.equals("temp-delta")) {
      keyword = getNextKeyword();
      if (keywordIsFloat(keyword)) {
        setTempDelta(keywordToFloat(keyword));
        printReturn(1); // Successful set temp-delta
      }
      else if (keyword.equals("") || keyword.equals("temp-delta")) {
        printDebug("Not enough arguments; try one of these:/r/n");
        printDebug("  set temp-delta 0.5/r/n");
        printReturn(0); // Unsuccessful set temp-delta
      } 
      else {
        invalidKeyword(keyword);
        printReturn(0); // Unsuccessful set temp-delta
      }
    }    
    else if (keyword.equals("") || keyword.equals("set")) {
      printDebug("Not enough arguments; try one of these:/r/n");
      printDebug("  set temp-delta 0.5/r/n");
      printDebug("  set temp-center 2.0/r/n");
      printReturn(0); // Unsuccessful set command
    }
    else {
      invalidKeyword(keyword);
      printReturn(0); // Unsuccessful set command
    }
  }
  else if (keyword.equals("debug")) {
    keyword = getNextKeyword();
    if (keyword.equals("on")) {
      enableDebug();
      printReturn(1); // Successful debug on
    }    
    else if (keyword.equals("off")) {
      disableDebug();
      printReturn(1); // Successful debug off
    }
    else if (keyword.equals("") || keyword.equals("debug")) {
      printDebug("Not enough arguments; try one of these:/r/n");
      printDebug("  debug on/r/n");
      printDebug("  debug off/r/n");
      printReturn(0); // Unsuccessful debug command
    }
  }
  else if (keyword.equals("temperature") || keyword.equals("temp")) {
    printReturn(getTemperature()); // Return the current temperature 
  }
  else if (keyword.equals("test")) {
    printDebug("Test command successful.\r\n");
    printReturn(1); // Successful test command
  }
  else if (keyword.equals("")) {
    // Do nothing
  }
  else {
    invalidKeyword(keyword);
    printReturn(0); // Unsuccessful command
  } 
}

/*
  getNextKeyword() pulls out and returns the next keyword from 
   the input string. Keywords are seperated by spaces and may 
   contain any type of symbol.
*/
String getNextKeyword() {
  String keyword; // Placeholder string for the current keyword
  int keywordLength; // Length of the keyword
  
  // Length of the keywoard is the same as the index of the space:
  keywordLength = lastString.indexOf(' ');
  // If the string contains no spaces (just one keyword):
  if (keywordLength == -1) {
    // The keyword is the whole string:
    keyword = lastString;
  }
  // Otherwise the string contains one or more keywords:
  else {
    // Collect the first keyword:  
    keyword = lastString.substring(0, keywordLength);
    // Remove the keyword (and space) from the string:
    lastString = lastString.substring(keywordLength + 1);
  } 
  return keyword;
}

/*
  invalidKeyword() current displays an error message stating that
   the current keyword is not valid. Future improvements may allow
   the suggestion of valid keywords that are similar.
*/
void invalidKeyword(String keyword) {
  printDebug("Error: keyword '");
  printDebug(keyword);
  printDebug("' is invalid.\r\n");
}

/*
  keywordIsInt() returns 'true' if the keyword is an integer, and
   'false' if it is not.
*/
boolean keywordIsInt(String keyword) {
  bool retVal = false; // Return value
  // Convert the string to a char array:
  char cstring[keyword.length() + 1];
  keyword.toCharArray(cstring, sizeof(cstring));
  // Convert the char array to an int:
  int tempInt = atoi(cstring);
  // If the conversion results in '0' and the string is not "0":
  if (tempInt == 0 && !keyword.equals("0")) {
    // The conversion failed, not an int:
    retVal = false; 
  }
  else {
    // Otherwise, the conversion was a success:
    retVal = true;
  }
  return retVal;
}

/*
  keywordToInt() returns the converted integer value of the
   keyword. keyword should be checked with keywordIsInt() first;
   keywordToInt() will return '0' if keyword conversion fails. 
*/
int keywordToInt(String keyword) {
  // Convert the string to a char array:
  char cstring[keyword.length() + 1];
  keyword.toCharArray(cstring, sizeof(cstring));
  // Convert the char array to an int:
  return atoi(cstring);
}

/*
  keywordIsFloat() returns 'true' if the keyword is a float, and
   'false' if it is not.
*/
boolean keywordIsFloat(String keyword) {
  if (keyword.length() == 0)
  {
    return false;
  }
  // Convert the string to a char array:
  char cstring[keyword.length() + 1];
  keyword.toCharArray(cstring, sizeof(cstring));
  
  // A float can only have one decimal point:
  bool hasDecimalPoint = false;
  // Walk through the word, looking for digits and decimal points:
  for(int i=0; i<keyword.length(); ++i)
  {
    if(cstring[i] == '.')
    {
      if(hasDecimalPoint) 
        return false; // We already had a decimal point
      else
        hasDecimalPoint = true; // We now have a decimal point
    }
    else if(cstring[i] < '0' || cstring[i] > '9')
    {
      // If the character is anything other than 0-9, not a float:
      return false;
    }
  }
  return true;
}

/*
  keywordToFloat() returns the converted float value of the
   keyword. keyword should be checked with keywordIsFloat() first;
   keywordToFloat() will return '0' if keyword conversion fails. 
*/
float keywordToFloat(String keyword) {
  // Convert the string to a char array:
  char cstring[keyword.length() + 1];
  keyword.toCharArray(cstring, sizeof(cstring));
  // Convert the char array to a float:
  return atof(cstring);
}

/*
  printDebug() will print a debug message to Serial if debug-mode is
   enabled (#define DEBUG). Any information that is not used by
   the host program should be printed with this command.
*/
void printDebug(String message) {
  if (debugMode)
  {
    // Thie will only be printed if debug-mode is enabled:
    Serial.print(message);
  }
}
void printDebug(int message) {
  if (debugMode)
  {
    Serial.print(message);
  }
}
void printDebug(float message) {
  if (debugMode)
  {
    Serial.print(message);
  }
}


/*
  printReturn() will print a value indicating the output of a
   command. This is usually a Boolean output of '0' or '1' for most
   commands that just need to indicate success. These messages may
   be suppressed by commenting out the '#define RETURN_VALS' line
   at the top of this file. 
*/
void printReturn(int value) {
  #ifdef RETURN_VALS
    Serial.println(value);
  #endif
}
void printReturn(float value) {
  #ifdef RETURN_VALS
    Serial.println(value);
  #endif
}
