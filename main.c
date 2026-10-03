#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// https://matthias-research.github.io/pages/tenMinutePhysics/18-flip.pdf

#define FPS 60
// Initial window setup
#define X 640
#define Y 640

#define CELL_FACTOR 20 // How many pixels make up one cell
#define CELL_COUNT ((X * Y) / CELL_FACTOR) // Total amount of cells
#define CELL_TYPE_COUNT 3 // Count of individual cell types for later expansion

#define PARTICLES_MAX 2000
#define PARTICLES_INITIAL_COUNT 500 // Initial amount of particles
#define PARTICLE_INITIAL_VELOCITY 10 // Maximum amount of initial velocity given to a particle
#define PARTICLE_FRICTION_VECTOR \
    (Vector2) { 0.9, 0.9 } // Friction vector applied every tick
#define PARTICLE_GRAVITY_VECTOR \
    (Vector2) { 0, 2 } // Gravity vector applied every tick
#define PARTICLE_WATER_WEIGHT 1.0f // Weight of a water particle
#define PARTICLE_CELL_WEIGHT 1.0f // PIC to FLIP ratio

typedef struct Cell Cell;

// Water particles independant of the grid
typedef struct {
    bool enabled; // Wether the particle actually exists or not
    Vector2 pos;
    Vector2 vel;
    Cell* cell; // The cell the particle is currently in
} Particle;

// Collection of a cells velocities at it's faces
typedef struct {
    double top;
    double bottom;
    double left;
    double right;
} CellVelocity;

// Grid-like cell containing particles
typedef struct Cell {
    uint8_t type; // 0 == Air; 1 == Water; 2 == Wall

    // As we're in a grid we only need whole numbers
    size_t x;
    size_t y;
    CellVelocity cvel;
    size_t particle_count; // Amount of particles in this cell
} Cell;

Color colors[] = { WHITE, BLUE, BLACK }; // Color mappings for the different cell types

Cell cells[CELL_COUNT]; // Flat 2D array of cells
Particle particles[PARTICLES_MAX]; // Array of particles

// Returns the first disabled particle of the particles array
Particle* getDisabledParticle(void)
{
    for (size_t i = 0; i < PARTICLES_MAX; i++)
        if (!particles[i].enabled)
            return &particles[i];
    return NULL;
}

Vector2 getRandomVector2(int x_min, int x_max, int y_min, int y_max)
{
    return (Vector2) { GetRandomValue(x_min, x_max), GetRandomValue(y_min, y_max) };
}

void initParticles(void)
{
    for (size_t i = 0; i < PARTICLES_INITIAL_COUNT; i++) {
        particles[i] = (Particle) { true, getRandomVector2(0, X, 0, Y), getRandomVector2(-PARTICLE_INITIAL_VELOCITY, PARTICLE_INITIAL_VELOCITY, -PARTICLE_INITIAL_VELOCITY, PARTICLE_INITIAL_VELOCITY) };
    }
}

void clearCellVelocity(CellVelocity* cvel)
{
    cvel->top = 0;
    cvel->bottom = 0;
    cvel->left = 0;
    cvel->right = 0;
}

Cell* getCellOfParticle(Particle* particle)
{
    if (!particle->enabled)
        return NULL;
    size_t cell_index = (particle->pos.x + (particle->pos.y * Y)) / CELL_FACTOR;
    if (cell_index > CELL_COUNT) {
        printf("Out of bounds particle\n");
        return NULL;
    }
    return &cells[cell_index];
}

void transferVelocityToParticles(Particle* particle)
{
    if (!particle->enabled)
        return;
}

void transferVelocityToGrid(Particle* particle)
{
    Cell* cell = particle->cell;
    if (cell == NULL)
        return;
    CellVelocity* cvel = &cell->cvel;
    double x = particle->pos.x - (cell->x * CELL_FACTOR);
    double y = particle->pos.y - (cell->y * CELL_FACTOR);
}

void updateParticles(void)
{
    Particle* particle;
    for (size_t i = 0; i < PARTICLES_MAX; i++) {
        particle = &particles[i];
        if (!particle->enabled)
            continue;
        particle->cell = getCellOfParticle(particle);
        if (particle->cell != NULL)
            particle->cell->particle_count++;
        particle->pos = Vector2Add(particle->pos, particle->vel);
        particle->vel = Vector2Multiply(particle->vel, PARTICLE_FRICTION_VECTOR);
        particle->vel = Vector2Add(particle->vel, PARTICLE_GRAVITY_VECTOR);
        transferVelocityToGrid(particle);
        if (particle->cell != NULL)
            particle->cell->type = 1;
    }
}

void updateCells(void)
{
    Cell* cell;
    for (size_t i = 0; i < CELL_COUNT; i++) {
        cell = &cells[i];
        cell->type = 0;
        cell->particle_count = 0;
        clearCellVelocity(&cell->cvel);
    }
}

void initCells(void)
{
    for (size_t i = 0; i < CELL_COUNT; i++) {
        cells[i] = (Cell) { 0, i % X, i / X };
    }
}

void drawCells(void)
{
    Cell* cell;
    for (size_t i = 0; i < CELL_COUNT; i++) {
        cell = &cells[i];
        DrawRectangle((i % X) * CELL_FACTOR, (i / X) * CELL_FACTOR, CELL_FACTOR, CELL_FACTOR, colors[cell->type]);
        // DrawPixel(cell->x, cell->y, colors[cells[i].type]);
    }
}

void tick(void)
{
    updateCells();
    updateParticles();
    drawCells();
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(X, Y, "GyroFlip");
    SetTargetFPS(FPS);
    SetRandomSeed(time(NULL));

    initCells();
    initParticles();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GRAY);
        tick();
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return EXIT_SUCCESS;
}
