#include <SDL2/SDL.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <thread>
#include <iostream>

//Screen dimension
const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 480;

int maxInterations = 100;
double middleX = -0.75; // x offset
double middleY = 0;     // y offset
double rangeX = 3.5;
double rangeY = 2;

const int numThreads = 32;

std::thread threads[numThreads];

Uint64 NOW = SDL_GetPerformanceCounter();
Uint64 LAST = 0;
double deltaTime = 0;

bool init();

void close();

SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

bool init()
{
    bool success = true;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }

        gWindow = SDL_CreateWindow( "Mandelbrotmenge", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
            }
        }
    }

    return success;
}
void close()
{
    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    SDL_Quit();
}

double colors[SCREEN_WIDTH][SCREEN_HEIGHT];

void work(int n)
{
    for (int x = n; x<SCREEN_WIDTH; x+=numThreads)
    {
        for(int y = 0; y<SCREEN_HEIGHT; y++)
        {
            double xPer = (double)x/(double)SCREEN_WIDTH;
            double yPer = (double)y/(double)SCREEN_HEIGHT;


            double cReal = xPer * rangeX + middleX - rangeX / 2;
            double cImaginary = yPer * rangeY + middleY - rangeY / 2;

            double zReal = 0;
            double zImaginary = 0;

            int iteration = 0;

            while(iteration<maxInterations && zReal*zReal + zImaginary * zImaginary <= 4) {
                double oldZ = zReal;
                zReal = zReal * zReal -zImaginary * zImaginary + cReal;
                zImaginary = 2 * oldZ * zImaginary + cImaginary;
                iteration++;
            }
            colors[x][y] = (double)iteration / (double)maxInterations;
        }

    }
}

int main( int argc, char* args[] )
{
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
            bool quit = false;

            SDL_Event e;

            while( !quit )
            {

                LAST = NOW;
                NOW = SDL_GetPerformanceCounter();

                deltaTime = (double)((NOW - LAST)*1000 / (double)SDL_GetPerformanceFrequency() ) / 1000;

                while( SDL_PollEvent( &e ) != 0 )
                {
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }else if (e.type == SDL_MOUSEWHEEL)
                    {
                        if( e.wheel.y == 1)
                        {
                            rangeX *= 0.8;
                            rangeY *= 0.8;
                        }else
                        {
                            rangeX /= 0.8;
                            rangeY /= 0.8;
                        }
                    }else if(e.type == SDL_KEYDOWN) {

                        if(e.key.keysym.sym == SDLK_w)
                        {
                            middleY -= 0.15 * deltaTime * rangeY;
                        }
                        else if(e.key.keysym.sym == SDLK_s)
                        {
                            middleY += 0.15 * deltaTime * rangeY;
                        }
                        else if(e.key.keysym.sym == SDLK_a)
                        {
                            middleX -= 0.15 * deltaTime * rangeX;
                        }
                        else if(e.key.keysym.sym == SDLK_d)
                        {
                            middleX += 0.15 * deltaTime * rangeX;
                        }
                        else if(e.key.keysym.sym == SDLK_UP)
                        {
                            maxInterations=maxInterations * 1.1 + 1;
                        }
                        else if(e.key.keysym.sym == SDLK_DOWN)
                        {
                            maxInterations= maxInterations / 1.1 + 1;
                        }
                    }
                }

                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 0xFF );
                SDL_RenderClear( gRenderer );

                for(int i = 0; i<numThreads; i++)
                {
                    threads[i] = std::thread([i](){work(i);});
                }

                for(int i = 0; i<numThreads; i++) {
                    threads[i].join();
                }

                for(int x = 0; x<SCREEN_WIDTH; x++)
                {
                    for (int y = 0; y<SCREEN_HEIGHT; y++)
                    {

                        SDL_SetRenderDrawColor( gRenderer, colors[x][y] * 255 / 2, colors[x][y] * 255 / 0.1, colors[x][y] * 255 / 2.5, 255 );

                        SDL_RenderDrawPoint(gRenderer, x, y);
                    }
                }

                //Update screen
                SDL_RenderPresent( gRenderer );
            }
    }

    close();

    return 0;
}