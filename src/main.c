#include "raylib.h"
#include "raymath.h"
#include "calculator.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// --- Constants & Config ---
#define BUTTON_SIZE 1.5f
#define BUTTON_SPACING 0.3f
#define BUTTON_HEIGHT 0.4f
#define CHASSIS_PADDING 0.5f
#define GRID_COLS 4
#define GRID_ROWS 5
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Colors
#define COL_CHASSIS (Color){ 30, 30, 35, 255 }
#define COL_SCREEN_BG (Color){ 10, 20, 10, 255 }
#define COL_SCREEN_TEXT (Color){ 100, 255, 100, 255 }
#define COL_BTN_NUM (Color){ 60, 60, 65, 255 }
#define COL_BTN_OP (Color){ 255, 165, 0, 255 }
#define COL_BTN_EQ (Color){ 50, 205, 50, 255 }
#define COL_BTN_CLR (Color){ 220, 50, 50, 255 }
#define COL_TEXT_LIGHT RAYWHITE
#define COL_TEXT_DARK BLACK

// State
double currentValue = 0.0;
double previousValue = 0.0;
char currentOperator = ' ';
bool clearDisplay = true;
char displayText[32] = "0";

// Layout
const char *buttonLabels[GRID_ROWS][GRID_COLS] = {
    {"C", "/", "*", "-"},
    {"7", "8", "9", "+"},
    {"4", "5", "6", "%"},
    {"1", "2", "3", "="},
    {"0", "", ".", ""} 
};

// Animation
typedef struct {
    bool pressed;
    float progress; // 0.0 to 1.0
} ButtonAnimState;

ButtonAnimState buttonAnimStates[GRID_ROWS][GRID_COLS] = {0};

// Resources
Font appFont;
Model buttonModel; // Unit cube
Model screenModel; // Quad
RenderTexture2D screenTexture;
RenderTexture2D buttonTextures[GRID_ROWS][GRID_COLS];

// Helper: Calculate button center position
Vector3 GetButtonPosition(int row, int col, float layoutWidth, float layoutHeight, float startY) {
    float startX = -layoutWidth / 2.0f + BUTTON_SIZE / 2.0f;
    // float startZ = -layoutHeight / 2.0f + BUTTON_SIZE / 2.0f; // Original Z logic was flipped? Let's stick to X/Y plane for layout on a desk?
    // Let's model it: X is left/right, Z is up/down on the desk (depth), Y is height of button.
    // The camera is looking down.
    
    // Grid:
    // Row 0 is Top (furthest Z?) or Top (highest Y on screen?). 
    // Let's say Z is "depth" into the screen. -Z is further away.
    // So Row 0 is at negative Z (top of calculator).
    
    float x = startX + col * (BUTTON_SIZE + BUTTON_SPACING);
    
    // Handle '0' button width offset
    if (row == GRID_ROWS - 1 && col == 0) {
        x += (BUTTON_SIZE + BUTTON_SPACING) * 0.5f; 
    }
    
    float z = startY + row * (BUTTON_SIZE + BUTTON_SPACING);
    
    return (Vector3){ x, 0.0f, z };
}

// Logic
void processButton(const char *label) {
    if (strcmp(label, "C") == 0) {
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
            if (isnan(result)) {
                strcpy(displayText, "Error");
                currentValue = 0.0;
            } else {
                sprintf(displayText, "%.2f", result);
                currentValue = result;
            }
            previousValue = 0.0;
            currentOperator = ' ';
            clearDisplay = true;
        }
    } else if (strchr("+-*/%", label[0]) && label[1] == '\0') {
        previousValue = currentValue;
        currentOperator = label[0];
        clearDisplay = true;
    } else if (strcmp(label, ".") == 0) {
        if (clearDisplay) {
            strcpy(displayText, "0.");
            clearDisplay = false;
        } else if (!strchr(displayText, '.')) {
            strcat(displayText, ".");
        }
        currentValue = atof(displayText);
    } else {
        if (clearDisplay) {
            strcpy(displayText, label);
            clearDisplay = false;
        } else {
            if (strlen(displayText) < 12) {
                strcat(displayText, label);
            }
        }
        currentValue = atof(displayText);
    }
}

