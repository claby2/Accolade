#include <SDL2/SDL.h>
#include "C:/MinGW/include/SDL2/SDL_image.h"
#include "C:/MinGW/include/SDL2/SDL_ttf.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h> 
#include <cmath>
#include <vector>

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

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

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
        // bool loadFromRenderedText(std::string textureText, SDL_Color textColor) {
        //     free();
        //     SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
        //     mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        //     mWidth = textSurface->w;
        //     mHeight = textSurface->h;
        //     SDL_FreeSurface(textSurface);
        //     return mTexture != NULL;
        // }
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
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){
            SDL_Rect renderQuad = { x, y, mWidth, mHeight };

            if( clip != NULL )
            {
                renderQuad.w = clip->w*SPRITE_ZOOM_FACTOR;
                renderQuad.h = clip->h*SPRITE_ZOOM_FACTOR;
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

        bool intersect(Enemy b){
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

bool loadMedia() {
    bool success = true;

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


    }
    return success;
}

void close() {
    gSpriteSheetTexture.free();
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

void printTileMap(int tileMap[][WINDOW_SIZE]){
    for(int i = 0; i < WINDOW_SIZE; i++) {
        for(int j = 0; j < WINDOW_SIZE; j++) {
            std::cout << tileMap[i][j];
        }
        std::cout << "\n";
    }
}

Enemy createEnemy(int x, int y, int vel, int t, int hp) {
    Enemy e(x, y, vel, t, hp);
    return e;
}

int main(int argc, char* args[]){
    srand((unsigned)time(NULL));
    if(!init()){
        printf("Failed to initialize!\n");
    } else if(!loadMedia()){
        printf("Failed to load media!\n");
    } else {

        int tileMap[WINDOW_SIZE][WINDOW_SIZE];
        setTileMap(tileMap);

        bool quit = false;
        SDL_Event e;
        Player player;

        std::vector<Enemy> enemies;

        // Enemy enemy(10, 10, 2, 17);

        for(int i = 0; i < 10; i++) {
            int x, y;
            if(rand() % 2 == 0){
                x = rand() % SCREEN_WIDTH;
                y = rand() % 2 == 0 ? -ENEMY_SIZE: SCREEN_HEIGHT + ENEMY_SIZE;
            } else {
                x = rand() % 2 == 0 ? -ENEMY_SIZE: SCREEN_WIDTH + ENEMY_SIZE;
                y = rand() % SCREEN_HEIGHT;
            }
            enemies.push_back(createEnemy(x, y, 1, 17, 1));
        }

        while(!quit){
            while(SDL_PollEvent( &e ) != 0) {
                if( e.type == SDL_QUIT ) {
                    quit = true;
                }

                if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT){
                    player.attackingFrame = ANIMATION_FRAME_RATE;
                    for(int i = 0; i < enemies.size(); i++){
                        if(enemies[i].attackable) {
                            --enemies[i].health;
                        }
                    }
                }

                player.handleEvents(e);
            }


            SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(gRenderer);
            // ########
            // #RENDER#
            // ########
            renderTileMap(tileMap);

            player.move();
            player.render();

            for(int i = 0; i < enemies.size(); i++){
                if(enemies[i].health == 0){
                    enemies.erase(enemies.begin() + i);
                } else {
                    if(player.intersect(enemies[i])){
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

            for(int i = enemies.size(); i < 10; i++) {
                int x, y;
                if(rand() % 2 == 0){
                    x = rand() % SCREEN_WIDTH;
                    y = rand() % 2 == 0 ? -ENEMY_SIZE: SCREEN_HEIGHT + ENEMY_SIZE;
                } else {
                    x = rand() % 2 == 0 ? -ENEMY_SIZE: SCREEN_WIDTH + ENEMY_SIZE;
                    y = rand() % SCREEN_HEIGHT;
                }
                enemies.push_back(createEnemy(x, y, 1, 17, 1));
            }

            SDL_RenderPresent(gRenderer);

        }
    }
    close();

    return 0;
}