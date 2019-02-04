

#include "SDL.h"

#ifdef _WIN32
#include "SDL_image.h"
	#include "SDL_ttf.h"
	#include "SDL_mixer.h"
#else
#include "../SDL2_ttf/SDL_ttf.h"
#include "../SDL2_mixer/SDL_mixer.h"
#include "../SDL2_image/SDL_image.h"

#include <jni.h>
#endif

#include <string>
#include <vector>
#include <time.h>
#include <iostream>

#define	IMG_DIR "images"
#define SND_DIR "sounds"
#define FNT_DIR "fonts"

#define SCREEN_WIDTH 	480
#define SCREEN_HEIGHT   800

const SDL_Color GRAY 		= {.r = 127  , .g = 127   , .b = 127   , .a = 255};
const SDL_Color SILVER		= {.r = 192  , .g = 192   , .b = 192   , .a = 255};
const SDL_Color RED			= {.r = 255  , .g = 0     , .b = 0     , .a = 255};
const SDL_Color GREEN		= {.r = 0    , .g = 255   , .b = 0     , .a = 255};
const SDL_Color BLUE		= {.r = 0    , .g = 0     , .b = 255   , .a = 255};
const SDL_Color YELLOW		= {.r = 255  , .g = 255   , .b = 0     , .a = 255};
const SDL_Color CYAN		= {.r = 0    , .g = 255   , .b = 255   , .a = 255};
const SDL_Color MAGENTA		= {.r = 255  , .g = 0     , .b = 255   , .a = 255};
const SDL_Color BLACK		= {.r = 0    , .g = 0     , .b = 0     , .a = 255};
const SDL_Color WHITE		= {.r = 255  , .g = 255   , .b = 255   , .a = 255};
const SDL_Color DARK_RED	= {.r = 100  , .g = 0     , .b = 0     , .a = 255};
const SDL_Color DARK_GREEN	= {.r = 0    , .g = 100   , .b = 0     , .a = 255};
const SDL_Color DARK_BLUE	= {.r = 0    , .g = 0     , .b = 100   , .a = 255};
const SDL_Color DARK_YELLOW	= {.r = 100  , .g = 100   , .b = 0     , .a = 255};
const SDL_Color DARK_CYAN	= {.r = 0    , .g = 100   , .b = 100   , .a = 255};
const SDL_Color DARK_MAGENTA= {.r = 100  , .g = 0     , .b = 100   , .a = 255};
const SDL_Color PURPLE		= {.r = 85   , .g = 26    , .b = 139   , .a = 255};
const SDL_Color GOLDEN		= {.r = 255  , .g = 215   , .b = 0     , .a = 255};

class helper {
public:
    static std::string int2string(int value);
    static SDL_Surface * load_image(std::string fileName, std::string path);
    static TTF_Font * 	 load_text_font(std::string fileName, std::string path, int tamanyo);
    static Mix_Music *	 load_sound(std::string fileName, std::string path);
#ifndef _WIN32
    static void doVibrateFromJNI(int duration);
#endif
};

std::string helper::int2string(int value) {
    char buffer[20];
    sprintf(buffer,"%d", value);
    return std::string(buffer);
}

#ifndef _WIN32
void helper::doVibrateFromJNI(int duration) {
    // retrieve the JNI environment.
    JNIEnv * javaEnviron = (JNIEnv *)SDL_AndroidGetJNIEnv();
    if (javaEnviron != NULL) {
        jclass clazz = javaEnviron->FindClass("com/game/buscaminas/BuscaMinasActivity");
        if (clazz == 0) {
            SDL_Log("Error JNI: Class com/game/buscaminas/BuscaMinasActivity not found!");
            return;
        }
        jmethodID method_id = javaEnviron->GetStaticMethodID(clazz, "doVibrate", "(I)V");
        if (method_id == 0) {
            SDL_Log("Error JNI: methodId is 0, method doVibrate (I)V not found!");
            return;
        }
        // Call Java-method
        javaEnviron->CallStaticVoidMethod(clazz, method_id, (jint)duration);
        javaEnviron->DeleteLocalRef(clazz);
    }
}
#endif

