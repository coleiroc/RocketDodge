#include "splashkit.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <ctime>

using namespace std;

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const string ROCKET_BITMAP = "rocket";
const string ASTEROID_BITMAP = "asteroid";

// Load resources
void load_resources() {
    load_bitmap(ROCKET_BITMAP, "images/rocket.gif");
    load_bitmap(ASTEROID_BITMAP, "images/asteroid.gif");
    load_sound_effect("background_music", "sounds/backgroundmusic.mp3");
    load_sound_effect("collision_sound", "sounds/explosionnoise.mp3");
}

// Rocket struct includes properties for the rocket
typedef struct {
    double x, y;
    double speed;
    bitmap rocket_bmp; // Bitmap for the rocket
} Rocket;

// Asteroid struct includes properties for asteroids
typedef struct {
    double x, y;
    double speed;
    bitmap asteroid_bmp; // Bitmap for the asteroid
} Asteroid;

// Particle struct for explosion effects
typedef struct {
    double x, y;     // Position of the particle
    double dx, dy;   // Velocity of the particle
    int lifespan;    // Lifespan of the particle
} Particle;

vector<Particle> particles; // Global list of particles

// Create particles at a given location
void create_particles(double x, double y) {
    int num_particles = 50; // Number of particles
    for (int i = 0; i < num_particles; i++) {
        Particle p;
        p.x = x;
        p.y = y;
        p.dx = rnd() * 6 - 3; // Increased range for bigger spread
        p.dy = rnd() * 6 - 3;
        p.lifespan = 60; // Lifespan of the particle
        particles.push_back(p);
    }
}

// Update all particles
void update_particles() {
    vector<Particle> updated_particles;
    for (Particle &p : particles) {
        p.x += p.dx;
        p.y += p.dy;
        p.lifespan--;
        if (p.lifespan > 0) {
            updated_particles.push_back(p);
        }
    }
    particles = updated_particles;
}

// Draw all particles
void draw_particles() {
    for (Particle &p : particles) {
        fill_circle(rgba_color(255, 200, 0, p.lifespan * 5), p.x, p.y, 3); // Range for explosion effect
    }
}

// Create a new rocket at the center of the screen
Rocket new_rocket() {
    Rocket result;
    result.x = SCREEN_WIDTH / 2;
    result.y = SCREEN_HEIGHT / 2;
    result.speed = 5;
    result.rocket_bmp = bitmap_named(ROCKET_BITMAP);
    return result;
}

// Create a new asteroid at a random position above the screen
Asteroid new_asteroid() {
    Asteroid a;
    a.x = rnd(SCREEN_WIDTH);
    a.y = -50; // Start off-screen above the top
    a.speed = 3 + rnd(5); // Random speed between 3 and 7
    a.asteroid_bmp = bitmap_named(ASTEROID_BITMAP);
    return a;
}

// Draw the rocket using its bitmap
void draw_rocket(const Rocket& rocket) {
    draw_bitmap(rocket.rocket_bmp, rocket.x - bitmap_width(rocket.rocket_bmp) / 2, rocket.y - bitmap_height(rocket.rocket_bmp) / 2);
}

// Draw an asteroid using its bitmap
void draw_asteroid(const Asteroid& asteroid) {
    draw_bitmap(asteroid.asteroid_bmp, asteroid.x - bitmap_width(asteroid.asteroid_bmp) / 2, asteroid.y - bitmap_height(asteroid.asteroid_bmp) / 2);
}

// Check circular collision detection with adjusted tighter radius
bool circular_collision(double x1, double y1, double radius1, double x2, double y2, double radius2) {
    double effective_radius1 = radius1 * 0.8; // Reduced radius for more accurate collision
    double effective_radius2 = radius2 * 0.8;
    double dx = x1 - x2;
    double dy = y1 - y2;
    double distance = sqrt(dx * dx + dy * dy);
    return distance < (effective_radius1 + effective_radius2);
}

// Update the rocket's position based on user input
void update_rocket(Rocket &rocket) {
    if (key_down(UP_KEY) && rocket.y > bitmap_height(rocket.rocket_bmp) / 2) {
        rocket.y -= rocket.speed;
    }
    if (key_down(DOWN_KEY) && rocket.y < SCREEN_HEIGHT - bitmap_height(rocket.rocket_bmp) / 2) {
        rocket.y += rocket.speed;
    }
    if (key_down(LEFT_KEY) && rocket.x > bitmap_width(rocket.rocket_bmp) / 2) {
        rocket.x -= rocket.speed;
    }
    if (key_down(RIGHT_KEY) && rocket.x < SCREEN_WIDTH - bitmap_width(rocket.rocket_bmp) / 2) {
        rocket.x += rocket.speed;
    }
}

// Update an asteroid's position
void update_asteroid(Asteroid &asteroid) {
    asteroid.y += asteroid.speed; // Move down the screen
    if (asteroid.y > SCREEN_HEIGHT + 50) {
        asteroid = new_asteroid();
    }
}

