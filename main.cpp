#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfx.h>
#include <SDL2/SDL_mixer.h>
#include <cmath>
#include <ctime>
#include <fstream>
using namespace std;

ifstream f_read, gamesave_read, Music_info_read, Help_read;
ofstream f_write, gamesave_write, Music_info_write, Help_write;
SDL_Rect launchpad_rect;
SDL_Rect background_rect;
SDL_Rect ball_rect;
SDL_Rect Num_rect;
SDL_Rect gameover_rect;
SDL_Rect savegame_rect;
SDL_Rect pause_rect;
SDL_Rect heart_rect;
SDL_Texture* background_t = NULL;
SDL_Texture* heart_t = NULL;
SDL_Texture* pause_t = NULL;
SDL_Texture* launch_pad_t = NULL;
SDL_Texture* stone_t = NULL;
SDL_Texture* ball_t = NULL;
SDL_Texture* gameover_t = NULL;
SDL_Texture* savegame_t = NULL;
SDL_Texture* Num_0 = NULL; SDL_Texture* Num_1 = NULL; SDL_Texture* Num_2 = NULL; SDL_Texture* Num_3 = NULL; SDL_Texture* Num_4 = NULL;
SDL_Texture* Num_5 = NULL; SDL_Texture* Num_6 = NULL; SDL_Texture* Num_7 = NULL; SDL_Texture* Num_8 = NULL; SDL_Texture* Num_9 = NULL;

struct stone{
    int x;
    int y;
    int strength;
};

struct user{
	bool saved;
	string name;
	int num_of_played_rounds;
	int score[1000];
	int level[1000];
	int lives;
};

string space_remove(string s);
void print_hearts(SDL_Renderer *render, int lives);
void print_num_pic(SDL_Renderer *render, int x, int num_x, int num_y, int l);
void write_player(user player);
void read_player(user &player);
void print_player_info(user player);
void save_score_level(user player, int score, int level);
void draw_stone(SDL_Renderer *render, stone s, SDL_Rect stone_rect);
void draw_stones(SDL_Renderer *render, stone s[][21], SDL_Rect stone_rect);
void draw_vector(SDL_Renderer *render, int x, int y, int mx, int my);
void load_saved_game(stone stones[][21], int &ball_damage, double &vx, double &vy, int &x, int &y, bool &shoted);
void save_game(stone stones[][21], int ball_damage, double vx, double vy, int x, int y, bool shoted);
void print_game_info(SDL_Renderer *render, user player);

