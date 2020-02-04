#include "define.h"


// Global values

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;

const int TOTAL_TEXTURES = 4;
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 1000;
const int MENU_WIDTH = 200;
const int BLOCK_WIDTH = 50;
const int BLOCK_HEIGHT = 50;
const int CONTAIN_WIDTH = 5;
const int CONTAIN_HEIGHT = 5;
const int TOTAL_SHAPES = 7;
int mVelX = 0;
int mVelY = 5;

int dropX = 0;
int dropY = 0;
int rotate = 0;
bool instantDrop = false;
LTexture *gTextures[TOTAL_TEXTURES];

Mix_Music *gMusic = NULL;
//Mix_Chunk *soundEffect = NULL;

enum TextureTypes{
  SINGLE_BLOCK = 0,
  BACKGROUND = 1,
  SCORE_TEXT = 2,
  START_BUTTON = 3
};
// holds load block information
bool blockForms[TOTAL_SHAPES][8] = {
  {1,1,1,1,
   0,0,0,0},
  {1,1,1,0,
   0,0,1,0},
  {0,0,1,0,
   1,1,1,0},
  {1,1,0,0,
   0,1,1,0},
  {0,1,1,0,
   1,1,0,0},
  {0,1,0,0,
   1,1,1,0},
  {1,1,0,0,
   1,1,0,0}
};
// holds drop block information
bool container[CONTAIN_HEIGHT][CONTAIN_WIDTH] = {
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
};

const int TOTAL_BLOCK_HEIGHT = SCREEN_HEIGHT/BLOCK_HEIGHT;
const int TOTAL_BLOCK_WIDTH = SCREEN_WIDTH/BLOCK_WIDTH;

bool blockMap[SCREEN_HEIGHT/BLOCK_HEIGHT][SCREEN_WIDTH/BLOCK_WIDTH];

enum blockTypes{
  STRAIGHT = 0,
  FORWARD_L = 1,
  REVERSE_L = 2,
  FORWARD_Z = 3,
  REVERSE_Z = 4,
  NORMAL_T = 5
};

enum gameStates{
  GAME_OVER = 0,
  INIT_STATE = 1,
  LOAD_BLOCK = 2,
  DROP_BLOCK = 3
};


void toStringMap(){
  printf("Current Pos: (%d,%d)\n",dropX,dropY);
  for(int i = 0; i < TOTAL_BLOCK_HEIGHT ; i++){
    for(int j = 0; j < TOTAL_BLOCK_WIDTH; j++){
      printf("%d, ", blockMap[i][j]);
    }
    printf("\n");
  }
  printf("===================================\n");
}

void initializeMap(){
  for(int i = 0; i < TOTAL_BLOCK_HEIGHT ; i++){
    for(int j = 0; j < TOTAL_BLOCK_WIDTH; j++){
      blockMap[i][j] = false;
    }
  }
}

bool init(){
  
  /* Initialize:
   *  SDL
   *  Window
   *  Renderer
   *  Images
   */
  
  dropX = (TOTAL_BLOCK_WIDTH/2 - CONTAIN_WIDTH/2)*BLOCK_WIDTH;
  dropY = -CONTAIN_HEIGHT;

  if(SDL_Init(SDL_INIT_VIDEO)<0){printf("SDL Error: %s\n",SDL_GetError()); return false;}
   
  gWindow = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH+MENU_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  
  if(gWindow == NULL){printf("Window Error: %s\n",SDL_GetError()); return false;}

  gRenderer = SDL_CreateRenderer(gWindow,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(gRenderer == NULL){printf("Renderer Error %s\n",SDL_GetError()); return false;}

  SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

  int imgFlags = IMG_INIT_PNG;
  if(!(IMG_Init(imgFlags) & imgFlags)){printf("IMG Error %s\n",IMG_GetError()); return false;}

  if(TTF_Init() == -1){printf("TTF Error: %s\n",TTF_GetError()); return false;}
  
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,2048)<0){printf("SDL_Mixer Error: %s\n",Mix_GetError());return false;}
  
  initializeMap();
  return true;
}