// Function to display the title screen and prompt the user to start the game
void show_title_screen(const string& player_name) {
    clear_screen(COLOR_BLACK);
    
    // Load the title GIF
    bitmap title_bmp = load_bitmap("title", "images/title.gif");

    // Load the Arial font from the program directory
    load_font("arial", "arial.ttf");

    // Position the title at the top of the screen
    int title_y = 50; 

    // Draw the title GIF at the center of the screen
    draw_bitmap(title_bmp, (SCREEN_WIDTH - bitmap_width(title_bmp)) / 2, title_y);
    
    // Draw the prompt and player's name 
    draw_text("Press Enter to Start", COLOR_WHITE, "arial", 36, (SCREEN_WIDTH - text_width("Press Enter to Start", "arial", 36)) / 2, SCREEN_HEIGHT / 2);
    draw_text("Player: " + player_name, COLOR_WHITE, "arial", 24, (SCREEN_WIDTH - text_width("Player: " + player_name, "arial", 24)) / 2, SCREEN_HEIGHT / 2 + 50);
    
    // Additional text
    draw_text("Avoid the Asteroids! They will begin falling shortly after the Game Starts. Good Luck!", COLOR_WHITE, "arial", 16, (SCREEN_WIDTH - text_width("Avoid the Asteroids! They will begin falling shortly after the Game Starts. Good Luck!", "arial", 16)) / 2, SCREEN_HEIGHT - 50);
    
    refresh_screen();
}

// Main game function
int main() {
    string player_name;

    // Get player's name from terminal
    cout << "Enter your name: ";
    getline(cin, player_name);

    open_window("Rocket Dodge", SCREEN_WIDTH, SCREEN_HEIGHT);
    load_resources();

    play_sound_effect("background_music", -1); // Play background music infinitely

    show_title_screen(player_name);

    // Wait for the Enter key to start the game
    while (!key_typed(RETURN_KEY)) {
        process_events();
    }

    Rocket rocket = new_rocket();
    vector<Asteroid> asteroids;
    double asteroid_spawn_timer = 0; // Track time for asteroid spawning

    // Start time for tracking elapsed time
    clock_t start_time = clock();
    clock_t current_time = start_time;
    double elapsed_time = 0;
    bool game_over = false;
    int score = 0;

    while (!quit_requested()) {
        process_events();
        if (key_typed(ESCAPE_KEY)) {
            break; // Break the loop and exit the game when ESC key is pressed
        }

        // Update rocket position
        update_rocket(rocket);

        // Calculate elapsed time
        current_time = clock();
        elapsed_time = (current_time - start_time) / (double)CLOCKS_PER_SEC;

        // Check if it's time to spawn a new asteroid
        asteroid_spawn_timer += elapsed_time;
        if (asteroid_spawn_timer >= 5) {
            asteroids.push_back(new_asteroid());
            asteroid_spawn_timer = 0; // Reset the timer
            // Increase speed of asteroids slightly
            for (Asteroid &asteroid : asteroids) {
                asteroid.speed += 0.5;
            }
        }

        // Update asteroid positions and check for collisions
        for (Asteroid &asteroid : asteroids) {
            update_asteroid(asteroid);
            if (circular_collision(rocket.x, rocket.y, bitmap_width(rocket.rocket_bmp) / 2, asteroid.x, asteroid.y, bitmap_width(asteroid.asteroid_bmp) / 2)) {
                create_particles(asteroid.x, asteroid.y);
                game_over = true; // Set game over flag
                play_sound_effect("collision_sound"); // Play collision sound effect
                break; // Exit the loop immediately after collision
            }
        }

        // If game over, show explosion animation and delay before transitioning to game over screen
        if (game_over) {
            // Show explosion animation
            while (!quit_requested()) {
                update_particles();
                clear_screen(COLOR_BLACK);
                draw_rocket(rocket);
                for (const Asteroid &asteroid : asteroids) {
                    draw_asteroid(asteroid);
                }
                draw_particles();
                refresh_screen(60);

                // Delay after explosion animation
                if (elapsed_time < 2) {
                    delay(100); // Reduce the delay time to allow frequent updates during explosion effect
                } else {
                    break; // Exit the loop after 2 seconds
                }

                // Calculate elapsed time within the loop
                current_time = clock();
                elapsed_time = (current_time - start_time) / (double)CLOCKS_PER_SEC;
            }

            // Show game over screen with score
            clear_screen(COLOR_BLACK);
            bitmap game_over_bmp = load_bitmap("gameover", "images/gameover.gif");
            draw_bitmap(game_over_bmp, (SCREEN_WIDTH - bitmap_width(game_over_bmp)) / 2, 50); // Position at the top
            draw_text("SCORE: " + to_string(asteroids.size()), COLOR_WHITE, "arial", 24, (SCREEN_WIDTH - text_width("SCORE: " + to_string(asteroids.size()), "arial", 24)) / 2, SCREEN_HEIGHT - 100); // Display score slightly upwards
            refresh_screen();
            delay(4000); // Delay for 4 seconds before exiting
            break; //Exit the game after 4 seconds
            

        }

        // Increment score for each frame without collision
        if (!game_over) {
            score++;
        }

        // Update particles
        update_particles();

        // Render game objects
        clear_screen(COLOR_BLACK);
        draw_rocket(rocket);
        for (const Asteroid &asteroid : asteroids) {
            draw_asteroid(asteroid);
        }
        draw_particles();
        refresh_screen(60);

        // Update start time for next iteration
        start_time = current_time;
    }

    return 0;
}