SDL_Surface * helper::load_image(std::string fileName, std::string path) {
#ifdef _WIN32
    std::string imagePath = path + "\\" + fileName;
#else
    std::string imagePath = path + "/" + fileName;
#endif
//	SDL_Surface *image = SDL_LoadBMP(imagePath.c_str());
//	if (image == nullptr)
//		SDL_Log("SDL_LoadBMP Error: %s\r\n", SDL_GetError());
//
    SDL_Surface *image = IMG_Load(imagePath.c_str());
    if (image == NULL)
        SDL_Log("Unable to load image %s! SDL_image Error: %s\n", imagePath.c_str(), IMG_GetError());
    return image;
}

TTF_Font * helper::load_text_font(std::string fileName, std::string path, int tamanyo) {
#ifdef _WIN32
    std::string fontPath = path + "\\" + fileName;
#else
    std::string fontPath = path + "/" + fileName;
#endif
    TTF_Font * font = TTF_OpenFont(fontPath.c_str(), tamanyo);
    if (font == NULL)
        SDL_Log("TTF_OpenFont Error: %s\r\n", TTF_GetError());
    return font;
}

Mix_Music * helper::load_sound(std::string fileName, std::string path) {
#ifdef _WIN32
    std::string musicPath = path + "\\" + fileName;
#else
    std::string musicPath = path + "/" + fileName;
#endif
    Mix_Music * music = Mix_LoadMUS(musicPath.c_str());
    if (music == NULL)
        SDL_Log("Mix_LoadMUS Error: %s\r\n", Mix_GetError());
    return music;
}

class Casilla {
public:
    Casilla(int x, int y, int size_x, int size_y, TTF_Font * text_font, int status, char value);
    ~Casilla(){};
    SDL_bool is_touched(int x, int y);
    void set_status(int status);
    int get_status(void);
    void plot(SDL_Renderer * screen);
private:
    int x, y, size_x, size_y, status;
    char value;
    TTF_Font * text_font;
    SDL_Color text_color[9];
};

Casilla::Casilla(int x, int y, int size_x, int size_y, TTF_Font * text_font, int status, char value) {
    text_color[0] = GRAY;
    text_color[1] = RED;
    text_color[2] = GREEN;
    text_color[3] = YELLOW;
    text_color[4] = CYAN;
    text_color[5] = MAGENTA;
    text_color[6] = PURPLE;
    text_color[7] = GOLDEN;
    text_color[8] = WHITE; // Color del valor del texto segun valor de la casilla
    this->x = x;
    this->y = y;
    this->size_x = size_x;
    this->size_y = size_y;
    this->status = status; // Estado de la casilla UP, DOWN, NAKED
    this->value = value; // Valor de la casilla ' ', '0'...'8' bomba
    this->text_font = text_font;
}

SDL_bool Casilla::is_touched(int x, int y) {
    SDL_bool retorno = SDL_FALSE;
    if ((x >= this->x) && (x <= this->x + this->size_x) && (y > this->y) && (y < this->y + this->size_y))
        retorno = SDL_TRUE;
    return retorno;
}

void Casilla::set_status(int status) {
    this->status = status;
}

int Casilla::get_status(void) {
    return this->status;
}

