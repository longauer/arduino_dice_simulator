# D&D Dice Report

## Overview

This program uses a total of four classes to simulate dice throws and display the results on a 7-segment display. Each class is designed to handle specific tasks such as generating random throws, managing button inputs, displaying data, and animating the process.

## 1. RandomThrowGenerator Class

The `RandomThrowGenerator` class models the dice. It contains data about the size and number of dice being thrown, and it also implements the behavior of the dice, specifically the resulting sum of the numbers rolled. 

To simulate randomness in dice throws, I used the built-in `micros` function. The generation of pseudorandom numbers is based on the assumption that the precision of this function is sufficient, meaning that the regularity of the function calls within the program will not trivially influence the outputs. Additionally, this holds true for the last few digits of the output as well. 

During testing, I encountered an issue where the last digit of the `micros` measurement is always odd, so I decided to only use the digits in the tens and hundreds places. To generate numbers in the range of 0-99, I implemented the `random_number` function based on the aforementioned assumption. 

To generalize this for generating numbers in the range of 0-x (where x<99), I created the `random_in_range` function. This function generates numbers in the given range approximately uniformly, assuming that `random_number` generates uniformly random integers in the 0-99 range.

The `RandomThrowGenerator` class can be in one of three states at any given time: `throw_generation`, `dice_config`, and `throws_config`. Each of these states has a corresponding function that defines the behavior for that particular mode. The function to execute is determined by a simple switch statement inside the `loop` function.

For handling button inputs and managing the display of data on a 7-segment display, I use specialized classes `Button` and `Display`.

---

## 2. Button Class

The `Button` class has two basic states: `pressed` and `released`. It implements methods to check the button’s state: 

- `is_pressed`
- `is_released`
- `activated`

These methods help determine if the button is being pressed or released, and whether it has been activated.

---

## 3. Display Class

The `Display` class implements two setter methods for setting the variables `dice_size_` and `num_throws_`. These are essential for displaying data on the 7-segment display. The class supports three display modes:

- `show`: Displays the set parameters in the configuration modes of the `RandomThrowGenerator` class.
- `show_throw`: Displays the results of the dice throws.
- `show_animation`: Displays an animation during the random number generation process.

To handle the animation itself, I created another class called `Animation`, which manages the entire animation process for the display.

---

## 4. Animation Design

When designing the animation, I focused on maximizing parameterization, so that the animation could be easily adjusted and fine-tuned to the desired form later on. The structure of the animation consists of moving elements in both vertical and horizontal segments of the display. Each element has its own independently adjustable movement frequency.

To implement the movement of these elements in the animation, I used three methods:

- `move_UV` (upper vertical)
- `move_LV` (lower vertical)
- `move_horizontal`

For transitions between states of the animation elements, I used appropriate bit masks. The class also implements a `loop` method that handles the display of the animation, as well as multiplexing.

---

## Summary

- The `RandomThrowGenerator` class manages dice throws, using a micros-based pseudorandom number generator.
- The `Button` class tracks the state of the buttons with methods to check if the button is pressed, released, or activated.
- The `Display` class manages three modes for displaying data: showing parameters, showing results, and showing animations.
- The `Animation` class handles the movement and transitions of display elements for a dynamic animation.
