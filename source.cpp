#include <SDL2/SDL.h>
#include "C:/MinGW/include/SDL2/SDL_image.h"
#include "C:/MinGW/include/SDL2/SDL_ttf.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h> 
#include <cmath>
#include <vector>
#include <map>

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 900;
const int WINDOW_SIZE = 30;
const int SPRITE_SIZE_WIDTH = SCREEN_WIDTH/WINDOW_SIZE;
const int SPRITE_SIZE_HEIGHT = SCREEN_HEIGHT/WINDOW_SIZE;
const int SPRITE_ZOOM_FACTOR = 2;
const int ANIMATION_FRAME_RATE = 16;
const int ENEMY_SIZE = 16;
const int PLAYER_WIDTH = 16;
const int PLAYER_HEIGHT = 28;
const int COIN_SIZE = 8;
const int CHEST_SIZE = 16;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;

class LTexture {
public:
		LTexture() {
            mTexture = NULL;
            mWidth = 0;
            mHeight = 0;
        }
		~LTexture() {
            free();
        }
		bool loadFromFile( std::string path ) {
            free();
            SDL_Texture* newTexture = NULL;
            SDL_Surface* loadedSurface = IMG_Load(path.c_str());
            SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));
            newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
            SDL_FreeSurface(loadedSurface);
            mTexture = newTexture;
            return mTexture != NULL;
        }
        bool loadFromRenderedText(std::string textureText, SDL_Color textColor) {
            free();
            SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
            mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
            mWidth = textSurface->w;
            mHeight = textSurface->h;
            SDL_FreeSurface(textSurface);
            return mTexture != NULL;
        }
		void free() {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            if(mTexture != NULL){
                SDL_DestroyTexture(mTexture);
                mTexture = NULL;
                mWidth = 0;
                mHeight = 0;
            }
        }
		void setColor( Uint8 red, Uint8 green, Uint8 blue ) {
            SDL_SetTextureColorMod(mTexture, red, green, blue);
        }
		void setBlendMode( SDL_BlendMode blending ) {
            SDL_SetTextureBlendMode(mTexture, blending);
        }
		void setAlpha( Uint8 alpha ) {
            SDL_SetTextureAlphaMod(mTexture, alpha);
        }
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE, int sizeMultiplier = SPRITE_ZOOM_FACTOR){
            SDL_Rect renderQuad = { x, y, mWidth, mHeight };

            if( clip != NULL )
            {
                renderQuad.w = clip->w*sizeMultiplier;
                renderQuad.h = clip->h*sizeMultiplier;
            }

            SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
        }
		int getWidth(){
            return mWidth;
        }
		int getHeight(){
            return mHeight;
        }

	private:
		SDL_Texture* mTexture;
		int mWidth;
		int mHeight;
};

SDL_Rect gSpriteClips[100];
LTexture gSpriteSheetTexture;
LTexture gTextTexture; // TTF

class Coin {
    public:
        float mPosX, mPosY;
        int frame;
    
    Coin(float x, float y) {
        mPosX = x + COIN_SIZE;
        mPosY = y + COIN_SIZE;
    }

    void render() {
        ++frame;
        if(frame / ANIMATION_FRAME_RATE >= ANIMATION_FRAMES){
            frame = 0;
        }
        gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[(frame / ANIMATION_FRAME_RATE) + 42], 0.0, NULL, SDL_FLIP_NONE);
    }

    private:
        int ANIMATION_FRAMES = 4;

};

class Enemy {
    public:
        int health;
        bool attackable;
        float mPosX, mPosY;
        float mVelX = 0, mVelY = 0;
        int frame;
        int direction;
        int type;
        int state; // Idle, Run: 0, 4
        float Dlen;
    
    Enemy(int x, int y, int vel, int t, int hp) {
        mPosX = x;
        mPosY = y;
        enemyVel = vel;
        type = t;
        health = hp;
    }