void Casilla::plot(SDL_Renderer * screen) {
    SDL_Surface * surface = NULL;
    SDL_Texture * texture = NULL;
    SDL_Rect box;
    SDL_Rect dst;
    int w, h;
    std::string sValue = std::string() + this->value;
    switch(this->status) {
        case 0x01: // UP
            SDL_SetRenderDrawColor(screen, 192  , 192   , 192   , 255); // SILVER
            box.x = this->x+1;
            box.y = this->y+1;
            box.w = this->size_x-1;
            box.h = this->size_y-1;
            SDL_RenderFillRect(screen, &box);
            SDL_SetRenderDrawColor(screen, 255  , 255   , 255   , 255); // WHITE
            SDL_RenderDrawLine(screen, this->x, this->y, this->x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y, this->x + this->size_x, this->y);
            SDL_SetRenderDrawColor(screen, 127  , 127   , 127   , 255); // GREY
            SDL_RenderDrawLine(screen, this->x + this->size_x, this->y, this->x + this->size_x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y + this->size_y, this->x + this->size_x, this->y + this->size_y);
            break;
        case 0x02: // DOWN
            SDL_SetRenderDrawColor(screen, 192  , 192   , 192   , 255); // SILVER
            box.x = this->x+1;
            box.y = this->y+1;
            box.w = this->size_x-1;
            box.h = this->size_y-1;
            SDL_RenderFillRect(screen, &box);
            SDL_SetRenderDrawColor(screen, 255  , 255   , 255   , 255); // WHITE
            SDL_RenderDrawLine(screen, this->x + this->size_x, this->y, this->x + this->size_x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y + this->size_y, this->x + this->size_x, this->y + this->size_y);
            SDL_SetRenderDrawColor(screen, 127  , 127   , 127   , 255);	// GREY
            SDL_RenderDrawLine(screen, this->x, this->y, this->x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y, this->x + this->size_x, this->y);
            break;
        case 0x03: // NAKED
            if (this->value == 'x') {
                SDL_SetRenderDrawColor(screen, 255  , 255   , 255   , 255); // WHITE
                box.x = this->x;
                box.y = this->y;
                box.w = this->size_x+1;
                box.h = this->size_y+1;
                SDL_RenderFillRect(screen, &box);
                surface = helper::load_image("mina.png",IMG_DIR);
                texture = SDL_CreateTextureFromSurface(screen, surface);
                SDL_FreeSurface(surface);
                dst.x = this->x;
                dst.y = this->y;
                dst.w = this->size_x;
                dst.h = this->size_y;
                SDL_RenderCopy(screen, texture, NULL, &dst);
/*
                SDL_SetRenderDrawColor(screen, 255  , 0     , 0     , 255); // RED
                box.x = this->x+1;
                box.y = this->y+1;
                box.w = this->size_x-1;
                box.h = this->size_y-1;
                SDL_RenderFillRect(screen, &box);
                SDL_SetRenderDrawColor(screen, 255  , 255   , 255   , 255); // WHITE
                SDL_RenderDrawLine(screen, this->x, this->y, this->x, this->y + this->size_y);
                SDL_RenderDrawLine(screen, this->x, this->y, this->x + this->size_x, this->y);
                SDL_SetRenderDrawColor(screen, 127  , 127   , 127   , 255); // GREY
                SDL_RenderDrawLine(screen, this->x + this->size_x, this->y, this->x + this->size_x, this->y + this->size_y);
                SDL_RenderDrawLine(screen, this->x, this->y + this->size_y, this->x + this->size_x, this->y + this->size_y);
*/
            } else {
                SDL_SetRenderDrawColor(screen, 127  , 127   , 127   , 255); // GREY
                box.x = this->x;
                box.y = this->y;
                box.w = this->size_x+1;
                box.h = this->size_y+1;
                SDL_RenderFillRect(screen, &box);
                surface = TTF_RenderText_Solid(this->text_font, sValue.c_str(), this->text_color[this->value-48]);
                texture = SDL_CreateTextureFromSurface(screen, surface);
                SDL_FreeSurface(surface);
                SDL_QueryTexture(texture, NULL, NULL, &w, &h);
                dst.x = (this->x + (this->size_x / 2) - (w/2));
                dst.y = (this->y + (this->size_y / 2) - (h/2));
                dst.w = w;
                dst.h = h;
                SDL_RenderCopy(screen, texture, NULL, &dst);
            }
            break;
        case 0x04: // MARKED
            SDL_SetRenderDrawColor(screen, 192  , 192   , 192   , 255); // SILVER
            box.x = this->x+1;
            box.y = this->y+1;
            box.w = this->size_x-1;
            box.h = this->size_y-1;
            SDL_RenderFillRect(screen, &box);
            SDL_SetRenderDrawColor(screen, 255  , 255   , 255   , 255); // WHITE
            SDL_RenderDrawLine(screen, this->x, this->y, this->x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y, this->x + this->size_x, this->y);
            SDL_SetRenderDrawColor(screen, 127  , 127   , 127   , 255); // GREY
            SDL_RenderDrawLine(screen, this->x + this->size_x, this->y, this->x + this->size_x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y + this->size_y, this->x + this->size_x, this->y + this->size_y);
            surface = helper::load_image("bandera.png",IMG_DIR);
            texture = SDL_CreateTextureFromSurface(screen, surface);
            SDL_FreeSurface(surface);
            dst.x = this->x;
            dst.y = this->y;
            dst.w = this->size_x;
            dst.h = this->size_y;
            SDL_RenderCopy(screen, texture, NULL, &dst);
/*
            SDL_SetRenderDrawColor(screen, 255  , 215   , 0     , 255); // GOLDEN
            box.x = this->x+1;
            box.y = this->y+1;
            box.w = this->size_x-1;
            box.h = this->size_y-1;
            SDL_RenderFillRect(screen, &box);
            SDL_SetRenderDrawColor(screen, 255  , 255   , 255   , 255); // WHITE
            SDL_RenderDrawLine(screen, this->x, this->y, this->x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y, this->x + this->size_x, this->y);
            SDL_SetRenderDrawColor(screen, 127  , 127   , 127   , 255); // GREY
            SDL_RenderDrawLine(screen, this->x + this->size_x, this->y, this->x + this->size_x, this->y + this->size_y);
            SDL_RenderDrawLine(screen, this->x, this->y + this->size_y, this->x + this->size_x, this->y + this->size_y);
*/
            break;
    }
    if (texture != NULL) SDL_DestroyTexture(texture);
}