int main( int argc, char * argv[] )
{
    srand(time(NULL));
    int H = 800, W = 1200, option, ball_damage, blanks = 0, r = 22, alpha, MusicNum;
    double v = 3, vx, vy, SIN, COS, ball_x, ball_y;
    string name, tmp1, tmp2;
    bool lost = 0, shoted = 0, new_line = 0, pass = 0, paused = 0;
    stone stones[8][21];
    user player;
    SDL_Rect stone_rect;
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 21; j++)
        {
            stones[i][j].strength = 0;
            stones[i][j].x = 132*i + 16*(i + 1);
            stones[i][j].y = 30*j + 5*(j + 1);
        }
    stone_rect.w = 132; stone_rect.h = 30;
    launchpad_rect.w = 200; launchpad_rect.h = 30; launchpad_rect.y = 750;
    background_rect.x = 0; background_rect.y = 0; background_rect.w = W; background_rect.h = H;
    Num_rect.h = 20; Num_rect.w = 15;
    ball_rect.h = 42; ball_rect.w = 50;
    gameover_rect.h = 500; gameover_rect.w = 500; gameover_rect.x = 350; gameover_rect.y = 150;
    savegame_rect.h = 200; savegame_rect.w = 200; savegame_rect.x = 500; savegame_rect.y = 300;
    pause_rect.h = 300; pause_rect.w = 300; pause_rect.x = 450; pause_rect.y = 250;
    heart_rect.h = 20; heart_rect.w = 20;
    cout << "(F5: Save and close , SPACE: Pause/Resume)" << endl;
    cout << "Player name: ";
    getline(cin, name);
    name = space_remove(name);
    tmp1 = "./Players/" + name + ".txt";
    tmp2 = "./Players/" + name + "_sg.txt";
    f_read.open(tmp1.c_str());
    if(f_read.is_open())
    {
		read_player(player);
		cout << "*********************************************" << endl;
		print_player_info(player);
		cout << "*********************************************" << endl;
		if(player.saved)
		{
			cout << "You can continue your saved game or start a new game (1 -> Continue , 2 -> New game) ..." << endl;
			cin >> option;
		}
		else
			option = 2;
        if(option == 2)
        {
            if(player.saved)
                player.num_of_played_rounds++;
            player.saved = 0;
            player.level[player.num_of_played_rounds] = 1;
            player.score[player.num_of_played_rounds] = 0;
            player.lives = 3;
            ball_damage = 1;
            cout << "New game Started ! :)" << endl;
            for(int i = 0; i < 8; i++)
                for(int j = 0; j < 3; j++)
                    stones[i][j].strength = 1;
        }
    }
    else
    {
        player.name = name;
        player.saved = 0;
        player.num_of_played_rounds = 0;
        player.level[0] = 1;
        player.score[0] = 0;
        player.lives = 3;
        ball_damage = 1;
        option = 2;
        cout << "New game Started ! :)" << endl;
        for(int i = 0; i < 8; i++)
            for(int j = 0; j < 3; j++)
                stones[i][j].strength = 1;
    }
    if(option == 1)
    {
        gamesave_read.open(tmp2.c_str());
        load_saved_game(stones, ball_damage, vx, vy, ball_rect.x, ball_rect.y, shoted);
        gamesave_read.close();
    }
    gamesave_write.open(tmp2.c_str());
    f_write.open(tmp1.c_str());
    Uint32 WND_flags = SDL_WINDOW_SHOWN;
    SDL_Window *window;
    SDL_Renderer *render;
    Music_info_read.open("./Assets/Sound/BackgroundMusic/Round_Num.txt");
    Music_info_read >> MusicNum;
    Music_info_read.close();
    Music_info_write.open("./Assets/Sound/BackgroundMusic/Round_Num.txt");
    if(MusicNum < 3)
        Music_info_write << MusicNum + 1;
    else
        Music_info_write << 1;
    Music_info_write.close();
    SDL_Init(SDL_INIT_EVERYTHING);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 8, 2048);
    Mix_Music *bgm;
    if(MusicNum == 1)
        bgm = Mix_LoadMUS("./Assets/Sound/BackgroundMusic/(Fine Layers Of)Slaysenflite.mp3");
    else if(MusicNum == 2)
        bgm = Mix_LoadMUS("./Assets/Sound/BackgroundMusic/In a Pile of Its Own Good.mp3");
    else if(MusicNum == 3)
        bgm = Mix_LoadMUS("./Assets/Sound/BackgroundMusic/The Ballad of Ace LeBaron.mp3");
    Mix_Chunk *stoneSmashEffect1 = Mix_LoadWAV("./Assets/Sound/Effects/stoneSmash1.wav");
    Mix_Chunk *stoneSmashEffect2 = Mix_LoadWAV("./Assets/Sound/Effects/stoneSmash2.wav");
    Mix_Chunk *launchEffect = Mix_LoadWAV("./Assets/Sound/Effects/launch.wav");
    Mix_Chunk *launchPadSmashEffect = Mix_LoadWAV("./Assets/Sound/Effects/launchPadSmash.wav");
    Mix_Chunk *outEffect = Mix_LoadWAV("./Assets/Sound/Effects/out.wav");
    Mix_Chunk *savegame = Mix_LoadWAV("./Assets/Sound/Effects/savegame.wav");
    Mix_Chunk *gameover = Mix_LoadWAV("./Assets/Sound/Effects/gameover.wav");
    Mix_VolumeMusic(10);
    Mix_VolumeChunk(stoneSmashEffect1, 200);
    Mix_VolumeChunk(stoneSmashEffect2, 200);
    Mix_VolumeChunk(launchEffect, 200);
    Mix_VolumeChunk(launchPadSmashEffect, 200);
    Mix_VolumeChunk(outEffect, 200);
    Mix_VolumeChunk(savegame, 200);
    Mix_VolumeChunk(gameover, 200);
    Mix_PlayMusic(bgm, -1);
    SDL_CreateWindowAndRenderer( W, H, WND_flags, &window, &render );
    SDL_SetWindowTitle(window, "BrickBreaker(By MMohseniZ)");
    SDL_RaiseWindow(window);
    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    SDL_Texture *texture;
    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, W, H);
    SDL_SetRenderTarget(render, texture);
    background_t = IMG_LoadTexture(render, "./Assets/IMG/background.jpg");
    SDL_RenderCopy(render, background_t, NULL, &background_rect);
    textRGBA(render, 10, 780, "Score: ", 2, 15, 255, 255, 255, 255);
    textRGBA(render, 550, 780, "Level: ", 2, 15, 255, 255, 255, 255);
    textRGBA(render, 1065, 780, "Lives: ", 2, 15, 255, 255, 255, 255);
    SDL_SetRenderTarget(render, NULL);
    launch_pad_t = IMG_LoadTexture(render, "./Assets/IMG/launchpad.jpg");
    stone_t = IMG_LoadTexture(render, "./Assets/IMG/stone.jpg");
    ball_t = IMG_LoadTexture(render, "./Assets/IMG/ball.png");
    gameover_t = IMG_LoadTexture(render, "./Assets/IMG/gameover.png");
    savegame_t = IMG_LoadTexture(render, "./Assets/IMG/savegame.png");
    pause_t = IMG_LoadTexture(render, "./Assets/IMG/pause.png");
    heart_t = IMG_LoadTexture(render, "./Assets/IMG/heart.png");
    Num_0 = IMG_LoadTexture(render, "./Assets/IMG/Num_0.png");
    Num_1 = IMG_LoadTexture(render, "./Assets/IMG/Num_1.png");
    Num_2 = IMG_LoadTexture(render, "./Assets/IMG/Num_2.png");
    Num_3 = IMG_LoadTexture(render, "./Assets/IMG/Num_3.png");
    Num_4 = IMG_LoadTexture(render, "./Assets/IMG/Num_4.png");
    Num_5 = IMG_LoadTexture(render, "./Assets/IMG/Num_5.png");
    Num_6 = IMG_LoadTexture(render, "./Assets/IMG/Num_6.png");
    Num_7 = IMG_LoadTexture(render, "./Assets/IMG/Num_7.png");
    Num_8 = IMG_LoadTexture(render, "./Assets/IMG/Num_8.png");
    Num_9 = IMG_LoadTexture(render, "./Assets/IMG/Num_9.png");
    int padx = 500;
    int mx;
    int my;
    if(player.saved)
    {
        ball_x = ball_rect.x + 25;
        ball_y = ball_rect.y + 21;
        paused = 1;
    }
    SDL_Event *event = new SDL_Event();
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    while(event -> key.keysym.sym != SDLK_F5 && !lost)
    {
        SDL_PollEvent(event);
        SDL_GetMouseState(&mx, &my);
        SDL_RenderCopy(render, texture, NULL, NULL);
        if(state[SDL_SCANCODE_SPACE])
        {
            paused = !paused;
            SDL_Delay(200);
        }
        if(!paused)
        {
            if(shoted)
            {
                if(mx > padx + 100 && padx + 200 <= 1200)
                    padx += 10;
                if(mx < padx + 100 && padx - 10 >= 0)
                    padx -= 10;
                ball_x += vx;
                ball_y += vy;
                if(vy == 0)
                    vy = -0.2;
                if(ball_x + r + vx > 1200)
                {
                    ball_x = 1200 - r;
                    vx = -vx;
                    Mix_PlayChannel(3, stoneSmashEffect1, 0);
                }
                else if(ball_x - r + vx < 0)
                {
                    ball_x = r;
                    vx = -vx;
                    Mix_PlayChannel(3, stoneSmashEffect1, 0);
                }
                if(ball_y - r + vy < 0)
                {
                    ball_y = r;
                    vy = -vy;
                    Mix_PlayChannel(3, stoneSmashEffect1, 0);
                }
                else if(ball_y + r > 800)
                {
                    player.lives--;
                    if(player.lives == 0)
                        lost = 1;
                    shoted = 0;
                    Mix_PlayChannel(5, outEffect, 0);
                }
                if(ball_x + r > padx && ball_x + r < padx + 10 && ball_y < 780 && ball_y > 755)
                {
                    ball_x = padx - r;
                    vx = -vx;
                    Mix_PlayChannel(1, launchPadSmashEffect, 0);
                }
                else if(ball_x - r < padx + 200 && ball_x - r > padx + 190 && ball_y < 780 && ball_y > 755)
                {
                    ball_x = padx + 200 + r;
                    vx = -vx;
                    Mix_PlayChannel(1, launchPadSmashEffect, 0);
                }
                else if(ball_y + r > 750 && ball_x >= padx + 5 && ball_x <= padx + 195)
                {
                    ball_y = 750 - r;
                    vy = -vy;
                    Mix_PlayChannel(1, launchPadSmashEffect, 0);
                }
                else if((ball_x - padx)*(ball_x - padx) + (ball_y - 750)*(ball_y - 750) < r*r)
                {
                    alpha = rand()%80 + 95;
                    vx = v*cos(alpha * 3.14159265 / 180);
                    vy = -v*sin(alpha * 3.14159265 / 180);
                    Mix_PlayChannel(1, launchPadSmashEffect, 0);
                }
                else if((ball_x - padx - 200)*(ball_x - padx - 200) + (ball_y - 750)*(ball_y - 750) < r*r)
                {
                    alpha = rand()%80 + 5;
                    vx = v*cos(alpha * 3.14159265 / 180);
                    vy = -v*sin(alpha * 3.14159265 / 180);
                    Mix_PlayChannel(1, launchPadSmashEffect, 0);
                }
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 21; j++)
                        if(stones[i][j].strength > 0)
                        {
                            if(ball_x < stones[i][j].x + 127 && ball_x > stones[i][j].x + 5 && ball_y + r > stones[i][j].y && ball_y + r < stones[i][j].y + 10)
                            {
                                ball_y = stones[i][j].y - r;
                                vy = -vy;
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(2, stoneSmashEffect1, 0);
                                break;
                            }
                            else if(ball_x < stones[i][j].x + 127 && ball_x > stones[i][j].x + 5 && ball_y - r > stones[i][j].y + 20 && ball_y - r < stones[i][j].y + 30)
                            {
                                ball_y = stones[i][j].y + r + 30;
                                vy = -vy;
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(2, stoneSmashEffect1, 0);
                                break;
                            }
                            else if(ball_y > stones[i][j].y + 5 && ball_y < stones[i][j].y + 25 && ball_x + r < stones[i][j].x + 10 && ball_x + r > stones[i][j].x)
                            {
                                ball_x = stones[i][j].x - r;
                                vx = -vx;
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(2, stoneSmashEffect1, 0);
                                break;
                            }
                            else if(ball_y > stones[i][j].y + 5 && ball_y < stones[i][j].y + 25 && ball_x - r < stones[i][j].x + 132 && ball_x - r > stones[i][j].x + 122)
                            {
                                ball_x = stones[i][j].x + r + 132;
                                vx = -vx;
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(2, stoneSmashEffect1, 0);
                                break;
                            }
                            else if((ball_x - stones[i][j].x)*(ball_x - stones[i][j].x) + (ball_y - stones[i][j].y - 30)*(ball_y - stones[i][j].y - 30) < r*r)
                            {
                                alpha = rand() % 80 - 175;
                                vx = v*cos(alpha * 3.14159265 / 180);
                                vy = -v*sin(alpha * 3.14159265 / 180);
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(3, stoneSmashEffect2, 0);
                                break;
                            }
                            else if((ball_x - stones[i][j].x)*(ball_x - stones[i][j].x) + (ball_y - stones[i][j].y)*(ball_y - stones[i][j].y) < r*r)
                            {
                                alpha = rand() % 80 + 95;
                                vx = v*cos(alpha * 3.14159265 / 180);
                                vy = -v*sin(alpha * 3.14159265 / 180);
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(3, stoneSmashEffect2, 0);
                                break;
                            }
                            else if((ball_x - stones[i][j].x - 132)*(ball_x - stones[i][j].x - 132) + (ball_y - stones[i][j].y)*(ball_y - stones[i][j].y) < r*r)
                            {
                                alpha = rand() % 80 + 5;
                                vx = v*cos(alpha * 3.14159265 / 180);
                                vy = -v*sin(alpha * 3.14159265 / 180);
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(3, stoneSmashEffect2, 0);
                                break;
                            }
                            else if((ball_x - stones[i][j].x - 132)*(ball_x - stones[i][j].x - 132) + (ball_y - stones[i][j].y - 30)*(ball_y - stones[i][j].y - 30) < r*r)
                            {
                                alpha = rand() % 80 - 95;
                                vx = v*cos(alpha * 3.14159265 / 180);
                                vy = -v*sin(alpha * 3.14159265 / 180);
                                ball_damage -= stones[i][j].strength;
                                stones[i][j].strength = -ball_damage;
                                if(ball_damage <= 0)
                                {
                                    player.level[player.num_of_played_rounds]++;
                                    ball_damage = player.level[player.num_of_played_rounds];
                                    new_line = 1;
                                }
                                if(stones[i][j].strength <= 0)
                                {
                                    player.score[player.num_of_played_rounds] += player.level[player.num_of_played_rounds];
                                    stones[i][j].strength = 0;
                                }
                                Mix_PlayChannel(3, stoneSmashEffect2, 0);
                                break;
                            }
                        }
                }
                ball_rect.x = ball_x - 25;
                ball_rect.y = ball_y - 21;
            }
            else
            {
                ball_rect.x = padx + 71;
                ball_rect.y = 712;
                ball_x = ball_rect.x;
                ball_y = ball_rect.y;
                SIN = (712 - my) / sqrt((mx - padx)*(mx - padx) + (my - 712)*(my - 712));
                COS = (mx - padx) / sqrt((mx - padx)*(mx - padx) + (my - 712)*(my - 712));
                vx = v*COS;
                vy = -v*SIN;
                draw_vector(render, padx + 96, 731, mx, my);
                if(event -> button.button == SDL_BUTTON_LEFT)
                    shoted = 1;
                Mix_PlayChannel(4, launchEffect, 0);
            }
            if(new_line)
            {
                new_line = 0;
                blanks = 0;
                for(int i = 0; i < 8; i++)
                    for(int j = 20; j >= 1; j--)
                        stones[i][j].strength = stones[i][j - 1].strength;
                for(int i = 0; i < 8; i++)
                {
                    pass = 0;
                    while(!pass)
                    {
                        stones[i][0].strength = rand() % (player.level[player.num_of_played_rounds] + 1);
                        if(stones[i][0].strength == 0 && blanks < 4)
                            pass = 1;
                        else if(stones[i][0].strength != 0)
                            pass = 1;
                    }
                    if(stones[i][0].strength == 0)
                        blanks++;
                }
                for(int i = 0; i < 8; i++)
                    if(stones[i][20].strength != 0)
                        lost = 1;
                ball_y += 35;
            }
        }
        launchpad_rect.x = padx;
        SDL_RenderCopy(render, launch_pad_t, NULL, &launchpad_rect);
        SDL_RenderCopy(render, ball_t, NULL, &ball_rect);
        draw_stones(render, stones, stone_rect);
        if(state[SDL_SCANCODE_F5])
        {
            save_game(stones,  ball_damage, vx, vy, ball_rect.x, ball_rect.y, shoted);
            player.saved = 1;
            write_player(player);
            paused = 0;
            SDL_RenderCopy(render, savegame_t, NULL, &savegame_rect);
            cout << "************************* Game saved successfully *************************" << endl;
            Mix_FreeMusic(bgm);
            Mix_PlayChannel(7, savegame, 0);
        }
        print_num_pic(render, player.score[player.num_of_played_rounds], 57, 778, -1);
        print_num_pic(render, player.level[player.num_of_played_rounds], 594, 778, -1);
        if(player.lives > 3)
            print_num_pic(render, player.lives, 1175, 778, -1);
        else
            print_hearts(render, player.lives);
        if(lost)
        {
            player.saved = 0;
            player.num_of_played_rounds++;
            write_player(player);
            SDL_RenderCopy(render, gameover_t, NULL, &gameover_rect);
            cout << "************************* You Lose! *************************" << endl;
            Mix_FreeMusic(bgm);
            Mix_PlayChannel(6, gameover, 0);
        }
        if(paused)
            SDL_RenderCopy(render, pause_t, NULL, &pause_rect);
        SDL_RenderPresent(render);
        SDL_Delay(5);
    }
    SDL_Delay(2000);
    gamesave_write.close();
    f_write.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(pause_t);
    SDL_DestroyTexture(stone_t);
    SDL_DestroyTexture(ball_t);
    SDL_DestroyTexture(launch_pad_t);
    SDL_DestroyTexture(background_t);
    SDL_DestroyTexture(savegame_t); SDL_DestroyTexture(gameover_t);
    SDL_DestroyTexture(Num_0); SDL_DestroyTexture(Num_1); SDL_DestroyTexture(Num_2); SDL_DestroyTexture(Num_3); SDL_DestroyTexture(Num_4);
    SDL_DestroyTexture(Num_6); SDL_DestroyTexture(Num_6); SDL_DestroyTexture(Num_7); SDL_DestroyTexture(Num_8); SDL_DestroyTexture(Num_9);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(render);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