    void move(int playerX, int playerY) {
        if(enemyVel != 0 && (mPosX != playerX || mPosY != playerY)) {
            Dlen = sqrt(((mPosX - playerX) * (mPosX - playerX)) + ((mPosY - playerY) * (mPosY - playerY)));
            mVelX = (mPosX - playerX)/Dlen;
            mVelY = (mPosY - playerY)/Dlen;

            mPosX += -mVelX*enemyVel;
            mPosY += -mVelY*enemyVel;
        }
        if(mPosX < playerX) {
            direction = 1;
        } else {
            direction = 0;
        }
    }

    void render() {
        if(mVelX != 0 || mVelY != 0) {
            state = 4;
        } else {
            state = 0;
        }
        ++frame;
        if(frame / ANIMATION_FRAME_RATE >= ANIMATION_FRAMES){
            frame = 0;
        }
        if(!direction){
            res = SDL_FLIP_HORIZONTAL;
        } else {
            res = SDL_FLIP_NONE;
        }
        if(attackable){
            gSpriteSheetTexture.setColor(255,255,255);
        } else {
            gSpriteSheetTexture.setColor(100,100,100);
        };
        gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[(frame / ANIMATION_FRAME_RATE) + type + state], 0.0, NULL, res);
        gSpriteSheetTexture.setColor(255,255,255);
    }

    private:
        SDL_RendererFlip res;
        int ANIMATION_FRAMES = 4;
        float enemyVel;
};

class Player {
    public:
        int mPosX, mPosY;
        float mVelX, mVelY;
        int attackingFrame;
        int frame;
        int direction; 
        int state; //Idle, Run, Hit: 0, 4, 8

        Player() {
            mPosX = SCREEN_WIDTH/2;
            mPosY = SCREEN_HEIGHT/2;
            attackingFrame = 0;
            frame = 0;
            direction = 1;
            mVelX = 0;
            mVelY = 0;
        }

        bool intersectEnemy(Enemy b) {
            float x1 = mPosX + ENEMY_SIZE;
            float y1 = mPosY + ENEMY_SIZE;
            int r1 = ENEMY_SIZE;

            float x2 = b.mPosX + PLAYER_WIDTH/2;
            float y2 = b.mPosY + PLAYER_HEIGHT/2;
            int r2 = PLAYER_WIDTH;

            if((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) <= (r1+r2)*(r1+r2)){
                return true;
            }

            return false;
        }

        bool intersectCoin(Coin b) {
            float x1 = mPosX;
            float y1 = mPosY;
            int r1 = COIN_SIZE;

            float x2 = b.mPosX - PLAYER_WIDTH/2;
            float y2 = b.mPosY - PLAYER_HEIGHT/2;
            int r2 = PLAYER_WIDTH;

            if((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) <= (r1+r2)*(r1+r2)){
                return true;
            }
            return false;
        }

        void handleEvents(SDL_Event& e) {
            if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
                switch(e.key.keysym.sym){
                    case SDLK_w: mVelY -= PLAYER_VEL; break;
                    case SDLK_s: mVelY += PLAYER_VEL; break;
                    case SDLK_a: mVelX -= PLAYER_VEL; direction = 0; break;
                    case SDLK_d: mVelX += PLAYER_VEL; direction = 1; break;
                }
            } else if(e.type == SDL_KEYUP && e.key.repeat == 0){
                switch(e.key.keysym.sym){
                    case SDLK_w: mVelY += PLAYER_VEL; break;
                    case SDLK_s: mVelY -= PLAYER_VEL; break;
                    case SDLK_a: mVelX += PLAYER_VEL; break;
                    case SDLK_d: mVelX -= PLAYER_VEL; break;
                }
            }
        }

        void move() {
            mPosX += mVelX;
            if((mPosX < 0) || (mPosX + PLAYER_WIDTH+13 > SCREEN_WIDTH)){
                mPosX -= mVelX;
            }

            mPosY += mVelY;
            if((mPosY < -13) || (mPosY + PLAYER_HEIGHT+24 > SCREEN_HEIGHT)){
                mPosY -= mVelY;
            }
        }

        void render() {
            if(attackingFrame > 0){
                attackingFrame--;
                gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[8], 0.0, NULL, res);
            } else {
                if(mVelX != 0 || mVelY != 0){
                    state = 4;
                } else {
                    state = 0;
                }
                ++frame;
                if(frame / ANIMATION_FRAME_RATE >= ANIMATION_FRAMES){
                    frame = 0;
                }
                if(!direction){
                    res = SDL_FLIP_HORIZONTAL;
                } else {
                    res = SDL_FLIP_NONE;
                }
                gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[(frame / ANIMATION_FRAME_RATE)+state], 0.0, NULL, res);
            }
        }

    private:
        SDL_RendererFlip res;
        int ANIMATION_FRAMES = 4;
        const int PLAYER_VEL = 3;
};

