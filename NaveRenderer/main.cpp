#include <iostream>
#include <stdexcept>
#include "obj_loader.h"
#include "renderer.h"

// -DHEADLESS permite generar la imagen sin instalar SDL.
#ifndef HEADLESS
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

int main(int argc,char* argv[]) {
    try {
        std::string path=argc>1?argv[1]:"Nave.obj";
        Mesh mesh=loadOBJ(path);
        assignParts(mesh);
        Framebuffer fb;
        constexpr float pi=3.14159265358979323846f;
        // CAMBIA ESTOS ANGULOS para elegir la vista inicial de tu nave.
        float yaw=305.0f*pi/180.0f,pitch=25.0f*pi/180.0f;
        std::cout<<mesh.vertices.size()<<" vertices, "<<mesh.triangles.size()<<" triangulos\n";
#ifdef HEADLESS
        render(mesh,fb,yaw,pitch);
        fb.savePPM(argc>2?argv[2]:"render.ppm");
        std::cout<<"Imagen guardada. Convierte el PPM a PNG con ppm_a_png.py\n";
#else
        SDL_SetMainReady();
        if(SDL_Init(SDL_INIT_VIDEO)!=0) throw std::runtime_error(SDL_GetError());
        SDL_Window* window=SDL_CreateWindow("Nave | Flechas: girar | R: reiniciar | S: guardar",
            SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer=window?SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC):nullptr;
        if(window && !renderer) renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
        SDL_Texture* texture=renderer?SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,SCREEN_WIDTH,SCREEN_HEIGHT):nullptr;
        if(!window || !renderer || !texture) {
            std::string error=SDL_GetError();
            if(texture) SDL_DestroyTexture(texture);
            if(renderer) SDL_DestroyRenderer(renderer);
            if(window) SDL_DestroyWindow(window);
            SDL_Quit();throw std::runtime_error(error);
        }
        bool running=true,dirty=true,save=false;
        while(running) {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                if(event.type==SDL_QUIT) running=false;
                if(event.type==SDL_KEYDOWN) {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:running=false;break;
                        case SDLK_LEFT:yaw-=0.08f;dirty=true;break;
                        case SDLK_RIGHT:yaw+=0.08f;dirty=true;break;
                        case SDLK_UP:pitch=std::min(1.45f,pitch+0.08f);dirty=true;break;
                        case SDLK_DOWN:pitch=std::max(-1.45f,pitch-0.08f);dirty=true;break;
                        case SDLK_r:yaw=305*pi/180;pitch=25*pi/180;dirty=true;break;
                        case SDLK_s:save=true;break;
                    }
                }
            }
            if(!running) break;
            if(dirty) {
                fb.clear();render(mesh,fb,yaw,pitch);
                SDL_UpdateTexture(texture,nullptr,fb.pixels.data(),SCREEN_WIDTH*sizeof(uint32_t));
                dirty=false;
            }
            if(save) {fb.savePPM("render.ppm");std::cout<<"Guardado: render.ppm\n";save=false;}
            SDL_RenderClear(renderer);SDL_RenderCopy(renderer,texture,nullptr,nullptr);
            SDL_RenderPresent(renderer);SDL_Delay(10);
        }
        SDL_DestroyTexture(texture);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
#endif
        return 0;
    } catch(const std::exception& e) {std::cerr<<"Error: "<<e.what()<<'\n';return 1;}
}