bool loadMedia(){
  bool success = true;
  // Load block
 
  gTextures[BACKGROUND] = new LTexture(gRenderer);
  success &=gTextures[BACKGROUND]->loadFromFile("background.png");

  gTextures[SINGLE_BLOCK] = new LTexture(gRenderer);
  
  success &= gTextures[SINGLE_BLOCK]->loadFromFile("tetrisTile.png");
 
  gFont = TTF_OpenFont("BebasNeue-Regular.ttf",28);
  success &= gFont!=NULL;

  std::string s = "Score";
  gTextures[SCORE_TEXT] = new LTexture(gRenderer);
  success &= gTextures[SCORE_TEXT]->loadFromRenderedText( s, SDL_Color{1,1,1}, gFont);
  
  gTextures[START_BUTTON] = new LTexture(gRenderer);
  success &= gTextures[START_BUTTON]->loadFromFile("start_button.png");  
  
  gMusic = Mix_LoadMUS("Tetris 99 - Main Theme.mp3");
  success &= gMusic!=NULL;

  return success;
}

void close(){
  // close all textures 
  for(int i = 0; i < TOTAL_TEXTURES;i++){
    if(gTextures[i]!=NULL) gTextures[i]->free();
  }
  TTF_CloseFont(gFont);
  gFont = NULL;

  Mix_FreeMusic(gMusic);
  gMusic = NULL;
}

bool checkCollision(SDL_Rect a, SDL_Rect b){
  int leftA, leftB;
  int rightA, rightB;
  int topA, topB;
  int bottomA, bottomB;

  leftA = a.x;
  rightA = a.x + a.w;
  topA = a.y;
  bottomA = a.y + a.h;

  leftB = b.x;
  rightB = b.x + b.w;
  topB = b.y;
  bottomB = b.y + b.h;

  // Check extreme of one side of A exceeds the opposite in B
  if(bottomA <= topB) return false;
  if(topA >= bottomB) return false;
  if(rightA <= leftB) return false;
  if(leftA >= rightB) return false;

  return true;
}

void placeMap(SDL_Rect &camera,int &score, int state){
  
  // since the offsets accumulate, use DP for solving
  int offset[TOTAL_BLOCK_HEIGHT+1] = {0};
  bool delRow[TOTAL_BLOCK_HEIGHT] = {true};
  
  // from the bottom to the top
  for(int i = TOTAL_BLOCK_HEIGHT-1; i >= 0 ; i--){
    
    delRow[i] = true;
    // right to left
    for(int j = 0; j < TOTAL_BLOCK_WIDTH; j++){
      if(blockMap[i][j])
      { 
        gTextures[SINGLE_BLOCK]->render(j*BLOCK_WIDTH - camera.x,i*BLOCK_HEIGHT - camera.y);
      }
      else{
        delRow[i] &= false;
      }
    }
    
    // accumulate previous offset
    offset[i] += offset[i+1];

    // set the row for deletion
    if(delRow[i]) {
      // increase offset
      offset[i]++;
      score++;
    }

  }
 
 
  for(int i = TOTAL_BLOCK_HEIGHT-1; i >=0; i--){ 
    if(delRow[i]){
      for(int j = 0; j < TOTAL_BLOCK_WIDTH; j++){blockMap[i][j] = false;}
    }
    else{
      if(offset[i]){
        // printf("DELETE i= %d offset[%d]= %d\n",i,i,offset[i]); // DEBUG deletions
        for(int j = 0; j < TOTAL_BLOCK_WIDTH; j++){
          blockMap[i+offset[i]][j] = blockMap[i][j];
          blockMap[i][j] = false;
        }
      }
    }
  }

  /*
  if(state == LOAD_BLOCK){
    printf("Current Place Map: \n");
    toStringMap();
  }
*/
  updateScore(score);
}
int handleMouse(SDL_Event &e,int &state){
  if( e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP ){
		//Get mouse position
		int x, y;
		SDL_GetMouseState( &x, &y );

		//Check if mouse is in button
		bool inside = true;

    SDL_Rect start = {SCREEN_WIDTH/2-BLOCK_WIDTH,SCREEN_HEIGHT/3,100,50};
    
    inside &= x > start.x && x < start.x + start.w;
    inside &= y > start.y && y < start.y + start.h;
    
		//Mouse is outside button
		if( inside ){
			//Set mouse over sprite
			switch( e.type )
			{
				case SDL_MOUSEMOTION:
				break;	
				case SDL_MOUSEBUTTONDOWN:
				break;
				case SDL_MOUSEBUTTONUP:
				state = INIT_STATE;
        break;
			}
		}
	}

}