class Tablero {
public:
    Tablero(int max_x, int max_y, int factor, int width, int height);
    ~Tablero();
    int get_num_masked_items(void);
    int get_num_bombs(void);
    SDL_bool destapa(int x, int y);
    void print_tablero(SDL_Renderer * screen);
    SDL_bool play(SDL_Renderer * screen, SDL_Event & event);
private:
    int width, height, x0, y0, xx, yy, start, max_x, max_y;
    std::vector<char> tabla;
    std::vector<Casilla*> casillas;
    SDL_bool pushed;
    TTF_Font * font;
};

Tablero::Tablero(int max_x, int max_y, int factor, int width, int height) {
    this->width 	= (width/max_x);
    this->height 	= (height/max_y);
    this->x0 		= this->y0 = 255;
    this->xx 		= this->yy = 10;
    this->pushed 	= SDL_FALSE;
    this->start  	= 0;
    this->max_x 	= max_x;
    this->max_y 	= max_y;
    srand(time(NULL));
    for (int x = 0; x < max_x; x++)
        for (int y = 0; y < max_y; y++)
            if (rand() % 100 < factor)
                this->tabla.push_back('x');
            else
                this->tabla.push_back('0');
    int c = 0;
    for (int y = 0; y < this->max_y; y++) {
        for (int x = 0; x < this->max_x; x++) {
            if (this->tabla[y*max_x+x] == '0') {
                if ( (y > 0) and (this->tabla[(y-1)*max_x+x] == 'x') ) 							c = c + 1;
                if ( (y < max_y-1) and (this->tabla[(y+1)*max_x+x] == 'x') ) 					c = c + 1;
                if ( (x > 0) and (this->tabla[y*max_x+x-1] == 'x') ) 							c = c + 1;
                if ( (x < max_x-1) and (this->tabla[y*max_x+x+1] == 'x') )						c = c + 1;
                if ( (y > 0) and (x > 0) and (this->tabla[(y-1)*max_x+x-1] == 'x') )			c = c + 1;
                if ( (y < max_y-1) and (x < max_x-1) and (this->tabla[(y+1)*max_x+x+1] == 'x') )c = c + 1;
                if ( (y > 0) and (x < max_x-1) and (this->tabla[(y-1)*max_x+x+1] == 'x') )		c = c + 1;
                if ( (y < max_y-1) and  (x > 0) and (this->tabla[(y+1)*max_x+x-1] == 'x') ) 	c = c + 1;
                this->tabla[y*max_x+x] = c + 48;
                c = 0;
            }
            //std::cout << this->tabla[y*max_x+x] << " ";
        }
        //std::cout << std::endl;
    }
    this->font = helper::load_text_font("DroidSans-Bold.ttf", FNT_DIR, width/max_x);
    // Crea Array de objetos casilla en UP con el valor de tabla
    for (int y = 0; y < this->max_y; y++)
        for (int x = 0; x < this->max_x; x++)
            this->casillas.push_back(new Casilla(x*this->width+1, y*this->height+1, this->width-1, this->height-1, this->font, 1, this->tabla[y*max_x+x]));
}

