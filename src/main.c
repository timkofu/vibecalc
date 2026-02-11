#include "raylib.h"
#include "calculator.h" // Include our calculator logic
#include <string.h>     // Required for strcmp
#include <math.h>       // Required for sinf and PI
#include <stdio.h>      // Required for sprintf
#include <stdlib.h>     // Required for atof

// Define button layout and dimensions
#define BUTTON_SIZE 1.5f
#define BUTTON_SPACING 0.2f
#define GRID_COLS 4
#define GRID_ROWS 5 // Changed to 5 rows for clear button

// Calculator state
double currentValue = 0.0;
double previousValue = 0.0;
char currentOperator = ' '; // '+', '-', '*', '/', '%', '='
bool clearDisplay = true;
char displayText[32] = "0";

// Button labels (hardcoded) - now with a 'C' button and '0' is wider
const char *buttonLabels[GRID_ROWS][GRID_COLS] = {
    {"C", "/", "*", "-"},
    {"7", "8", "9", "+"},
    {"4", "5", "6", "%"},
    {"1", "2", "3", "="},
    {"0", ".", "", ""} // Empty slots for layout
};

// Button animation states (per button)
typedef struct {
    bool pressed;
    float progress;
    Texture2D textTexture; // Texture for button label
} ButtonAnimState;

ButtonAnimState buttonAnimStates[GRID_ROWS][GRID_COLS] = {0};
const float buttonPressAnimationDuration = 0.2f; // seconds for press and release

// Render texture for the display
RenderTexture2D displayRenderTexture;
Font calculatorFont;