void checkKeyboardInput() {
    int key = 0;
    while ((key = GetKeyPressed()) != 0) {
        const char *label = NULL;
        if (key >= KEY_ZERO && key <= KEY_NINE) {
            if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
                static char numStr[2] = {0};
                numStr[0] = '0' + (key - KEY_ZERO);
                label = numStr;
            } else if (key == KEY_FIVE) label = "%";
            else if (key == KEY_EIGHT) label = "*";
        }
        else if (key >= KEY_KP_0 && key <= KEY_KP_9) {
            static char numStr[2] = {0};
            numStr[0] = '0' + (key - KEY_KP_0);
            label = numStr;
        }
        else if (key == KEY_KP_DECIMAL || key == KEY_PERIOD) label = ".";
        else if (key == KEY_KP_ADD) label = "+";
        else if (key == KEY_KP_SUBTRACT || key == KEY_MINUS) label = "-";
        else if (key == KEY_KP_MULTIPLY) label = "*";
        else if (key == KEY_KP_DIVIDE || key == KEY_SLASH) label = "/";
        else if (key == KEY_KP_EQUAL || key == KEY_ENTER || key == KEY_KP_ENTER) label = "=";
        else if (key == KEY_BACKSPACE || key == KEY_DELETE) label = "C";
        else if (key == KEY_EQUAL) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) label = "+";
            else label = "=";
        }

        if (label) {
            processButton(label);
            for (int r = 0; r < GRID_ROWS; r++) {
                for (int c = 0; c < GRID_COLS; c++) {
                    if (strcmp(buttonLabels[r][c], label) == 0) {
                        buttonAnimStates[r][c].pressed = true;
                        buttonAnimStates[r][c].progress = 0.0f;
                        goto next_key;
                    }
                }
            }
            next_key:;
        }
    }
}

