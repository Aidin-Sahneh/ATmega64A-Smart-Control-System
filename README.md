ATmega64A Smart System Controller (Digital Systems 2 Lab)
=========================================================

This is the final project for the "Digital Systems 2 Lab" course (Student ID: 40120243) at K.N. Toosi University, instructed by Dr. Aslani.

The project is a comprehensive smart control system built around the **ATmega64A microcontroller** and simulated in **Proteus**. It features a real-time status display, remote control via a simulated Bluetooth (UART) connection, and various sensor integrations.

*(Note: Add a screenshot of the schematic from your report to the `docs/media` folder)*

Features
--------

-   **Real-Time LCD Display:** A 20x4 LCD shows all system statuses, including temperature and device states.

-   **Temperature Sensing:** An **LM35 sensor** reads ambient temperature, which can be displayed in Celsius or Fahrenheit, toggled by a push-button (SW1).

-   **Bluetooth (UART) Control:** A virtual terminal simulates a Bluetooth module, allowing for:

    -   **Individual Device Control:** Remotely turn 4 LEDs, 2 Relays, and 1 Buzzer ON/OFF using single-character commands (e.g., 'A'/'a').

    -   **Simultaneous Relay Control:** A single command ('L'/'l') controls both relays at once.

    -   **Two-Way Terminal:** The system automatically transmits a status string (Temp, Relay states) every 500ms and receives text from the user to display on the LCD.

-   **RGB LED Control:** A common-cathode RGB LED is controlled via **Hardware PWM** (Timers 1 & 2). The system parses hex color codes (e.g., `#RRGGBB`) sent over the terminal to set the color.

-   **On-Board Controls:** Hardware push-buttons are used for:

    -   Toggling C/F temperature units (SW1).

    -   Activating a synchronized LED blinking mode (SW2).

    -   Controlling the blink speed (SW3 & SW4).

Hardware & Software
-------------------

-   **Microcontroller:** ATmega64A

-   **Simulation:** Proteus ISIS

-   **IDE:** CodeVisionAVR

-   **Key Peripherals:**

    -   LM044L (20x4 LCD)

    -   LM35 (Analog Temperature Sensor)

    -   Virtual Terminal (Simulating HC-05)

    -   2N2222 Transistors (for Relay/Buzzer drivers)

    -   RGB Common-Cathode LED

How to Run the Simulation
-------------------------

1.  Clone this repository.

2.  Open the `simulation/Aidin Sahneh.pdsprj` file in Proteus.

3.  Open the `firmware/Aidin-Digital2.cproj` file in CodeVisionAVR, compile it, and generate the `.hex` file.

4.  In Proteus, ensure the ATmega64A component is programmed to use the `.hex` file you just generated.

5.  Run the simulation.

6.  Interact with the system using the on-board push-buttons (SW1-SW4).

7.  Open the **Virtual Terminal** to send commands (e.g., 'A'/'a' for LED1, 'L'/'l' for both relays, or `#FF0000` for the RGB LED) and receive status updates.