Tablero::~Tablero() {
    for (int c = 0; c < this->max_x * this->max_y; c++)
        if (this->casillas[c] != NULL)
            delete(this->casillas[c]);
    TTF_CloseFont(this->font);
}

int Tablero::get_num_masked_items(void) {
    int retorno = 0;
    for (int c = 0; c < this->max_x * this->max_y; c++)
        if (this->casillas[c]->get_status() != 3)
            retorno++;
    return retorno;
}

int Tablero::get_num_bombs(void) {
    int retorno = 0;
    for (int c = 0; c < this->max_x * this->max_y; c++)
        if (this->tabla[c] == 'x')
            retorno++;
    return retorno;
}

SDL_bool Tablero::destapa(int x, int y) {
    SDL_bool retorno = SDL_TRUE;
    this->casillas[y*this->max_x+x]->set_status(3);
    if (this->tabla[y*this->max_x+x] != 'x') {
        retorno = SDL_FALSE;
        if (this->tabla[y*this->max_x+x] == '0')
            for (int j = -1; j < 2; j++)
                for (int i = -1; i < 2; i++)
                    if ((x + j >= 0) && (x + j < this->max_x) && (y + i >= 0) && (y + i < this->max_y))
                        if (this->casillas[(y+i)*this->max_x+x+j]->get_status() != 3)
                            this->destapa(x+j, y+i);
    }
    return retorno;
}

void Tablero::print_tablero(SDL_Renderer * screen) {
    for (int y = 0; y < this->max_y; y++)
        for (int x = 0; x < this->max_x; x++)
            this->casillas[y*this->max_x+x]->plot(screen);
}

