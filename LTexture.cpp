#include "define.h"

LTexture::LTexture(SDL_Renderer* renderer)
{
  lRenderer = renderer;
  mTexture = NULL;
  mWidth = 0;
  mHeight = 0;
}

LTexture::~LTexture()
{
  free();
}

bool LTexture::loadFromFile(std::string path)
{
  free();
  
  SDL_Texture* newTexture = NULL;
  
  // Create Surface from Image
  SDL_Surface* loadedSurface = IMG_Load(path.c_str());
  if(loadedSurface==NULL){printf("SDL_image Error: %s\n",IMG_GetError()); return 0;}

  // sets the transparent background color
  SDL_SetColorKey(loadedSurface,SDL_TRUE, SDL_MapRGB(loadedSurface->format,0,0xFF,0xFF));
  
  // Create Texture from Surface
  newTexture = SDL_CreateTextureFromSurface(lRenderer,loadedSurface);
  SDL_FreeSurface(loadedSurface); // Surface is no longer needed, put before next check
  
  if(newTexture == NULL){
    printf("SDL Error: %s\n",SDL_GetError());
    return 0;
  }
  else{
    mWidth = loadedSurface->w;
    mHeight = loadedSurface->h; 
    mTexture = newTexture;
    return 1;
  }
}

#if defined(_SDL_TTF_H) || defined(SDL_TTF_H)
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font *loadFont)
{
	free();

	SDL_Surface* textSurface = TTF_RenderText_Solid( loadFont, textureText.c_str(), textColor );
	if( textSurface == NULL ){ printf("SDL_ttf Error: %s\n",TTF_GetError() ); return 0;}
  
  mTexture = SDL_CreateTextureFromSurface( lRenderer, textSurface ); 
  SDL_FreeSurface( textSurface );
  if( mTexture == NULL ){ 
    printf("SDL Error: %s\n", SDL_GetError() ); 
    return 0;
  }
  else{
    mWidth = textSurface->w;
	  mHeight = textSurface->h; 
    return 1;
  }

}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue ){
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending ){
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha ){
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::setScale(float x, float y){
  SDL_RenderSetScale( lRenderer,x,y); 
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip ){
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	//Render to screen
	SDL_RenderCopyEx( lRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth(){return mWidth;}
int LTexture::getHeight(){return mHeight;}