class Weapon {
    public:
        bool display;
        int clipIndex;
        bool isDropped; // true is dropped on the floor, false is in player inventory
        int mPosX = 0;
        int mPosY = 0;

        Weapon(int weaponClip, bool weaponState, int x = NULL, int y = NULL, bool hasIntroduced = true) {
            if(!hasIntroduced) {
                display = false;
            } else {
                display = true;
            }
        }

        void setProperties(int weaponClip, bool weaponState, int x = NULL, int y = NULL) {
            clipIndex = weaponClip;
            isDropped = weaponState;
            if(isDropped) {
                mPosX = x;
                mPosY = y;
            }
        }

        bool intersect(Player b) {
            if(isDropped) {
                float x1 = mPosX + PLAYER_WIDTH/2;
                float y1 = mPosY + PLAYER_HEIGHT/2;
                int r1 = PLAYER_WIDTH;

                float x2 = b.mPosX + PLAYER_WIDTH/2;
                float y2 = b.mPosY + PLAYER_HEIGHT/2;
                int r2 = PLAYER_WIDTH;

                if((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) <= (r1+r2)*(r1+r2)){
                    return true;
                }
                return false;
            }
            return false;
        }

        void render() {
            if(!isDropped) {
                mPosX = 0;
                mPosY = 0;
                gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[clipIndex], 0.0, NULL, SDL_FLIP_NONE, 4);
            } else {
                gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[clipIndex], 0.0, NULL, SDL_FLIP_NONE, 2);
            }
        }
};

class Chest {
    public:
        int mPosX;
        int mPosY;
        int opened = false;

    Chest(int x, int y) {
        mPosX = x;
        mPosY = y;
    }

    void intersectMouse(int mouseX, int mouseY) {
        if(mouseX >= mPosX && mouseX <= mPosX + CHEST_SIZE*SPRITE_ZOOM_FACTOR && mouseY >= mPosY && mouseY <= mPosY + CHEST_SIZE*SPRITE_ZOOM_FACTOR) {
            opened = true;
        }
    }

    bool render() {
        if(opened) {
            frame++;
            if(frame / ANIMATION_FRAME_RATE > ANIMATION_FRAMES) {
                return true;
            }
        }
        gSpriteSheetTexture.render(mPosX, mPosY, &gSpriteClips[(frame / ANIMATION_FRAME_RATE) + 46], 0.0, NULL, SDL_FLIP_NONE);
        return false;
    }

    private:
        int ANIMATION_FRAMES = 2;
        int frame = 0;
};

