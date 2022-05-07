# SIKTEC-MDSwitch
A lightweight Library to easily use Multi Direction Switches.

<br/>

## Description
Interrupt based triggering - all ISR logic is cross platform (AVR, ESP) and done in the background correctly. (without debouncing problems) with easy to use callback attachments. This way your code will be cleaner and safer.
`MDSwitch`h also supports MODES - Meaning you can set several modes which can be switched and has different callbacks attached, perfect workflow for menu based systems and for switching the device functionality based on the required state / mode.

<br />

### **Physically tested with:**

| BORAD    | Pins (int, push, ccw, cw)              | Info                         | 
|:--------:|:---------------------------------------|:-----------------------------|
| ESP32    | 35, 24, 36, 39                         | INPUT only pins              |
| UNO      | 3, 4, 5, 6                             | Only 2,3 support interrupts  |
| NANO     | 3, 4, 5, 6                             | Only 2,3 support interrupts  |
| MEGA     | 3, 4, 5, 6                             |                              |
| LEONARDO | 3, 4, 5, 6                             |                              |
| DUE      | 3, 4, 5, 6                             | *3.3v logic                  |

<br/>

<a id="table-contents"></a>

## Table of Contents:
- [Library Features](#features)
- [Quick Installation](#installation)
- [Example Included](#examples)
- [Decalring MDSwitch](#declaring)
- [All the key/buttons/terminals codes](#all-the-key-codes)
- [MDSwitch Initialization and MODES](#initialization)
- [Assigning an Event callback](#assigning-a-callback)
- [Auto invoking callbacks `tick()` method](#ticking)
- [MDSwitch additional Methods](#more-methods)
- [Important Notes](#important-notes)

<br/>

<a id="features"></a>

## Library Features 

<hr />

[Return](#table-contents)

- Interrupt based - Fully managed by the library - single interrupt.
- Callbacks don't requires ISR decleration volatile or whatever - Just simple function pointers.
- Smooth key/button captures - Debouncing and 'noise' is taking care of by the library.
- Dynamic Enable / Disable states.
- Modes - You can modes which stores Event -> Callbacks scheme to easily change the functionality.
- Small, Lightweight and very fast.
- Well documented code.

<br/>

<a id="installation"></a>

## Installation:

<hr />

[Return](#table-contents)

You can install the library through one of the following:
1. Arduino or PlatformIO library manager: Search for "SIKTEC-MDSwitch" and click install.
2. Download the repositories as a ZIP file and install it through the Arduino IDE by:<br/>
   `Sketch -> Include library -> Add .ZIP Library.`
3. Download the library and include it in your project folder - Then you can Include it directly:<br/>
    `#include "{path to}\SIKTEC_MDSwitch.h"`

<br/>

<a id="examples"></a>

## Example included:

<hr />

[Return](#table-contents)

The example included with this library demonstrates how to declare and initialize the library, set modes, assign callbacks and alter between the modes based on the captured events.
The code is well documented and should be easy to build upon and extend the functionality.
The example will create 3 modes:
- MODE: 0<br/>
`CCW :: will increment a counter and print to serial output.`<br />
`CW :: will decrement a counter and print to serial output.`<br />
`PUSH :: will change the mode (incrementing) and print a message to serial output.`<br />
`ANY :: will print the current counter to serial output.`
- MODE: 1<br/>
`CCW :: wont do anything (no callback attached).`<br />
`CW :: wont do anything (no callback attached).`<br />
`PUSH :: will change the mode (incrementing) and print a message to serial output.`<br />
`ANY :: will print the current counter to serial output.`
- MODE: 2 <br/>
`CCW :: wont do anything (no callback attached).`<br />
`CW :: wont do anything (no callback attached).`<br />
`PUSH :: will change the mode (incrementing) and print a message to serial output.`<br />
`ANY :: wont do anything (no callback attached).`

<br/>

<a id="declaring"></a>

## Declaring of 'MDSwitch' object:

<hr />

[Return](#table-contents)

Call `MDSwitch` with all required pins - Make sure you are using for SW_INT an external interrupt supported pin.<br />
the 3 terminal (key/button pins) can be any digital pin that can be set as INPUT.<br />

```cpp

#include <SIKTEC_MDSwitch.h>

...

//using namespace SIKtec; // Optional 

SIKtec::MDSwitch mdswitch(
    SW_PUSH, // Push button pin number -> middle switch terminal 
    SW_CCW,  // CCW button pin number  -> right switch terminal 
    SW_CW,   // CW button pin number   -> left switch terminal 
    SW_INT   // Shared interrupt pin.
);

...
```
> - **Note**: all terminals are and interrupt should be pulled low - If you are using SIKTEC's prototype board this is allready done.
> - **Note**: Since we assume terminals are pulled down, we can use Digital pins which can be only inputs without internal pulldown/up options (e.g. ESP32 34-39)

<br/>

<a id="all-the-key-codes"></a>

## All the key/buttons/terminals codes:

<hr />

[Return](#table-contents)

By default the shield has 4 key events - Events are mapped using a simple enum value;

| enum | Button              | More info |
|:----:|:--------------------|------------|
| `MDS_KEYS::PUSH`  | Middle switch "push in" Event |                                                                           |
| `MDS_KEYS::CW`    | Switch side push event        |                                                                           |
| `MDS_KEYS::CCW`   | Switch side push event        |                                                                           |
| `MDS_KEYS::ANY`   | Any of above                  | when attaching a callback to this event all keys will trigger the calback |

> **Note:** `MDS_KEYS::ANY` won't replace the key/button events - It will be triggered AFTER the specific callback (if there is one attached).

<br/>

<a id="initialization"></a>

## Initializing and setting MODES

<hr />

[Return](#table-contents)

Creating a **MDSwitch** object does not initialize it - Its necessary to call the `.init(const int _modes)` method which initializes the object and does all the pin declarations + allocates memory for the callbacks pointers.

```cpp
mdswitch.init(3 /* how many modes */); // each mode can take a set of callbacks.
```
The method expects an integer which sets how many MODES are we going to use - MODES are basically an internal state which can be switched. Each mode can have its own set of callbacks attached and invoked.
> **Note**: By default only 1 mode will be created - (mode index 0).

To change modes and set the current enabled/used mode:

```cpp
...

mdswitch.mode(0); //set mode 1
mdswitch.mode(1); //set mode 2
mdswitch.mode(2); //set mode 3

...

int usedmode = mdswitch.mode(); // returns the current enabled/used mode index.

```

<br/>

<a id="assigning-a-callback"></a>

## Assigning an Event callback

<hr />

[Return](#table-contents)

By default there is are no callbacks when a button is pressed/captured. To assign your custom callback functions attach them using the `.attach()` method.

```cpp

//Attach callbacks:
mdswitch.attach(-1, MDS_KEYS::PUSH, &cb_push);  // -1 means all modes - 0,1,2.
mdswitch.attach(0, MDS_KEYS::CCW,  &cb_ccw_cw);    // only in mode 0.
mdswitch.attach(0, MDS_KEYS::CW,   &cb_ccw_cw);     // only in mode 0.
mdswitch.attach(0, MDS_KEYS::ANY,  &cb_any);    // any will be called in mode 0 + 1
mdswitch.attach(1, MDS_KEYS::ANY,  &cb_any);
...
mdswitch.detach(1, MDS_KEYS::PUSH); // will detach the push event callback from mode 1 ONLY

```
> - **Note:** A good practice will be to define callbacks inside the `setup()` function.<br />
> - **MDS_KEYS::ANY** Is a special event which is triggered when any button/key event is captured - AFTER the dedicated callback.

All callbacks function should be declared as ` void cb(const int mode, const MDS_KEYS key)`.<br />
When a callback is invoked its called with the current mode and the key/button which invoked it. This way you can use the same function
for several event callbacks. 

```cpp
//Example of simple callbacks declaration:
void cb_ccw_cw(const int mode, const MDS_KEYS key) {

    if (key == MDS_KEYS::CCW)
        Serial.println("Invoked CCW callback");

    if (key == MDS_KEYS::CW)
        Serial.println("Invoked CW callback");
}

```
> - You don't need to care about ISR stuff in your callbacks (e.g volatile, IRAM_ATTR etc.) thats because the callback is not directly invoked by the interrupt handler - Its being called (if needed) from the `tick()` handler.<br/>
> - **Why cb's are not directly invoked?** ISR routines should be as fast as possible. The library ISR handler only debounces the event and sets an internal flag, indicating which event should be consumed. Doing more than that directly from an interrupt routine can cause other interrupt events to be ignored or in some systems exceptions and even reset of the system. Also, the code is simpler and easier for most of the library users. 

<br/>

<a id="ticking"></a>

## Auto invoking callbacks `tick()` method:

<hr />

[Return](#table-contents)

The best practice is to call the `tick()` method from the `loop()` - The tick method checks wether an event was captured and consumes it. By consume I mean if a callback was attached it will be triggered otherwise the event will be ignored.

```cpp
void loop() {

    //This is the tick - Extremely fast, just checks if a key was recorded.  
    //If an event was captured and it has an attached callback it will be triggered.
    mdswitch.tick();

    delay(20);
}
```

<br/>

<a id="more-methods"></a>

## MDSwitch additional Methods:

<hr />

[Return](#table-contents)

**Dynamically enable/disable callbacks**:

```cpp
mdswitch.enable();
mdswitch.disable();
```
**Reading the inputs programatically on demand**:

```cpp
MDS_KEYS key = MDSwitch::read();
```
**Programatically invoking a specific callback**:

```cpp
bool invoked1 = mdswitch.invoke(MDS_KEYS::CCW);     // will invoke the CCW callback in the current mode.
bool invoked2 = mdswitch.invoke(1, MDS_KEYS::CW);   // will invoke the CW callback in a specific mode.
```
> - The ability to invoke callbacks of other events in different modes is usefull when creating advanced menu based systems.
> - `invoke()` doesn't change the internal captured event nor the current mode of the object.
> - `ANY` callback will not be invoked automatically when using the `invoke()` method.  

<br/>

<a id="important-notes"></a>

## Additional notes: 

<hr />

[Return](#table-contents)

1. By default `MDSwitch` can hold up to 5 callbacks per mode. One callback for each event.
2. The library is implementing a non blocking delay of `80ms` for debouncing. If you need to adjust that - Change the defined value of `SIKTEC_MDS_DEBOUNCE_DELAY` in `SIKTEC_MDSwitch.h`.
3. Initialize the object with the ACTUAL number of modes you are planing to actually use - Each additional mode allocates additional memory to store the callbacks pointers.