RenderTexture2D CreateButtonTexture(const char* text, Color bgColor, Color textColor) {
    RenderTexture2D target = LoadRenderTexture(128, 128);
    BeginTextureMode(target);
        ClearBackground(bgColor);
        
        // Add a subtle border/bevel effect
        DrawRectangleLines(0, 0, 128, 128, Fade(BLACK, 0.2f));
        DrawRectangleLines(1, 1, 126, 126, Fade(BLACK, 0.2f));
        DrawRectangleLines(2, 2, 124, 124, Fade(BLACK, 0.1f));
        
        Vector2 textSize = MeasureTextEx(appFont, text, 60, 2);
        Vector2 pos = { (128 - textSize.x) / 2, (128 - textSize.y) / 2 };
        DrawTextEx(appFont, text, pos, 60, 2, textColor);
    EndTextureMode();
    return target;
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VibeCalc 3D");
    
    // Camera Setup
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 15.0f, 12.0f }; // High angle
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load Resources
    appFont = LoadFont("UbuntuMono-R.ttf");
    SetTextureFilter(appFont.texture, TEXTURE_FILTER_BILINEAR); 

    // Generate Button Textures
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            const char* label = buttonLabels[r][c];
            if (strlen(label) == 0) continue;
            
            Color bg = COL_BTN_NUM;
            Color fg = COL_TEXT_LIGHT;
            if (strchr("+-*/%", label[0])) { bg = COL_BTN_OP; fg = COL_TEXT_DARK; }
            else if (label[0] == '=') { bg = COL_BTN_EQ; fg = COL_TEXT_DARK; }
            else if (label[0] == 'C') { bg = COL_BTN_CLR; fg = COL_TEXT_LIGHT; }
            
            buttonTextures[r][c] = CreateButtonTexture(label, bg, fg);
        }
    }

    screenTexture = LoadRenderTexture(512, 128);
    
    // Models
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    buttonModel = LoadModelFromMesh(cubeMesh); // Reused for buttons and chassis parts
    
    // Animation Vars
    float introAnim = 0.0f;
    bool isClosing = false;
    float closeAnim = 0.0f;

    SetTargetFPS(60);

    // Dimensions
    float gridW = GRID_COLS * BUTTON_SIZE + (GRID_COLS - 1) * BUTTON_SPACING;
    float gridH = GRID_ROWS * BUTTON_SIZE + (GRID_ROWS - 1) * BUTTON_SPACING;
    float chassisW = gridW + CHASSIS_PADDING * 2;
    float chassisH = gridH + BUTTON_SIZE * 2 + CHASSIS_PADDING * 2; // Extra space for screen
    
    // Start layout calculation
    // Center the grid on the chassis.
    // Screen is above the grid.
    
    while (!WindowShouldClose() || isClosing) {
        float dt = GetFrameTime();

        // Intro/Outro Logic
        if (!isClosing) {
            introAnim += dt * 1.5f;
            if (introAnim > 1.0f) introAnim = 1.0f;
            
            if (WindowShouldClose()) {
                isClosing = true;
            }
            
            if (introAnim >= 1.0f) {
                // Input Processing
                checkKeyboardInput();
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Ray ray = GetMouseRay(GetMousePosition(), camera);
                    
                    // Hit test buttons
                    // Need to reconstruct button positions exactly as drawn
                    float startY = -gridH / 2.0f + BUTTON_SIZE; // Push grid down to make room for screen
                    
                    for (int r = 0; r < GRID_ROWS; r++) {
                        for (int c = 0; c < GRID_COLS; c++) {
                            if (strlen(buttonLabels[r][c]) == 0) continue;
                            
                            Vector3 pos = GetButtonPosition(r, c, gridW, gridH, startY);
                            float w = BUTTON_SIZE;
                            if (r == GRID_ROWS - 1 && c == 0) w = BUTTON_SIZE * 2 + BUTTON_SPACING;
                            
                            BoundingBox box = {
                                (Vector3){ pos.x - w/2, pos.y - BUTTON_HEIGHT/2, pos.z - BUTTON_SIZE/2 },
                                (Vector3){ pos.x + w/2, pos.y + BUTTON_HEIGHT/2, pos.z + BUTTON_SIZE/2 }
                            };
                            
                            RayCollision collision = GetRayCollisionBox(ray, box);
                            if (collision.hit) {
                                processButton(buttonLabels[r][c]);
                                buttonAnimStates[r][c].pressed = true;
                                buttonAnimStates[r][c].progress = 0.0f;
                            }
                        }
                    }
                }
            }
        } else {
            closeAnim += dt * 2.0f;
            if (closeAnim >= 1.0f) break;
        }

        // Update Button Anims
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_COLS; c++) {
                if (buttonAnimStates[r][c].pressed) {
                    buttonAnimStates[r][c].progress += dt * 5.0f; // Fast press
                    if (buttonAnimStates[r][c].progress >= 1.0f) {
                        buttonAnimStates[r][c].pressed = false;
                        buttonAnimStates[r][c].progress = 0.0f;
                    }
                }
            }
        }

        // Draw Screen Texture
        BeginTextureMode(screenTexture);
            ClearBackground(COL_SCREEN_BG);
            // Draw digital-looking background grid?
            DrawRectangleLines(0, 0, 512, 128, Fade(COL_SCREEN_TEXT, 0.1f));
            
            // Draw Text Right Aligned
            Vector2 textSize = MeasureTextEx(appFont, displayText, 80, 2);
            DrawTextEx(appFont, displayText, (Vector2){ 512 - textSize.x - 20, (128 - textSize.y)/2 }, 80, 2, COL_SCREEN_TEXT);
        EndTextureMode();

        // ---------------- Draw Scene ----------------
        BeginDrawing();
        ClearBackground((Color){ 20, 20, 20, 255 }); // Dark world background

        BeginMode3D(camera);
            
            // Animation Transform
            // Simple cubic ease out: 1 - (1-t)^3
            float t = introAnim;
            float easeOut = 1.0f - powf(1.0f - t, 3.0f);
            float animOffset = (1.0f - easeOut) * 20.0f;
            
            if (isClosing) {
                float t2 = closeAnim;
                float easeIn = t2 * t2 * t2;
                animOffset -= easeIn * 20.0f;
            }

            // Apply global transform (simulated by adding to positions)
            
            // 1. Draw Chassis
            Vector3 chassisPos = { 0.0f, -0.5f + animOffset, 0.0f }; // Slightly below Y=0
            DrawCube(chassisPos, chassisW, 0.5f, chassisH, COL_CHASSIS);
            DrawCubeWires(chassisPos, chassisW, 0.5f, chassisH, BLACK); // Outline
            
            // 2. Draw Screen (Inset)
            // Screen is at top of chassis.
            float screenZ = -chassisH / 2.0f + BUTTON_SIZE + CHASSIS_PADDING; 
            Vector3 screenPos3D = { 0.0f, 0.0f + animOffset, screenZ }; // Slightly above chassis surface
            
            // Screen Bezel
            DrawCube((Vector3){screenPos3D.x, screenPos3D.y - 0.1f, screenPos3D.z}, chassisW - CHASSIS_PADDING, 0.4f, BUTTON_SIZE + 0.5f, BLACK);

            // Screen Content (Textured Plane/Cube)
            // We use a thin cube for the screen "glass"
            Model screenQuad = buttonModel; // Reuse cube
            screenQuad.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = screenTexture.texture;
            
            // We need to rotate the cube so the top face shows the texture correctly?
            // Actually, simpler to DrawCubeTexture if we had it.
            // Let's use DrawBillboard? No, perspective.
            // Let's use a specialized Quad model or just map it.
            // Since GenMeshCube maps to all faces, we just scale it flat.
            // Note: Raylib GenMeshCube UVs might be flipped.
            
            // Let's use DrawCube for the glass and DrawPlane for the text?
            // Or DrawModelEx with rotation.
            // Standard Cube UVs: Top face is usually +Y.
            
            // To be safe and ensure orientation: Draw a textured quad slightly above the black bezel.
            // rlPushMatrix();
            // rlTranslatef(screenPos3D.x, screenPos3D.y + 0.11f, screenPos3D.z);
            // rlRotatef(90, 1, 0, 0); // Face up
            // DrawPlane((Vector3){0,0,0}, (Vector2){chassisW - CHASSIS_PADDING - 0.2f, BUTTON_SIZE}, COL_SCREEN_BG); 
            // ... Texture mapping on DrawPlane is full.
            
            // Let's stick to the simplest valid approach: A very thin cube with the texture.
            // Issue: Text will be on sides too. 
            // Solution: Dark sides are invisible if thin enough or same color.
            
            // Alternative: Use DrawModel with a Plane Mesh for the screen.
            // Mesh plane = GenMeshPlane(chassisW - CHASSIS_PADDING - 0.2f, BUTTON_SIZE, 1, 1);
            // But we can't gen mesh every frame.
            
            // Reuse buttonModel (Cube) but scale Y to very small.
            // Texture mapping on Top Face of Cube:
            // If we look at raylib source, GenMeshCube maps texture to all faces.
            // So if we have a texture, it appears on top.
            
            Vector3 screenScale = { chassisW - CHASSIS_PADDING - 0.4f, 0.1f, BUTTON_SIZE };
            // Correct texture relies on how we view it.
            // Camera is looking from +Z/+Y.
            // Top face is +Y.
            
            screenQuad.transform = MatrixScale(screenScale.x, screenScale.y, screenScale.z);
            // Fix: Raylib cube UVs might be flipped vertically.
            // ScreenTexture is rendered normally.
            
            // Draw the screen model
            Vector3 screenDrawPos = { screenPos3D.x, screenPos3D.y + 0.15f, screenPos3D.z };
            DrawModel(screenQuad, screenDrawPos, 1.0f, WHITE);
            
            
            // 3. Draw Buttons
            float startY_Grid = -gridH / 2.0f + BUTTON_SIZE; 
            
            for (int r = 0; r < GRID_ROWS; r++) {
                for (int c = 0; c < GRID_COLS; c++) {
                    if (strlen(buttonLabels[r][c]) == 0) continue;
                    
                    Vector3 pos = GetButtonPosition(r, c, gridW, gridH, startY_Grid);
                    pos.y += animOffset;
                    
                    // Press animation
                    if (buttonAnimStates[r][c].pressed) {
                        float p = buttonAnimStates[r][c].progress;
                        float dip = sinf(p * PI) * 0.15f;
                        pos.y -= dip;
                    }
                    
                    float w = BUTTON_SIZE;
                    if (r == GRID_ROWS - 1 && c == 0) w = BUTTON_SIZE * 2 + BUTTON_SPACING;
                    
                    // Scale the shared cube model
                    Vector3 btnScale = { w, BUTTON_HEIGHT, BUTTON_SIZE };
                    
                    // Apply texture
                    buttonModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = buttonTextures[r][c].texture;
                    
                    // Draw
                    // Note: LoadModelFromMesh(GenMeshCube) creates a model centered at 0.
                    // We need to scale it.
                    // Changing model.transform is persistent for the model, so we must reset or use DrawModelEx with scale.
                    // DrawModelEx(model, pos, rotationAxis, rotationAngle, scale, tint);
                    // But DrawModelEx applies uniform scale? No, it takes Vector3 scale.
                    
                    DrawModelEx(buttonModel, pos, (Vector3){0,1,0}, 0.0f, btnScale, WHITE);
                    
                    // Optional: Draw a wireframe or border for definition?
                    // DrawCubeWires(pos, w, BUTTON_HEIGHT, BUTTON_SIZE, Fade(BLACK, 0.3f));
                }
            }
            
        EndMode3D();
        
        // 2D Overlay (Instructions)
        DrawText("VibeCalc 3D", 10, 10, 20, RAYWHITE);
        DrawText("Use Keyboard or Mouse", 10, 35, 10, LIGHTGRAY);
        
        EndDrawing();
    }

    // Cleanup
    UnloadModel(buttonModel); // Unloads mesh too? Yes, usually.
    // Actually LoadModelFromMesh takes ownership of mesh.
    UnloadFont(appFont);
    UnloadRenderTexture(screenTexture);
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (strlen(buttonLabels[r][c]) > 0) UnloadRenderTexture(buttonTextures[r][c]);
        }
    }

    CloseWindow();
    return 0;
}