void processButton(const char *label) {
    if (strcmp(label, "C") == 0) { // Clear
        currentValue = 0.0;
        previousValue = 0.0;
        currentOperator = ' ';
        clearDisplay = true;
        strcpy(displayText, "0");
    } else if (strcmp(label, "=") == 0) {
        if (currentOperator != ' ') {
            double result = 0.0;
            switch (currentOperator) {
                case '+': result = add(previousValue, currentValue); break;
                case '-': result = subtract(previousValue, currentValue); break;
                case '*': result = multiply(previousValue, currentValue); break;
                case '/': result = divide(previousValue, currentValue); break;
                case '%': result = modulus(previousValue, currentValue); break;
            }
            sprintf(displayText, "%.2f", result); // Format to 2 decimal places
            currentValue = result;
            previousValue = 0.0;
            currentOperator = ' ';
            clearDisplay = true;
        }
    } else if (strcmp(label, "+") == 0 || strcmp(label, "-") == 0 ||
               strcmp(label, "*") == 0 || strcmp(label, "/") == 0 ||
               strcmp(label, "%") == 0) { // Operators
        previousValue = currentValue;
        currentOperator = label[0];
        clearDisplay = true;
    } else if (strcmp(label, ".") == 0) { // Decimal point
        if (clearDisplay) {
            strcpy(displayText, "0.");
            clearDisplay = false;
        } else if (!strchr(displayText, '.')) { // Only add if not already present
            strcat(displayText, ".");
        }
        currentValue = atof(displayText);
    } else { // Numbers
        if (clearDisplay) {
            strcpy(displayText, label);
            clearDisplay = false;
        } else {
            if (strlen(displayText) < 10) { // Limit display length
                strcat(displayText, label);
            }
        }
        currentValue = atof(displayText);
    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "VibeCalc 3D Calculator");

    // Define the camera to look into our 3D world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 12.0f, 25.0f };  // Adjusted camera position: higher and further back
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 55.0f;                                // Increased field-of-view
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Load font for 3D text
    calculatorFont = LoadFont("UbuntuMono-R.ttf"); // Assuming font is in cwd
    
    // Setup display render texture
    displayRenderTexture = LoadRenderTexture(300, 70); // Adjusted size for better display text

    // Pre-render button labels to textures
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            if (strlen(buttonLabels[row][col]) > 0) {
                // Determine texture size based on text and desired font size
                Vector2 textSize = MeasureTextEx(calculatorFont, buttonLabels[row][col], 40, 0); // Increased font size
                // Create a render texture for each button label
                RenderTexture2D buttonTextRenderTexture = LoadRenderTexture((int)textSize.x + 8, (int)textSize.y + 8); // Increased padding
                BeginTextureMode(buttonTextRenderTexture);
                    ClearBackground(BLANK); // Transparent background
                    DrawTextEx(calculatorFont, buttonLabels[row][col], (Vector2){4,4}, 40, 0, RAYWHITE); // Increased font size and offset
                EndTextureMode();
                buttonAnimStates[row][col].textTexture = buttonTextRenderTexture.texture;
            }
        }
    }


    // Animation variables
    float launchAnimationProgress = 0.0f; // 0.0 to 1.0
    const float launchAnimationDuration = 1.5f; // seconds

    // Closing animation variables
    bool isClosing = false;
    float closingAnimationProgress = 0.0f; // 0.0 to 1.0
    const float closingAnimationDuration = 1.0f; // seconds

    // Main game loop
    while (!WindowShouldClose() || isClosing)    // Detect window close button or ESC key, or if closing animation is active
    {
        // Update
        //----------------------------------------------------------------------------------
        if (!isClosing) {
            if (launchAnimationProgress < 1.0f) {
                launchAnimationProgress += GetFrameTime() / launchAnimationDuration;
                if (launchAnimationProgress > 1.0f) launchAnimationProgress = 1.0f;
            }

            // Update individual button animations
            for (int row = 0; row < GRID_ROWS; row++) {
                for (int col = 0; col < GRID_COLS; col++) {
                    if (buttonAnimStates[row][col].pressed) {
                        buttonAnimStates[row][col].progress += GetFrameTime() / buttonPressAnimationDuration;
                        if (buttonAnimStates[row][col].progress > 1.0f) {
                            buttonAnimStates[row][col].progress = 1.0f;
                            buttonAnimStates[row][col].pressed = false;
                        }
                    }
                }
            }


            // Mouse input
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && launchAnimationProgress >= 1.0f) { // Only allow input after launch animation
                Ray mouseRay = GetMouseRay(GetMousePosition(), camera);

                // Check for button clicks
                float startX = -(GRID_COLS * BUTTON_SIZE + (GRID_COLS - 1) * BUTTON_SPACING) / 2 + BUTTON_SIZE / 2;
                float startY = -(GRID_ROWS * BUTTON_SIZE + (GRID_ROWS - 1) * BUTTON_SPACING) / 2 + BUTTON_SIZE / 2;
                float currentZ = 0.0f;

                for (int row = 0; row < GRID_ROWS; row++) {
                    for (int col = 0; col < GRID_COLS; col++) {
                        if (strlen(buttonLabels[row][col]) == 0) continue; // Skip empty buttons

                        // Adjust button width for '0' button
                        float currentButtonWidth = BUTTON_SIZE;
                        if (row == GRID_ROWS - 1 && col == 0) { // '0' button
                            currentButtonWidth = BUTTON_SIZE * 2 + BUTTON_SPACING;
                        }

                        Vector3 buttonCenter = {
                            startX + col * (BUTTON_SIZE + BUTTON_SPACING) + (strcmp(buttonLabels[row][col], "0") == 0 ? BUTTON_SIZE / 2 + BUTTON_SPACING / 2 : 0.0f),
                            startY + row * (BUTTON_SIZE + BUTTON_SPACING),
                            currentZ
                        };
                        BoundingBox buttonBox = {
                            (Vector3){ buttonCenter.x - currentButtonWidth/2, buttonCenter.y - BUTTON_SIZE/2, buttonCenter.z - 0.25f },
                            (Vector3){ buttonCenter.x + currentButtonWidth/2, buttonCenter.y + BUTTON_SIZE/2, buttonCenter.z + 0.25f }
                        };

                        RayCollision collision = GetRayCollisionBox(mouseRay, buttonBox);
                        if (collision.hit) {
                            processButton(buttonLabels[row][col]);
                            buttonAnimStates[row][col].pressed = true;
                            buttonAnimStates[row][col].progress = 0.0f;
                            break; // Only one button can be clicked at a time
                        }
                    }
                }
            }


            if (WindowShouldClose()) {
                isClosing = true;
                closingAnimationProgress = 0.0f;
            }
        } else { // Handle closing animation
            closingAnimationProgress += GetFrameTime() / closingAnimationDuration;
            if (closingAnimationProgress > 1.0f) {
                break; // Exit loop when closing animation is complete
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);

                float currentOffsetY;
                if (!isClosing) {
                    // Calculate vertical offset for launch animation
                    currentOffsetY = (1.0f - launchAnimationProgress) * 15.0f; // Start 15 units above, move to 0
                } else {
                    // Calculate vertical offset for closing animation (reverse of launch)
                    currentOffsetY = closingAnimationProgress * 15.0f; // Move 15 units upwards
                }

                // Draw calculator display (a flat cuboid)
                Vector3 displayPosition = { 0.0f, BUTTON_SIZE * 2.0f + currentOffsetY, 0.0f }; // Lowered display Y-position
                Vector3 displaySize = { BUTTON_SIZE * 4 + BUTTON_SPACING * 3, BUTTON_SIZE, 0.5f };
                DrawCube(displayPosition, displaySize.x, displaySize.y, displaySize.z, DARKBLUE); // Dark color for display
                DrawCubeWires(displayPosition, displaySize.x, displaySize.y, displaySize.z, BLACK);

                // Draw display text in 3D using billboard
                BeginTextureMode(displayRenderTexture);
                    ClearBackground(BLANK); // Transparent background
                    DrawTextEx(calculatorFont, displayText, (Vector2){4,4}, 40, 0, RAYWHITE); // Increased font size for display
                EndTextureMode();
                // Draw display texture as a billboard
                DrawBillboard(camera, displayRenderTexture.texture, (Vector3){displayPosition.x, displayPosition.y + 0.05f, displayPosition.z + 0.26f}, 0.5f, RAYWHITE); // Reduced billboard size


                // Calculate the starting position for the button grid
                float startX = -(GRID_COLS * BUTTON_SIZE + (GRID_COLS - 1) * BUTTON_SPACING) / 2 + BUTTON_SIZE / 2;
                float startY = -(GRID_ROWS * BUTTON_SIZE + (GRID_ROWS - 1) * BUTTON_SPACING) / 2 + BUTTON_SIZE / 2;
                float currentZ = 0.0f; // Buttons will be flat on the Z plane

                // Draw buttons
                for (int row = 0; row < GRID_ROWS; row++) {
                    for (int col = 0; col < GRID_COLS; col++) {
                        if (strlen(buttonLabels[row][col]) == 0) continue; // Skip empty buttons

                        // Adjust button width for '0' button
                        float currentButtonWidth = BUTTON_SIZE;
                        if (row == GRID_ROWS - 1 && col == 0) { // '0' button
                            currentButtonWidth = BUTTON_SIZE * 2 + BUTTON_SPACING;
                        }

                        float buttonZOffset = 0.0f;
                        if (buttonAnimStates[row][col].pressed) {
                            buttonZOffset = -0.2f * sinf(buttonAnimStates[row][col].progress * PI); // In and out effect
                        }

                        Vector3 buttonPos = {
                            startX + col * (BUTTON_SIZE + BUTTON_SPACING) + (strcmp(buttonLabels[row][col], "0") == 0 ? BUTTON_SIZE / 2 + BUTTON_SPACING / 2 : 0.0f),
                            startY + row * (BUTTON_SIZE + BUTTON_SPACING) + currentOffsetY, // Apply animation offset
                            currentZ + buttonZOffset // Apply press animation offset
                        };
                        Color buttonColor = GRAY; // Default button color
                        if (strcmp(buttonLabels[row][col], "/") == 0 || strcmp(buttonLabels[row][col], "*") == 0 ||
                            strcmp(buttonLabels[row][col], "-") == 0 || strcmp(buttonLabels[row][col], "+") == 0 ||
                            strcmp(buttonLabels[row][col], "%") == 0) { // Added modulus
                            buttonColor = ORANGE; // Operators
                        } else if (strcmp(buttonLabels[row][col], "=") == 0) {
                            buttonColor = LIME; // Equals button
                        } else if (strcmp(buttonLabels[row][col], "C") == 0) {
                            buttonColor = RED; // Clear button
                        } else {
                            buttonColor = DARKGRAY; // Number buttons
                        }
                        
                        DrawCube(buttonPos, currentButtonWidth, BUTTON_SIZE, 0.5f, buttonColor); // Use currentButtonWidth
                        DrawCubeWires(buttonPos, currentButtonWidth, BUTTON_SIZE, 0.5f, BLACK);

                        // Draw button label in 3D using pre-rendered texture
                        DrawBillboard(camera, buttonAnimStates[row][col].textTexture, (Vector3){buttonPos.x, buttonPos.y + 0.1f, buttonPos.z + 0.26f}, 0.3f, RAYWHITE); // Reduced billboard size
                    }
                }

            EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(displayRenderTexture);
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            if (strlen(buttonLabels[row][col]) > 0) {
                UnloadTexture(buttonAnimStates[row][col].textTexture);
            }
        }
    }
    UnloadFont(calculatorFont); // Unload the font
    CloseWindow();        // Close window and unload OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}