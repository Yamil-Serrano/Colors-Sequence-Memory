# Colors Sequence Memory

A simple memory game built on the ESP8266, using 74HC595 and 74HC165 shift registers to control LEDs and read buttons with minimal GPIO usage for them.
The game shows a random color sequence on LEDs that the player must repeat using buttons.
If the player matches the sequence, the game adds a new random color until reaching a maximum of 10 colors. If the player makes a mistake, the game ends.

---

> ⚠️ Note: The full version is currently in development. For now, you can use the basic setup and explore the code to understand how the shift registers work together.

---

## 🧩 Part of the Shift Register I/O Expansion Series

This project combines what was learned in these earlier repos:

* **Part 1**: [74HC595 – Output Expansion](https://github.com/Yamil-Serrano/74HC595-ESP8266-Output-Expansion)
* **Part 2**: [74HC165 – Input Expansion](https://github.com/Yamil-Serrano/74HC165-ESP8266-Input-Expansion)

Now, in **Sequence-Memory**, we use both together to build an actual interactive game.

---

## What You'll Learn

* How to use both **74HC595** and **74HC165** simultaneously
* How to create a memory-based color sequence game
* How to use shared clock lines efficiently
* Combining input reading and LED control with minimal GPIOs

---

## How It Works

* The game starts with a random sequence of 4 colors (Red, Yellow, Green, Blue).
* Colors are represented by specific bits in a byte sent to the 74HC595, lighting up LEDs.
* The player repeats the sequence by pressing corresponding buttons (read via 74HC165).
* If correct, a new random color is added to the sequence.
* The game continues until the player makes a mistake or reaches the max length.

---

## Hardware Used

* ESP8266 (NodeMCU, Wemos D1 mini, etc.)
* 74HC595 (Output shift register)
* 74HC165 (Input shift register)
* 4 LEDs: Red, Yellow, Green, Blue
* 4 Push buttons
* Buzzer
* Breadboard + jumper wires
* Pull-down resistors (10kΩ)

---

## Getting Started

1. Clone this repository:

```bash
git clone https://github.com/Yamil-Serrano/Colors-Sequence-Memory.git
```

2. Open `src/main.cpp` in Arduino IDE or PlatformIO.
3. Wire the circuit following the schematic (coming soon).
4. Upload and play!

---

## License

This project is licensed under the MIT License – see the [LICENSE](LICENSE.md) file for details.

---

## Contact

If you have any questions or suggestions, feel free to reach out to me:

GitHub: [Neowizen](https://github.com/Yamil-Serrano)