bool loadMedia() {
    bool success = true;

    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 28);
    if(gFont == NULL){
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {
        SDL_Color textColor = {0, 0, 0};
        if( !gTextTexture.loadFromRenderedText( "The quick brown fox jumps over the lazy dog", textColor )) {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }

    if(!gSpriteSheetTexture.loadFromFile("images/0x72_DungeonTilesetII_v1.1.png")){
        printf("Failed to load sprite sheet texture!\n");
    } else {
        //PLAYER IDLE ANIMATION
        gSpriteClips[0].x = 128;
        gSpriteClips[0].y = 100;
        gSpriteClips[0].w = PLAYER_WIDTH;
        gSpriteClips[0].h = PLAYER_HEIGHT;

        gSpriteClips[1].x = 144;
        gSpriteClips[1].y = 100;
        gSpriteClips[1].w = PLAYER_WIDTH;
        gSpriteClips[1].h = PLAYER_HEIGHT;

        gSpriteClips[2].x = 160;
        gSpriteClips[2].y = 100;
        gSpriteClips[2].w = PLAYER_WIDTH;
        gSpriteClips[2].h = PLAYER_HEIGHT;

        gSpriteClips[3].x = 176;
        gSpriteClips[3].y = 100;
        gSpriteClips[3].w = PLAYER_WIDTH;
        gSpriteClips[3].h = PLAYER_HEIGHT;

        //PLAYER RUN ANIMATION
        gSpriteClips[4].x = 192;
        gSpriteClips[4].y = 100;
        gSpriteClips[4].w = PLAYER_WIDTH;
        gSpriteClips[4].h = PLAYER_HEIGHT;

        gSpriteClips[5].x = 208;
        gSpriteClips[5].y = 100;
        gSpriteClips[5].w = PLAYER_WIDTH;
        gSpriteClips[5].h = PLAYER_HEIGHT;

        gSpriteClips[6].x = 224;
        gSpriteClips[6].y = 100;
        gSpriteClips[6].w = PLAYER_WIDTH;
        gSpriteClips[6].h = PLAYER_HEIGHT;

        gSpriteClips[7].x = 240;
        gSpriteClips[7].y = 100;
        gSpriteClips[7].w = PLAYER_WIDTH;
        gSpriteClips[7].h = PLAYER_HEIGHT;

        //PLAYER HIT
        gSpriteClips[8].x = 256;
        gSpriteClips[8].y = 100;
        gSpriteClips[8].w = PLAYER_WIDTH;
        gSpriteClips[8].h = PLAYER_HEIGHT;

        //FLOORS
        gSpriteClips[9].x = 16;
        gSpriteClips[9].y = 64;
        gSpriteClips[9].w = 16;
        gSpriteClips[9].h = 16;

        gSpriteClips[10].x = 32;
        gSpriteClips[10].y = 64;
        gSpriteClips[10].w = 16;
        gSpriteClips[10].h = 16;

        gSpriteClips[11].x = 48;
        gSpriteClips[11].y = 64;
        gSpriteClips[11].w = 16;
        gSpriteClips[11].h = 16;

        gSpriteClips[12].x = 16;
        gSpriteClips[12].y = 80;
        gSpriteClips[12].w = 16;
        gSpriteClips[12].h = 16;

        gSpriteClips[13].x = 32;
        gSpriteClips[13].y = 80;
        gSpriteClips[13].w = 16;
        gSpriteClips[13].h = 16;

        gSpriteClips[14].x = 48;
        gSpriteClips[14].y = 80;
        gSpriteClips[14].w = 16;
        gSpriteClips[14].h = 16;

        gSpriteClips[15].x = 16;
        gSpriteClips[15].y = 96;
        gSpriteClips[15].w = 16;
        gSpriteClips[15].h = 16;

        gSpriteClips[16].x = 32;
        gSpriteClips[16].y = 96;
        gSpriteClips[16].w = 16;
        gSpriteClips[16].h = 16;

        //TINY ZOMBIE IDLE
        gSpriteClips[17].x = 368;
        gSpriteClips[17].y = 16;
        gSpriteClips[17].w = ENEMY_SIZE;
        gSpriteClips[17].h = ENEMY_SIZE;

        gSpriteClips[18].x = 384;
        gSpriteClips[18].y = 16;
        gSpriteClips[18].w = ENEMY_SIZE;
        gSpriteClips[18].h = ENEMY_SIZE;

        gSpriteClips[19].x = 400;
        gSpriteClips[19].y = 16;
        gSpriteClips[19].w = ENEMY_SIZE;
        gSpriteClips[19].h = ENEMY_SIZE;

        gSpriteClips[20].x = 416;
        gSpriteClips[20].y = 16;
        gSpriteClips[20].w = ENEMY_SIZE;
        gSpriteClips[20].h = ENEMY_SIZE;

        //TINY ZOMBIE RUN
        gSpriteClips[21].x = 432;
        gSpriteClips[21].y = 16;
        gSpriteClips[21].w = ENEMY_SIZE;
        gSpriteClips[21].h = ENEMY_SIZE;

        gSpriteClips[22].x = 448;
        gSpriteClips[22].y = 16;
        gSpriteClips[22].w = ENEMY_SIZE;
        gSpriteClips[22].h = ENEMY_SIZE;

        gSpriteClips[23].x = 464;
        gSpriteClips[23].y = 16;
        gSpriteClips[23].w = ENEMY_SIZE;
        gSpriteClips[23].h = ENEMY_SIZE;

        gSpriteClips[24].x = 480;
        gSpriteClips[24].y = 16;
        gSpriteClips[24].w = ENEMY_SIZE;
        gSpriteClips[24].h = ENEMY_SIZE;

        //GOBLIN IDLE
        gSpriteClips[25].x = 368;
        gSpriteClips[25].y = 32;
        gSpriteClips[25].w = ENEMY_SIZE;
        gSpriteClips[25].h = ENEMY_SIZE;

        gSpriteClips[26].x = 384;
        gSpriteClips[26].y = 32;
        gSpriteClips[26].w = ENEMY_SIZE;
        gSpriteClips[26].h = ENEMY_SIZE;

        gSpriteClips[27].x = 400;
        gSpriteClips[27].y = 32;
        gSpriteClips[27].w = ENEMY_SIZE;
        gSpriteClips[27].h = ENEMY_SIZE;

        gSpriteClips[28].x = 416;
        gSpriteClips[28].y = 32;
        gSpriteClips[28].w = ENEMY_SIZE;
        gSpriteClips[28].h = ENEMY_SIZE;

        //GOBLIN RUN
        gSpriteClips[29].x = 432;
        gSpriteClips[29].y = 32;
        gSpriteClips[29].w = ENEMY_SIZE;
        gSpriteClips[29].h = ENEMY_SIZE;

        gSpriteClips[30].x = 448;
        gSpriteClips[30].y = 32;
        gSpriteClips[30].w = ENEMY_SIZE;
        gSpriteClips[30].h = ENEMY_SIZE;

        gSpriteClips[31].x = 464;
        gSpriteClips[31].y = 32;
        gSpriteClips[31].w = ENEMY_SIZE;
        gSpriteClips[31].h = ENEMY_SIZE;

        gSpriteClips[32].x = 480;
        gSpriteClips[32].y = 32;
        gSpriteClips[32].w = ENEMY_SIZE;
        gSpriteClips[32].h = ENEMY_SIZE;

        //IMP IDLE
        gSpriteClips[33].x = 368;
        gSpriteClips[33].y = 48;
        gSpriteClips[33].w = ENEMY_SIZE;
        gSpriteClips[33].h = ENEMY_SIZE;

        gSpriteClips[34].x = 384;
        gSpriteClips[34].y = 48;
        gSpriteClips[34].w = ENEMY_SIZE;
        gSpriteClips[34].h = ENEMY_SIZE;

        gSpriteClips[35].x = 400;
        gSpriteClips[35].y = 48;
        gSpriteClips[35].w = ENEMY_SIZE;
        gSpriteClips[35].h = ENEMY_SIZE;

        gSpriteClips[36].x = 416;
        gSpriteClips[36].y = 48;
        gSpriteClips[36].w = ENEMY_SIZE;
        gSpriteClips[36].h = ENEMY_SIZE;

        //IMP RUN
        gSpriteClips[37].x = 432;
        gSpriteClips[37].y = 48;
        gSpriteClips[37].w = ENEMY_SIZE;
        gSpriteClips[37].h = ENEMY_SIZE;

        gSpriteClips[38].x = 448;
        gSpriteClips[38].y = 48;
        gSpriteClips[38].w = ENEMY_SIZE;
        gSpriteClips[38].h = ENEMY_SIZE;

        gSpriteClips[39].x = 464;
        gSpriteClips[39].y = 48;
        gSpriteClips[39].w = ENEMY_SIZE;
        gSpriteClips[39].h = ENEMY_SIZE;

        gSpriteClips[40].x = 480;
        gSpriteClips[40].y = 48;
        gSpriteClips[40].w = ENEMY_SIZE;
        gSpriteClips[40].h = ENEMY_SIZE;

        //KNIFE
        gSpriteClips[41].x = 293;
        gSpriteClips[41].y = 18;
        gSpriteClips[41].w = 6;
        gSpriteClips[41].h = 13;

        //COIN ANIMATION
        gSpriteClips[42].x = 288;
        gSpriteClips[42].y = 272;
        gSpriteClips[42].w = COIN_SIZE;
        gSpriteClips[42].h = COIN_SIZE;

        gSpriteClips[43].x = 296;
        gSpriteClips[43].y = 272;
        gSpriteClips[43].w = COIN_SIZE;
        gSpriteClips[43].h = COIN_SIZE;

        gSpriteClips[44].x = 304;
        gSpriteClips[44].y = 272;
        gSpriteClips[44].w = COIN_SIZE;
        gSpriteClips[44].h = COIN_SIZE;

        gSpriteClips[45].x = 312;
        gSpriteClips[45].y = 272;
        gSpriteClips[45].w = COIN_SIZE;
        gSpriteClips[45].h = COIN_SIZE;

        //CHEST
        gSpriteClips[46].x = 240;
        gSpriteClips[46].y = 224;
        gSpriteClips[46].w = CHEST_SIZE;
        gSpriteClips[46].h = CHEST_SIZE;

        //CHEST OPEN ANIMATION
        gSpriteClips[47].x = 256;
        gSpriteClips[47].y = 224;
        gSpriteClips[47].w = CHEST_SIZE;
        gSpriteClips[47].h = CHEST_SIZE;

        gSpriteClips[48].x = 272;
        gSpriteClips[48].y = 224;
        gSpriteClips[48].w = CHEST_SIZE;
        gSpriteClips[48].h = CHEST_SIZE;

    }
    return success;
}

void close() {
    gSpriteSheetTexture.free();    
    gTextTexture.free();
    TTF_CloseFont(gFont);
    gFont = NULL;
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool init() {
	bool success = true;
	if(SDL_Init( SDL_INIT_VIDEO ) < 0) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else {
		if(!SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
			printf( "Warning: Linear texture filtering not enabled!" );
		}
		gWindow = SDL_CreateWindow( "Accolade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if(gWindow == NULL) {
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if(gRenderer == NULL) {
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else {
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) & imgFlags)) {
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

                if(TTF_Init() == -1){
                    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
			}
		}
	}
	return success;
}

void setTileMap(int tileMap[][WINDOW_SIZE]) {
    for(int i = 0; i < WINDOW_SIZE; i++) {
        for(int j = 0; j < WINDOW_SIZE; j++) {
            int p = rand()%100;
            if(p >= 0 && p <= 92) { //99 98 97 96 95 94 93
                tileMap[i][j] = 9;
            } else if(p == 93) {
                tileMap[i][j] = 10;
            } else if(p == 94) {
                tileMap[i][j] = 11;
            } else if(p == 95) {
                tileMap[i][j] = 12;
            } else if(p == 96) {
                tileMap[i][j] = 13;
            } else if(p == 97) {
                tileMap[i][j] = 14;
            } else if(p == 98) {
                tileMap[i][j] = 15;
            } else {
                tileMap[i][j] = 16;
            }
        }
    }
}

void renderTileMap(int tileMap[][WINDOW_SIZE]) {
    for(int i = 0; i < WINDOW_SIZE; i++) {
        for(int j = 0; j < WINDOW_SIZE; j++) {
            gSpriteSheetTexture.render(j*SPRITE_SIZE_WIDTH, i*SPRITE_SIZE_HEIGHT, &gSpriteClips[tileMap[i][j]], 0.0, NULL, SDL_FLIP_NONE);
        }
    }
}

void printTileMap(int tileMap[][WINDOW_SIZE]){ // DEBUG ONLY
    for(int i = 0; i < WINDOW_SIZE; i++) {
        for(int j = 0; j < WINDOW_SIZE; j++) {
            std::cout << tileMap[i][j];
        }
        std::cout << "\n";
    }
}

Enemy createEnemy(float x, float y, int vel, int t, int hp) {
    Enemy e(x, y, vel, t, hp);
    return e;
}

Coin createCoin(int x, int y) {
    Coin c(x, y);
    return c;
}

Chest createChest(int x, int y) {
    Chest c(x, y);
    return c;
}

std::pair<float, float> getEnemySpawnPosition() {
    float x, y;
    float delay = rand()%(21)+40;
    if(rand() % 2 == 0){
        x = rand() % SCREEN_WIDTH;
        y = rand() % 2 == 0 ? -ENEMY_SIZE*delay: SCREEN_HEIGHT + ENEMY_SIZE*delay;
    } else {
        x = rand() % 2 == 0 ? -ENEMY_SIZE*delay: SCREEN_WIDTH + ENEMY_SIZE*delay;
        y = rand() % SCREEN_HEIGHT;
    }

    return std::make_pair(x, y);
}

void createEnemies(std::vector<Enemy>& enemies, int amount, int speed, int clip, int health) {
    for(int i = enemies.size(); i < amount; i++) {
        std::pair<float, float> pos = getEnemySpawnPosition();
        enemies.push_back(createEnemy(pos.first, pos.second, speed, clip, health));
    }
}

int main(int argc, char* args[]){
    srand((unsigned)time(NULL));
    if(!init()){
        printf("Failed to initialize!\n");
    } else if(!loadMedia()){
        printf("Failed to load media!\n");
    } else {
        int monsterData[3][3] = {
            {2, 17, 1}, // Tiny Zombie
            {1, 25, 2}, // Goblins
            {1, 33, 5} // Imp
        };


        int waveData[7][4] = { // In each wave, monster: amount, speed, clip, and health is defined in order
            {-1, 0, 0, 0}, // Not an actual wave
            {10, 2, 17, 1}, // Tiny Zombie 
            {10, 1, 25, 2}, // Goblins 
            {5, 1, 33, 5}, // Imp
            {0, 0, 0, 0}, // Knife Event
            {0, 0, 0, 0},
            {0, 0, 0 , 0},
        };

        std::string waveNarration[7] {
            "Welcome to Accolade, WASD to move, left click to kick and continue.",
            "Here are some zombies for you to fight!",
            "Next wave! goblins!",
            "Third wave already? Imps.",
            "",
            "Hmmm what is that next to you? A KNIFE?!",
            "Now you do extra damage, (left click to continue)",
        };

        std::map<int, int> weaponData; // Track weapon damage (clip, damage)
        weaponData[0] = 1;
        weaponData[41] = 2; // Knife

        int tileMap[WINDOW_SIZE][WINDOW_SIZE];
        setTileMap(tileMap);

        bool quit = false;
        SDL_Event e;
        Player player;

        std::vector<Enemy> enemies;
        std::vector<Coin> coins;
        std::vector<Chest> chests;

        int currentWeapon = 0; //No weapon ( just a kick :) )
        int currentWave = 0;
        int killCount = 0;
        int coinCount = 0;
        bool endlessMode = false;

        SDL_Color c;
        c.r = 255;
        c.g = 255;
        c.b = 255;

        Weapon weapon(41, true, rand()%SCREEN_WIDTH, rand()%SCREEN_HEIGHT, false);

        //Endless Mode Variables
        int enemiesToKill;
        int enemiesToGenerate;
        int enemyAmount;
        int enemySpeed;
        int enemyClip;
        int enemyHealth;

        while(!quit){
            while(SDL_PollEvent( &e ) != 0) {
                if( e.type == SDL_QUIT ) {
                    quit = true;
                }

                if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT){
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    for(int i = 0; i < chests.size(); i++) {
                        chests[i].intersectMouse(mouseX, mouseY);
                    }
                    if(player.attackingFrame == 0) {
                        if(currentWave == 0) {
                            killCount = -1;
                        }
                        player.attackingFrame = ANIMATION_FRAME_RATE;
                        for(int i = 0; i < enemies.size(); i++){
                            if(enemies[i].attackable) {
                                enemies[i].health -= weaponData[currentWeapon];
                            }
                        }
                    }
                }

                player.handleEvents(e);
            }


            SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(gRenderer);

            renderTileMap(tileMap);

            player.move();
            player.render();

            for(int i = 0; i < enemies.size(); i++){
                if(enemies[i].health <= 0){
                    killCount++;
                    coins.push_back(createCoin(enemies[i].mPosX, enemies[i].mPosY));
                    enemies.erase(enemies.begin() + i);
                } else {
                    if(player.intersectEnemy(enemies[i])){
                        enemies[i].mVelX = 0;
                        enemies[i].mVelY = 0;
                        enemies[i].attackable = true;
                    } else {
                        enemies[i].attackable = false;
                        enemies[i].move(player.mPosX + PLAYER_WIDTH/2, player.mPosY + PLAYER_HEIGHT/2);
                    }
                    enemies[i].render();
                }
            }

            if(currentWave != sizeof(waveData)/sizeof(waveData[0])) {
                if(killCount == waveData[currentWave][0]) {
                    killCount = 0;
                    if(currentWave + 1 < sizeof(waveData)/sizeof(waveData[0]) && waveData[currentWave][0] != 0) {
                        currentWave++;  
                    }
                    if(waveData[currentWave][0] != 0) {
                        createEnemies(
                            enemies, 
                            waveData[currentWave][0], 
                            waveData[currentWave][1], 
                            waveData[currentWave][2], 
                            waveData[currentWave][3]
                        ); 
                    }
                }
            } else {
                endlessMode = true;
            }

            if(endlessMode) {
                if(enemies.empty()) {
                    killCount = 0;
                    enemiesToKill = (rand()%20)+10;
                    enemiesToGenerate = enemiesToKill;

                    for(int i = 0; i < enemiesToGenerate; i++) {
                        int enemyType = rand()%3;
                        std::pair<float, float> pos = getEnemySpawnPosition();
                        enemies.push_back(createEnemy(pos.first, pos.second, monsterData[enemyType][0], monsterData[enemyType][1], monsterData[enemyType][2]));
                    }
                }
            }

            //Wave Specific Events
            if(currentWave == 4) {
                weapon.display = true;
                weapon.setProperties(41, true, rand()%SCREEN_WIDTH, rand()%SCREEN_HEIGHT);
                currentWave++;
            } else if(currentWave == 5) {
                if(weapon.isDropped && weapon.intersect(player)) {
                    currentWeapon = 41;
                    weapon.isDropped = false;
                    currentWave++;
                }
            } else if(currentWave == 6) {
                if(player.attackingFrame > 0) {
                    currentWave++;
                    killCount = waveData[currentWave][0];
                }
            }

            if(weapon.display) {
                weapon.render();
            }

            for(int i = 0; i < coins.size(); i++){
                if(player.intersectCoin(coins[i])) {
                    coinCount += (rand()%50)+1;
                    coins.erase(coins.begin() + i);
                } else {
                    coins[i].render();
                }
            }

            for(int i = 0; i < chests.size(); i++){
                if(chests[i].render()) {
                    chests.erase(chests.begin() + i);
                }
            }

            if(currentWave < sizeof(waveNarration)/sizeof(waveNarration[0])) {
                gTextTexture.loadFromRenderedText(waveNarration[currentWave], c);
                gTextTexture.render(0, SCREEN_HEIGHT-gTextTexture.getHeight());
            }

            gTextTexture.loadFromRenderedText("Coins: " + std::to_string(coinCount), c);
            gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth(), 0);
            
            SDL_RenderPresent(gRenderer);
        }
    }
    close();

    return 0;
}