int handleInputs(SDL_Event &e,int &state){
  // If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 ){
    // Adjust the velocity
    switch( e.key.keysym.sym )
    {
      case SDLK_UP: instantDrop = true; break;
      case SDLK_DOWN: mVelY += BLOCK_HEIGHT/2; break;
      case SDLK_LEFT: mVelX -= BLOCK_WIDTH; break;
      case SDLK_RIGHT: mVelX += BLOCK_WIDTH; break;
      case SDLK_z: rotate--; break;
      case SDLK_x: rotate++; break;
    }
  }

  // If a key was released
  else if( e.type == SDL_KEYUP && e.key.repeat == 0 ){
    // Adjust the velocity
    switch( e.key.keysym.sym )
    {
      case SDLK_UP: instantDrop = false; break;
      case SDLK_DOWN: mVelY -= BLOCK_HEIGHT/2; break;
      case SDLK_LEFT: mVelX += BLOCK_WIDTH; break;
      case SDLK_RIGHT: mVelX -= BLOCK_WIDTH; break;
    }
  }
  else{
    handleMouse(e,state);
  }
  //printf("mVelX=%d,mVelY=%d,dropX=%d,dropY=%d",mVelX,mVelY,dropX,dropY);
}
void renderBlock(SDL_Rect &camera){
 for(int i = 0; i < CONTAIN_HEIGHT ; i++){
    for(int j = 0; j < CONTAIN_WIDTH; j++){
      if(container[i][j]){ 
        gTextures[SINGLE_BLOCK]->render(dropX+j*BLOCK_WIDTH - camera.x, dropY+i*BLOCK_HEIGHT - camera.y);
      }
    }
 }
}

int outsideScreen(SDL_Rect &camera){
  for(int i = 0; i < CONTAIN_HEIGHT ; i++){
    for(int j = 0; j < CONTAIN_WIDTH; j++){
      if(container[i][j]){
        if( (dropX + (j+1)*BLOCK_WIDTH) > SCREEN_WIDTH){
          return 1; 
        }
        if( (dropX + (j)*BLOCK_WIDTH) < 0){
          return 1;
        }
        if( (dropY + (i+1)*BLOCK_HEIGHT) > SCREEN_HEIGHT){
          return 2;
        }

      }
    }
  }
  return false;
}

bool collideMap(){
  
  // from the bottom to the top
  for(int i = TOTAL_BLOCK_HEIGHT-1; i >= 0 ; i--){
    // right to left
    for(int j = 0; j <TOTAL_BLOCK_WIDTH ; j++){
      
      if(blockMap[i][j]){
       
        // current block 
        SDL_Rect cur = {j*BLOCK_WIDTH, i*BLOCK_HEIGHT, BLOCK_WIDTH, BLOCK_HEIGHT};
        
        // if the block is within the vecinity of the dropping block
        if( cur.x >= dropX &&
            cur.x <= (dropX+CONTAIN_WIDTH*BLOCK_WIDTH) && 
            cur.y >= dropY &&
            cur.y <= (dropY+CONTAIN_HEIGHT*BLOCK_HEIGHT) ){
          
          // check if it intersects
          for(int k = 0; k < CONTAIN_HEIGHT; k++){
            for(int l = 0; l < CONTAIN_WIDTH; l++){

              if(container[k][l]){ 
                SDL_Rect A = {dropX+l*BLOCK_WIDTH, dropY+k*BLOCK_HEIGHT,BLOCK_WIDTH,BLOCK_HEIGHT}; 
                if(checkCollision(A,cur)) return true;
             
              }
            }
          }
        }

      }

    }
  }
 return false; 
}