SDL_bool Tablero::play(SDL_Renderer * screen, SDL_Event & event) {
    SDL_bool retorno = SDL_FALSE;
    int x, y;
#ifndef _WIN32
    int w, h;
    SDL_GetRendererOutputSize(screen, &w, &h);
#endif
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            SDL_Log("TODO: Salir\r\n");
            return SDL_TRUE;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            //Window resize/orientation change
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)	{
                //w = e.window.data1; h = e.window.data2;
                SDL_RenderPresent(screen);
            }
        }
        else if ((event.type == SDL_FINGERDOWN) || (event.type == SDL_MOUSEBUTTONDOWN)) {
#ifdef _WIN32
            this->xx = event.button.x;
			this->yy = event.button.y;
#else
            this->xx = (event.tfinger.x * w);
            this->yy = (event.tfinger.y * h);
#endif
            if ((this->xx > 0) && (this->yy > 0) && (this->xx < this->width*this->max_x) && (this->yy < this->height*this->max_y)) {
                this->xx /= this->width;
                this->yy /= this->height;
                this->x0 = this->xx;
                this->y0 = this->yy;

                if (this->casillas[this->yy * this->max_x + this->xx]->get_status() < 3)
                    this->casillas[this->yy * this->max_x + this->xx]->set_status(2);
                this->pushed = SDL_TRUE;
                this->start = SDL_GetTicks();
            }
        }
        else if ((event.type == SDL_FINGERMOTION) || (event.type == SDL_MOUSEMOTION)) {
#ifdef _WIN32
            SDL_GetMouseState(&x, &y);
#else
            x = (event.tfinger.x * w);
            y = (event.tfinger.y * h);
#endif
            if ((x > 0) && (y > 0) && (x < this->width*this->max_x) && (y < this->height*this->max_y)) {
                x /= this->width;
                y /= this->height;
                if (this->pushed == SDL_TRUE) {
                    if ((this->xx !=  x) || (this->yy != y)) {
                        if (this->casillas[this->yy*this->max_x+this->xx]->get_status() < 3)
                            this->casillas[this->yy*this->max_x+this->xx]->set_status(1);
                        if (this->casillas[y*this->max_x+x]->get_status() < 3)
                            this->casillas[y*this->max_x+x]->set_status(2);
                        this->xx = x;
                        this->yy = y;
                    }
                }
            }
        }
        else if ((event.type == SDL_FINGERUP) || (event.type == SDL_MOUSEBUTTONUP)) {
#ifdef _WIN32
                x = event.button.x;
				y = event.button.y;
#else
                x = (event.tfinger.x * w);
                y = (event.tfinger.y * h);
#endif
            if ((x > 0) && (y > 0) && (x < this->width*this->max_x) && (y < this->height*this->max_y)) {
                if (this->pushed == SDL_TRUE) {
                    this->pushed = SDL_FALSE;
                    x /= this->width;
                    y /= this->height;
                    if ((this->x0 == this->xx) && (this->y0 == this->yy) && (this->casillas[this->y0*this->max_x+this->x0]->get_status() < 3))
                        retorno = this->destapa(this->x0, this->y0);
                    else
                        if (this->casillas[y*this->max_x+x]->get_status() == 2)
                            this->casillas[y*this->max_x+x]->set_status(1);
                }
            }
        }
    }
    if (this->pushed == SDL_TRUE) {
#ifdef _WIN32
        SDL_GetMouseState(&x, &y);
#else
        x = (event.tfinger.x * w);
        y = (event.tfinger.y * h);
#endif
        if ((x > 0) && (y > 0) && (x < this->width*this->max_x) && (y < this->height*this->max_y)) {
            x /= this->width;
            y /= this->height;
            if ((this->x0 == x) && (this->y0 == y) && ((SDL_GetTicks() - this->start) > 1000) &&
                (this->casillas[y * this->max_x + x]->get_status() != 3)) {
                this->pushed = SDL_FALSE;
                if (this->casillas[y * this->max_x + x]->get_status() == 4)
                    this->casillas[y * this->max_x + x]->set_status(1);
                else
                    this->casillas[y * this->max_x + x]->set_status(4);
            }
        }
    }
    this->print_tablero(screen);
    return retorno;
}

