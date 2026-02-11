# VibeCalc: A 3D Calculator in C

This document summarizes the current state of the VibeCalc project, a 3D calculator application built in C using the `raylib` graphics library.

## Project Status

All core requirements and identified issues have been addressed. The calculator features a functional 3D interface, animations, and basic arithmetic operations.

### Completed Tasks:

*   Research lightweight C 3D graphics libraries suitable for Ubuntu 24.04 with minimal dependencies.
*   Verify raylib's dependencies and integration with C projects on Linux.
*   Install raylib and its dependencies on the system.
*   Set up the C project structure and a basic build system (e.g., Makefile).
*   Implement the core calculator logic (addition, subtraction, multiplication, division, modulus).
*   Integrate the chosen 3D graphics library and set up a basic rendering loop.
*   Design and implement 3D models for calculator elements (numbers, operators, display).
*   Implement launching animations for the calculator application.
*   Implement animations for arithmetic operations (button press animation).
*   Implement closing animations for the calculator application.
*   Apply a dark color theme to the 3D rendered elements.
*   Implement user input handling for numbers and operations.
*   Compile and test the application, ensuring all features and animations work correctly.
*   **Fixes based on screenshots**:
    *   Implemented 3D text rendering using `RenderTexture2D` and `DrawBillboard` for display and button labels.
    *   Corrected font loading to use `UbuntuMono-R.ttf`.
    *   Adjusted button layout, making the "0" button wider and correctly positioned.
    *   Fine-tuned camera position and billboard scaling for optimal visual presentation.
    *   Ensured all calculator elements are visible and properly aligned within the viewport.

## Key Implementation Details

*   **Graphics Library**: `raylib` (version 5.6-dev)
*   **Programming Language**: C
*   **Core Calculator Logic**: Implemented in `src/calculator.c` and `src/calculator.h`, supporting `add`, `subtract`, `multiply`, `divide` (with zero-division handling), and `modulus` (using `fmod` for floating-point values).
*   **3D Text Rendering**: Text for both the display and button labels is rendered in 3D using `raylib`'s `RenderTexture2D` to create textures from text, which are then displayed as `DrawBillboard` elements in the 3D scene. The `UbuntuMono-R.ttf` font is used.
*   **Animations**:
    *   **Launch Animation**: Calculator elements animate into view from above.
    *   **Button Press Animation**: Buttons briefly move inwards when clicked.
    *   **Closing Animation**: Elements animate upwards and off-screen when the application closes.
*   **Theming**: A dark color theme is applied to the calculator elements.
*   **User Input**: Mouse clicks are detected and translated into button presses via raycasting, updating the calculator's state and display.

## How to Build and Run

1.  **Dependencies**: Ensure `raylib` is installed on your system. The project expects `raylib` headers in `/usr/local/include` and library files in `/usr/local/lib`. If you're on Ubuntu 24.04, you would have installed dependencies like `build-essential`, `git`, `cmake`, `libx11-dev`, `libgl1-mesa-dev`, `libglu1-mesa-dev`, `libxrandr-dev`, `libxi-dev`, `libxcursor-dev`, `libxinerama-dev`.
2.  **Font File**: Ensure `UbuntuMono-R.ttf` is present in the project's root directory (same directory as `Makefile`).
3.  **Build**: Open a terminal in the project's root directory (`/home/timothy/code_base/vibecalc`) and run:
    ```bash
    make clean && make
    ```
4.  **Run**: Execute the compiled application:
    ```bash
    ./vibecalc
    ```
5.  **Interaction**: Use the mouse to click on the calculator buttons. To trigger the closing animation, click the window's close button (X) or press the `ESC` key.

## Future Improvements (Potential Next Steps)

*   Implement proper 3D text models instead of billboards for a more integrated look.
*   Add keyboard input support.
*   Improve error handling (e.g., display "Error" on division by zero instead of exiting).
*   Implement more complex calculator functions (e.g., square root, scientific functions).
*   Refine UI/UX for better aesthetics and usability.
*   Add lighting and shadows to the 3D scene.