void print_hearts(SDL_Renderer *render, int lives)
{
    heart_rect.x = 1110;
    heart_rect.y = 780;
    if(lives >= 1)
        SDL_RenderCopy(render, heart_t, NULL, &heart_rect);
    if(lives >= 2)
    {
        heart_rect.x += 30;
        SDL_RenderCopy(render, heart_t, NULL, &heart_rect);
    }
    if(lives == 3)
    {
        heart_rect.x += 30;
        SDL_RenderCopy(render, heart_t, NULL, &heart_rect);
    }
}
void print_num_pic(SDL_Renderer *render, int x, int num_x, int num_y, int l)
{
    int help[10], cnt = 0, len;
    if(x == 0)
    {
        cnt = 1;
        help[0] = 0;
    }
    else
    {
        while(x)
        {
            help[cnt++] = x % 10;
            x /= 10;
        }
    }
    if(l == -1)
        len = 0;
    else
        len = (l - 15*cnt)/2;
    for(int i = cnt - 1; i >= 0; i--)
    {
        if(help[i] == 0)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_0, NULL, &Num_rect);
        }
        else if(help[i] == 1)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_1, NULL, &Num_rect);
        }
        else if(help[i] == 2)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_2, NULL, &Num_rect);
        }
        else if(help[i] == 3)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_3, NULL, &Num_rect);
        }
        else if(help[i] == 4)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_4, NULL, &Num_rect);
        }
        else if(help[i] == 5)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_5, NULL, &Num_rect);
        }
        else if(help[i] == 6)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_6, NULL, &Num_rect);
        }
        else if(help[i] == 7)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_7, NULL, &Num_rect);
        }
        else if(help[i] == 8)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_8, NULL, &Num_rect);
        }
        else if(help[i] == 9)
        {

            Num_rect.x = num_x + len + 11*(cnt - i - 1);
            Num_rect.y = num_y;
            SDL_RenderCopy(render, Num_9, NULL, &Num_rect);
        }
    }
}
void save_game(stone stones[][21], int ball_damage, double vx, double vy, int x, int y, bool shoted)
{
    gamesave_write << x << endl << y << endl << vx << endl << vy << endl << ball_damage << endl << shoted << endl;
    for(int j = 0; j < 14; j++)
    {
        for(int i = 0; i < 8; i++)
            gamesave_write << stones[i][j].strength << " ";
        gamesave_write << endl;
    }
}
void load_saved_game(stone stones[][21], int &ball_damage, double &vx, double &vy, int &x, int &y, bool &shoted)
{
    gamesave_read >> x >> y >> vx >> vy >> ball_damage >> shoted;
    for(int j = 0; j < 14; j++)
        for(int i = 0; i < 8; i++)
            gamesave_read >> stones[i][j].strength;
}
void draw_vector(SDL_Renderer *render, int x, int y, int mx, int my)
{
    double SIN = (y - my) / sqrt((mx - x)*(mx - x) + (my - y)*(my - y));
    double COS = (mx - x) / sqrt((mx - x)*(mx - x) + (my - y)*(my - y));
    for(int i = 0; i <= 200; i += 20)
    {
        if(i % 40 == 0)
            thickLineRGBA(render, x + i*COS, y - i*SIN, x + (i + 20)*COS, y - (i + 20)*SIN, 5, 255, 0, 0, 255);
        else
            thickLineRGBA(render, x + i*COS, y - i*SIN, x + (i + 20)*COS, y - (i + 20)*SIN, 5, 255, 0, 0, 50);
    }
}
void draw_stone(SDL_Renderer *render, stone s, SDL_Rect stone_rect)
{
    stone_rect.x = s.x;
    stone_rect.y = s.y;
    SDL_RenderCopy(render, stone_t, NULL, &stone_rect);
    print_num_pic(render, s.strength, s.x, s.y + 5, 132);
}
void draw_stones(SDL_Renderer *render, stone s[][21], SDL_Rect stone_rect)
{
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 21; j++)
            if(s[i][j].strength != 0)
                draw_stone(render, s[i][j], stone_rect);
}
void save_score_level(user player, int score, int level)
{
    if(player.num_of_played_rounds > 5)
    {
        for(int i = 0; i < player.num_of_played_rounds - 1; i++)
        {
            player.score[i] = player.score[i + 1];
            player.level[i] = player.level[i + 1];
        }
        player.score[4] = score;
        player.level[4] = level;
    }
    else
    {
        player.score[player.num_of_played_rounds - 1] = score;
        player.level[player.num_of_played_rounds - 1] = level;
    }
    player.num_of_played_rounds++;
}
void write_player(user player)
{
    f_write << player.name << " " << player.saved << " " << player.num_of_played_rounds << " " << player.lives << " ";
    if(player.saved)
    {
        for(int i = 0; i <= player.num_of_played_rounds; i++)
            f_write << player.score[i] << " ";
        for(int i = 0; i <= player.num_of_played_rounds; i++)
            f_write << player.level[i] << " ";
    }
    else
    {
        for(int i = 0; i < player.num_of_played_rounds; i++)
            f_write << player.score[i] << " ";
        for(int i = 0; i < player.num_of_played_rounds; i++)
            f_write << player.level[i] << " ";
    }
}
void read_player(user &player)
{
    f_read >> player.name >> player.saved >> player.num_of_played_rounds >> player.lives;
    if(player.saved)
    {
        for(int i = 0; i <= player.num_of_played_rounds; i++)
            f_read >> player.score[i];
        for(int i = 0; i <= player.num_of_played_rounds; i++)
            f_read >> player.level[i];
    }
    else
    {
        for(int i = 0; i < player.num_of_played_rounds; i++)
            f_read >> player.score[i];
        for(int i = 0; i < player.num_of_played_rounds; i++)
            f_read >> player.level[i];
    }
    f_read.close();
}
void print_player_info(user player)
{
	cout << "Player: " << player.name << endl;
	for(int i = 0; i < player.num_of_played_rounds; i++)
	{
			cout << "Round #" << i + 1 << " score: " << player.score[i] << ", Level: " << player.level[i] << endl;
	}
	if(player.saved)
        cout << "Your current state -> " << " score: " << player.score[player.num_of_played_rounds] << ", Level: " << player.level[player.num_of_played_rounds] << endl;
	if(player.num_of_played_rounds == 0 && !player.saved)
        cout << "No records." << endl;
}

string space_remove(string s)
{
	string tmp = "";
	for(int i = 0; i < s.length(); i++)
	{
		if(s[i] != ' ' && s[i] != 9)
			tmp += s[i];
	}
	return tmp;
}