int main(int argc, char *argv[]) {
    Mix_Music * music = NULL;
    SDL_Renderer * screen = NULL;
    SDL_Window * window = NULL;
    SDL_DisplayMode displayMode;
    SDL_bool salir;
    int clock, diff;
    int imgFlags, mixFlags;
    int width, height, w, h;
    TTF_Font * font = NULL;
    SDL_Surface * textMessage = NULL;
    SDL_Texture * texture = NULL;
    SDL_Rect dst;
    SDL_Event event;
    //Initialize SDL
    if (SDL_Init( SDL_INIT_EVERYTHING ) < 0) {
        SDL_Log("SDL could not initialize! SDL Error: %s\r\n", SDL_GetError());
    } else {
#ifndef _WIN32
        if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
            SDL_Log("SDL Mixer could not initialize! Mix Error: %s\r\n", Mix_GetError());
        } else {
#endif
            // Initialize PNG loading
            imgFlags = IMG_INIT_PNG;
            if (!(IMG_Init( imgFlags ) & imgFlags)) {
                SDL_Log("SDL_image could not initialize! %s\r\n", IMG_GetError());
            } else {
                //Initialize SDL_ttf
                if (TTF_Init() < 0) {
                    SDL_Log("TTF could not initialize! TTF Error: %s\r\n", TTF_GetError());
                } else {
                    //Initialize SDL_mixer
                    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
                        SDL_Log("MIX could not initialize! MIX Error: %s\r\n", Mix_GetError());
                    } else {
                        //Get device display mode
#ifdef _WIN32
                        width = SCREEN_WIDTH;
                        height = SCREEN_HEIGHT;
#else
                        if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
                            width = displayMode.w;
                            height = displayMode.h;
                        } else {
                            width = SCREEN_WIDTH;
                            height = SCREEN_HEIGHT;
                        }
#endif
                        // Create window
                        window = SDL_CreateWindow("BUSCA MINAS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
                        if (window == NULL) {
                            SDL_Log("Window could not be created! SDL Error: %s\r\n", SDL_GetError());
                        } else {
                            //Create renderer for window
                            screen = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                            if (screen == NULL) {
                                SDL_Log("Renderer could not be created! SDL Error: %s\r\n", SDL_GetError());
                            } else {
                                //Initialize renderer color
                                SDL_SetRenderDrawColor(screen, 0xFF, 0xFF, 0xFF, 0xFF);
                                music = helper::load_sound("7batldun.ogg", SND_DIR);
                                if (music != NULL) Mix_PlayMusic(music, -1);
                                // Creamos los objetos del juego
                                Tablero *tablero = new Tablero(12, 20, 18, width, height);
                                if (tablero != NULL) {
                                    salir = SDL_FALSE;
                                    clock = SDL_GetTicks();
                                    while (salir == SDL_FALSE) {
                                        salir = tablero->play(screen, event);
                                        SDL_RenderPresent(screen);
                                        diff = SDL_GetTicks() - clock;
                                        if (diff < 60) {
                                            SDL_Delay(60 - diff);
                                            clock = SDL_GetTicks();
                                        }
#ifndef _WIN32
                                        SDL_RenderClear(screen);
#endif
                                        if (tablero->get_num_masked_items() == tablero->get_num_bombs())
                                            salir = SDL_TRUE;
                                    }
#ifdef _WIN32
                                    font = helper::load_text_font("B.ttf", FNT_DIR, 80);
#else
                                    SDL_Delay(1000);
                                    font = helper::load_text_font("B.ttf", FNT_DIR, 160);
#endif

                                    if (tablero->get_num_masked_items() == tablero->get_num_bombs()) {
                                        textMessage = TTF_RenderText_Solid(font, "YOU WIN", GREEN);
                                    } else {
                                        textMessage = TTF_RenderText_Solid(font, "YOU LOSE", RED);
                                    }
                                    texture = SDL_CreateTextureFromSurface(screen, textMessage);
                                    SDL_FreeSurface(textMessage);
                                    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
                                    dst.x = (width / 2) - (w / 2);
                                    dst.y = (height / 2) - (h / 2);
                                    dst.w = w;
                                    dst.h = h;
                                    SDL_RenderCopy(screen, texture, NULL, &dst);
                                    // --
                                    textMessage = TTF_RenderText_Solid(font, (helper::int2string(SDL_GetTicks()/1000)+"s").c_str(), YELLOW);
                                    texture = SDL_CreateTextureFromSurface(screen, textMessage);
                                    SDL_FreeSurface(textMessage);
                                    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
                                    dst.x = (width / 2) - (w / 2);
                                    dst.y = (height / 4)*3 - (h / 2);
                                    dst.w = w;
                                    dst.h = h;
                                    SDL_RenderCopy(screen, texture, NULL, &dst);
                                    SDL_RenderPresent(screen);
#ifndef _WIN32
                                    helper::doVibrateFromJNI(1000);
#endif
                                    SDL_Delay(1000);
                                    SDL_DestroyTexture(texture);
                                    delete (tablero);

                                }
                            }
                        }
                    }
                }
            }
#ifndef _WIN32
        }
#endif
    }

    if (screen != NULL) SDL_DestroyRenderer(screen);
    if (window != NULL) SDL_DestroyWindow(window);
    if (music != NULL) 	Mix_FreeMusic(music);
    if (font != NULL) 	TTF_CloseFont(font);

    Mix_CloseAudio();	// Close audio

    Mix_Quit();         // Quit SDL_mixer
    TTF_Quit();    		// Quit SDL_ttf
    IMG_Quit();			// Quit SDL_image
    SDL_Quit();			// Quit SDL

    return 0;
}