int writeToMap(){
  for(int i = 0; i < CONTAIN_HEIGHT; i++){
    for(int j = 0; j < CONTAIN_WIDTH; j++){
      if(container[i][j]){
        int x =  (dropX+j*BLOCK_WIDTH)/BLOCK_WIDTH;
        int y =  (dropY+i*BLOCK_HEIGHT)/BLOCK_HEIGHT;

      //  printf("x = %d, y=%d\n",x,y);
        if(y<0) return GAME_OVER;
        else blockMap[y%TOTAL_BLOCK_HEIGHT][x%TOTAL_BLOCK_WIDTH] = true;
      }
    }
  }
  return LOAD_BLOCK;
/*
  printf("Block Written to Map\n");
  toStringMap();
  */
}

void rotateCCW(){ 
  for(int i = 0; i <CONTAIN_HEIGHT/2;i++){
    for(int j = i; j < CONTAIN_WIDTH-1-i;j++){
      bool temp = container[CONTAIN_WIDTH-1-j][i];        
      container[CONTAIN_WIDTH-1-j][i] = container[i][j];
      container[i][j] = container[j][CONTAIN_WIDTH-1-i];
      container[j][CONTAIN_WIDTH-1-i] = container[CONTAIN_WIDTH-1-i][CONTAIN_WIDTH-1-j]; 
      container[CONTAIN_WIDTH-1-i][CONTAIN_WIDTH-1-j] = temp; 
    }
  }
}

void rotateCW(){
  for(int i = 0; i <CONTAIN_HEIGHT/2;i++){
    for(int j = i; j <CONTAIN_WIDTH-1-i;j++){
      bool temp = container[j][CONTAIN_WIDTH-1-i];            // right: increasing column, decreasing row
      container[j][CONTAIN_WIDTH-1-i] = container[i][j];     // top: increasing column, increasing row
      container[i][j] = container[CONTAIN_WIDTH-1-j][i];     // left: decreasing column, increasing row
      container[CONTAIN_WIDTH-1-j][i] = container[CONTAIN_WIDTH-1-i][CONTAIN_WIDTH-1-j]; // bottom: decreasing column, decreasing row
      container[CONTAIN_WIDTH-1-i][CONTAIN_WIDTH-1-j] = temp; 
    }
  }
}

void dropping(SDL_Rect &camera,int &state){
  static int delay = 0;
  
  //  Try rotations 
  switch(rotate){
    case -1: rotateCCW(); break;
    case 1: rotateCW(); break;
  }

  if(outsideScreen(camera)==1 || collideMap()){
    switch(rotate){
      case 1: rotateCCW(); break;
      case -1: rotateCW(); break;
    }
  }
  rotate = 0;

  // Try X movement
  dropX += mVelX;
  if(outsideScreen(camera)==1 || collideMap()) dropX -= mVelX;
   
  // Try Y movement
  do{
    dropY += mVelY;
    
    if(outsideScreen(camera)==2 || collideMap()){
        if(delay>=24|| instantDrop){
        delay = 0;
        state = writeToMap();
      }
      else{
        dropY -= mVelY;
        delay++;
      }
      instantDrop = false;
    }

  }while(instantDrop);
}

void clearBlock(){
  // bug where the values dissapear from blockMap 
  for(int i = 0; i< CONTAIN_HEIGHT; i++){
    for(int j = 0; j< CONTAIN_WIDTH; j++){
      container[i][j] ^= container[i][j];
    }
  } 
}

