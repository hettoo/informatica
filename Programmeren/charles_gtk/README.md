# Charles GTK

charles\_gtk is a GTK rewrite of the Charles framework used for programming
assignments.

## Usage

Executing `make` will result in building your code with my implementation and
executing the resulting executable. Use `make help` to see other useful targets.

## Keybinds

### Main

* q: Quit
* -: Low speed
* =: Normal speed
* +: High speed
* r: Reset everything

### Assignments

* 1: Go back to point of departure
* 2: Clean string with balls
* 3: Clean chaos with balls
* 4: Follow path
* 5: Leave labyrinth
* 6: Write number
* 7: Add numbers
* 8: Subtract numbers
* 9: Multiply numbers

### Manual control

* j: Turn left
* l: Turn right
* k: Step
* b: Toggle ball
* w: Toggle wall
* B: Sticky ball
* W: Sticky wall
* i: Toggle invincibility
* o: Open a file
* s: Save to a file

### View

* ,: Zoom in
* .: Zoom out
* f: Toggle fullscreen
* a: Align window to world size
* Arrows: Move camera

## Mousebinds

* Right button: Walk to the field pointed to by the mouse

## Notes

* Don't hand in the `user.cpp` file, put the custom part in the original file
  (`Charles.cpp`), make sure it works and hand in thatone instead.
* The `add()`, `subtract()` and `multiply()` functions are set up and called
  automatically by my implementation, but not by the original one. You will have
  to write that yourself in the end. Note that my code for these functions
  depends on your own `write_number(int positive decimal)` function, so make
  sure it works exactly as requested before doing the next assignments.