void loadBlock(int nBlock){
  dropX = (TOTAL_BLOCK_WIDTH/2 - CONTAIN_WIDTH/2)*BLOCK_WIDTH;
  dropY = -CONTAIN_HEIGHT*BLOCK_HEIGHT;
  
  clearBlock();  

  for(int i = 0; i < 8;i++){
    container[1 + i/4][1+i%4] = blockForms[nBlock][i];
  }

}

bool updateScore(int& score){
  std::string s = "Score: " + std::to_string(score);
  return gTextures[SCORE_TEXT]->loadFromRenderedText( s, SDL_Color{0,0,0}, gFont);
}

void renderMini(std::queue<int> &q, SDL_Rect &camera, double scale, int offset){
  
  gTextures[SINGLE_BLOCK]->setScale(scale,scale);
  
  int accum= 2*offset;
 
  for(int j = 0; j < TOTAL_SHAPES; j++){
    int blockVal = q.front();
    q.pop();
    q.push(blockVal);
    for(int i = 0; i < 8; i++){
      if(blockForms[blockVal][i]){
        int x = (SCREEN_WIDTH+BLOCK_WIDTH)/scale +BLOCK_WIDTH/2 + (i%4)*BLOCK_WIDTH;
        int y = accum + ((int)(i/4)) * BLOCK_HEIGHT;
        gTextures[SINGLE_BLOCK]->render(x - camera.x,y - camera.y);
      }
    }
    accum += offset;
  }

  gTextures[SINGLE_BLOCK]->setScale(1,1);
}

int main(int argc, char *argv[]){
  if(!init()){ printf("Failed to Initialize!\n"); return 0;}
  if(!loadMedia()){printf("Failed to load Media\n"); return 0;}
 
  int score = 0;
  bool quit = false; 
  SDL_Event e;
  int state = GAME_OVER;
  
  int total_frames = 1;
  int frame = 0;

  SDL_Rect camera = {0,0,SCREEN_WIDTH, SCREEN_HEIGHT};
  
  std::queue<int> blockQueue;//blockForms
  
  while(blockQueue.size()<TOTAL_SHAPES){
    blockQueue.push(rand()%TOTAL_SHAPES);
  }
  while(!quit){ 
    
    while(SDL_PollEvent(&e)!=0){
      if(e.type == SDL_QUIT) quit = true;
      handleInputs(e,state);
    }
    // set background as white 
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF,0xFF,0xFF);
    SDL_RenderClear(gRenderer);

    switch(state){
      case(GAME_OVER):
        if(Mix_PlayingMusic) Mix_HaltMusic();
        break;
      case(INIT_STATE):
        // plays music (-1) forever or until halt
        Mix_PlayMusic(gMusic,-1);
        initializeMap();
        clearBlock();
        score = 0;
        mVelY = 5;
        state = LOAD_BLOCK;
        break;
      case(LOAD_BLOCK):
        loadBlock(blockQueue.front()); // for some reason this was screwing the top off, i commented and uncommented and it worked
       // toStringMap();
        blockQueue.pop();
        blockQueue.push(rand()%TOTAL_SHAPES);
        state = DROP_BLOCK;
        break;
      case(DROP_BLOCK): if(frame == total_frames) dropping(camera,state); break;
    };

    gTextures[BACKGROUND]->render(0,0);
    renderMini(blockQueue,camera,0.5, 3* BLOCK_HEIGHT);  
    gTextures[SCORE_TEXT]->render(SCREEN_WIDTH+BLOCK_WIDTH,BLOCK_HEIGHT);
    renderBlock(camera);
    placeMap(camera,score,state);
    
    if(state == GAME_OVER) 
      gTextures[START_BUTTON]->render(SCREEN_WIDTH/2-BLOCK_WIDTH,SCREEN_HEIGHT/3);
    
    SDL_RenderPresent(gRenderer);

    frame = frame == total_frames ? 0 : frame+1;
  }
  return 0;
}
