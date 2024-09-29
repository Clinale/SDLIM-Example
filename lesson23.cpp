/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/

//The headers
#include "SDLIM/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include <string>
// #include <stdlib.h>
// #include <string.h>

typedef unsigned short unicode;

// special key codes
#define  MG_KEY_F1                        0x0001
#define  MG_KEY_F2                        0x0002
#define  MG_KEY_F3                        0x0003
#define  MG_KEY_F4                        0x0004
#define  MG_KEY_F5                        0x0005
#define  MG_KEY_F6                        0x0006
#define  MG_KEY_F7                        0x0007
#define  MG_KEY_F8                        0x0008
#define  MG_KEY_F9                        0x0009
#define  MG_KEY_F10                       0x000A
#define  MG_KEY_F11                       0x000B
#define  MG_KEY_F12                       0x000C
#define  MG_KEY_LEFT                      0x0064
#define  MG_KEY_UP                        0x0065
#define  MG_KEY_RIGHT                     0x0066
#define  MG_KEY_DOWN                      0x0067
#define  MG_KEY_PAGE_UP                   0x0068
#define  MG_KEY_PAGE_DOWN                 0x0069
#define  MG_KEY_HOME                      0x006A
#define  MG_KEY_END                       0x006B
#define  MG_KEY_INSERT                    0x006C

#define  MG_KEY_RSHIFT                    0x0070
#define  MG_KEY_LSHIFT                    0x0071
#define  MG_KEY_RCTRL                     0x0072
#define  MG_KEY_LCTRL                     0x0073
#define  MG_KEY_RALT                      0x0074
#define  MG_KEY_LALT                      0x0075
#define  MG_KEY_RMETA                     0x0076
#define  MG_KEY_LMETA                     0x0077

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

//The surfaces
SDL_Surface *background = NULL;
SDL_Surface *message = NULL;
SDL_Surface *screen = NULL;

//The event structure
SDL_Event event;

//The font
TTF_Font *font = NULL;
//The font
TTF_Font *unicode_font = NULL;

//The color of the font
SDL_Color textColor = { 0xFF, 0xFF, 0xFF };


unsigned long utf8ToCodepoint(const unsigned char *p)
{
    unsigned long codepoint = 0;
    if ((*p & 0x80) == 0)
    {
        codepoint = *p;
        p++;
    }
    else if ((*p & 0xE0) == 0xC0)
    {
        codepoint = (*p & 0x1F) << 6;
        p++;
        codepoint |= (*p & 0x3F);
        p++;
    }
    else if ((*p & 0xF0) == 0xE0)
    {
        codepoint = (*p & 0x0F) << 12;
        p++;
        codepoint |= (*p & 0x3F) << 6;
        p++;
        codepoint |= (*p & 0x3F);
        p++;
    }
    else if ((*p & 0xF8) == 0xF0)
    {
        codepoint = (*p & 0x07) << 18;
        p++;
        codepoint |= (*p & 0x3F) << 12;
        p++;
        codepoint |= (*p & 0x3F) << 6;
        p++;
        codepoint |= (*p & 0x3F);
        p++;
    }
    else
    {
        printf("Not UTF-8: %c %d\n" ,(char)*p , (int)*p );
        p++;
    }
    return codepoint;
}


// #c---
/*****************************************************************************
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码.
 *
 * 参数:
 *    unic     字符的Unicode编码值
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针
 *    outsize  pOutput缓冲的大小
 *
 * 返回值:
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 .
 *
 * 注意:
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小!
 ****************************************************************************/
int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,
        int outSize)
{
    // assert(pOutput != NULL);
    // assert(outSize >= 6);
 
    if ( unic <= 0x0000007F )
    {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *pOutput     = (unic & 0x7F);
        return 1;
    }
    else if ( unic >= 0x00000080 && unic <= 0x000007FF )
    {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(pOutput+1) = (unic & 0x3F) | 0x80;
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    }
    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )
    {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(pOutput+2) = (unic & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    }
    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )
    {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+3) = (unic & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;
        return 4;
    }
    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )
    {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+4) = (unic & 0x3F) | 0x80;
        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;
        return 5;
    }
    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )
    {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+5) = (unic & 0x3F) | 0x80;
        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;
        return 6;
    }
 
    return 0;
}

void printfUnicode(unsigned long unic) {
    unsigned char tmp[8];
    int len = enc_unicode_to_utf8_one(unic, tmp, 8);
    printf("unicode: %04x\n", unic);
    puts("utf8 strings:");
    for (int i=0; i<len; i++){
        printf("%02x ", tmp[i]);
    }
    puts("");
    
    unsigned long u = utf8ToCodepoint(tmp);
    printf("utf82unicode: %04x\n", u);

}
int mapSDLSpecialKeyToMG( int inSDLKey ) {
    switch( inSDLKey ) {
        case SDLK_F1: return MG_KEY_F1;
        case SDLK_F2: return MG_KEY_F2;
        case SDLK_F3: return MG_KEY_F3;
        case SDLK_F4: return MG_KEY_F4;
        case SDLK_F5: return MG_KEY_F5;
        case SDLK_F6: return MG_KEY_F6;
        case SDLK_F7: return MG_KEY_F7;
        case SDLK_F8: return MG_KEY_F8;
        case SDLK_F9: return MG_KEY_F9;
        case SDLK_F10: return MG_KEY_F10;
        case SDLK_F11: return MG_KEY_F11;
        case SDLK_F12: return MG_KEY_F12;
        case SDLK_LEFT: return MG_KEY_LEFT;
        case SDLK_UP: return MG_KEY_UP;
        case SDLK_RIGHT: return MG_KEY_RIGHT;
        case SDLK_DOWN: return MG_KEY_DOWN;
        case SDLK_PAGEUP: return MG_KEY_PAGE_UP;
        case SDLK_PAGEDOWN: return MG_KEY_PAGE_DOWN;
        case SDLK_HOME: return MG_KEY_HOME;
        case SDLK_END: return MG_KEY_END;
        case SDLK_INSERT: return MG_KEY_INSERT;
        case SDLK_RSHIFT: return MG_KEY_RSHIFT;
        case SDLK_LSHIFT: return MG_KEY_LSHIFT;
        case SDLK_RCTRL: return MG_KEY_RCTRL;
        case SDLK_LCTRL: return MG_KEY_LCTRL;
        case SDLK_RALT: return MG_KEY_RALT;
        case SDLK_LALT: return MG_KEY_LALT;
        case SDLK_RMETA: return MG_KEY_RMETA;
        case SDLK_LMETA: return MG_KEY_LMETA;
        default: return 0;
    }
}

//The key press interpreter
class StringInput
{
    private:
    //The storage string
    std::string str;

    unicode buffer[16];
    int buffer_len;

    //The text surface
    SDL_Surface *text;

    public:
    //Initializes variables
    StringInput();

    //Does clean up
    ~StringInput();

    //Handles input
    void handle_input();

    //Shows the message on screen
    void show_centered();
};

SDL_Surface *load_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool init()
{
    SDL_SetIMValues(SDL_IM_ENABLE, 1, NULL);
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    
    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        return false;
    }

    //Set the window caption
    SDL_WM_SetCaption( "High Score", NULL );

    //If everything initialized fine
    return true;
}

bool load_files()
{
    //Load the background image
    background = load_image( "background.png" );

    //Open the font
    font = TTF_OpenFont( "lazy.ttf", 42 );

    unicode_font = TTF_OpenFont("font.ttf", 42);
    //If there was a problem in loading the background
    if( background == NULL )
    {
        return false;
    }

    //If there was an error in loading the font
    if( font == NULL || unicode_font == NULL)
    {
        return false;
    }

    //If everything loaded fine
    return true;
}

void clean_up()
{
    //Free the surfaces
    SDL_FreeSurface( background );
    SDL_FreeSurface( message );

    
    //Close the font that was used
    TTF_CloseFont( font );

    //Quit SDL_ttf
    TTF_Quit();

    //Quit SDL
    SDL_Quit();
}

StringInput::StringInput()
{
    //Initialize the string
    str = "";

    //Initialize the surface
    text = NULL;

    buffer[0] = 0;
    buffer_len = 0;

    //Enable Unicode
    //SDL_EnableUNICODE( SDL_ENABLE );

    //SDL_SetIMValues(SDL_IM_ENABLE, 1, NULL);
    //SDL_SetIMValues(SDL_IM_ONOFF, 1, NULL);

    SDL_EnableUNICODE( SDL_ENABLE );

}

StringInput::~StringInput()
{
    //Free text surface
    SDL_FreeSurface( text );

   //SDL_SetIMValues(SDL_IM_ONOFF, 0, NULL);
   //SDL_SetIMValues(SDL_IM_ENABLE, 0, NULL);

    //Disable Unicode
    SDL_EnableUNICODE( SDL_DISABLE );
}

void StringInput::handle_input()
{
    //If a key was pressed
    if( event.type == SDL_KEYDOWN )
    {
        //Keep a copy of the current version of the string
        //std::string temp = str;
        int tmp_len = buffer_len;
        //If the string less than maximum size
        printf("\n----\nbuffer_len: %d\n", buffer_len);
        if (buffer_len < 16)
        //if( str.length() <= 16 )
        {
            int mgKey = mapSDLSpecialKeyToMG( event.key.keysym.sym );
            if ( (event.key.keysym.sym != SDLK_BACKSPACE ) && mgKey == 0) {
                printf("scancode: %x  sym: %x\n", event.key.keysym.scancode, event.key.keysym.sym);
                // printf("unicode: %x\n", event.key.keysym.unicode);
                printfUnicode(event.key.keysym.unicode);
                buffer[buffer_len] = event.key.keysym.unicode;
                buffer[++buffer_len] = 0;
            }
            /*
            printf("scancode: %x  sym: %x\n", event.key.keysym.scancode, event.key.keysym.sym);
            printf("unicode: %x\n", event.key.keysym.unicode);
            //If the key is a space
            if( event.key.keysym.unicode == (Uint16)' ' )
            {
                //Append the character
                str += (char)event.key.keysym.unicode;
            }
            //If the key is a number
            else if( ( event.key.keysym.unicode >= (Uint16)'0' ) && ( event.key.keysym.unicode <= (Uint16)'9' ) )
            {
                //Append the character
                str += (char)event.key.keysym.unicode;
            }
            //If the key is a uppercase letter
            else if( ( event.key.keysym.unicode >= (Uint16)'A' ) && ( event.key.keysym.unicode <= (Uint16)'Z' ) )
            {
                //Append the character
                str += (char)event.key.keysym.unicode;
            }
            //If the key is a lowercase letter
            else if( ( event.key.keysym.unicode >= (Uint16)'a' ) && ( event.key.keysym.unicode <= (Uint16)'z' ) )
            {
                //Append the character
                str += (char)event.key.keysym.unicode;
            }
            if(event.key.keysym.scancode == 0 && event.key.keysym.sym == 0) {
                int len = SDL_FlushIMString(NULL);
                printf("len: %d\n", len);
                if(len > 0) {
                    int flag = 0;
                    unsigned short *buff = (unsigned short *)malloc((len + 1)*2); 
                    SDL_FlushIMString(buff);
                    for (int i=0; i<len; i++){
                        printf("%04x ", buff[i]);
                    }
                    printf("\n");
                    free(buff);
                }
            }
            */
        }
        
        //If backspace was pressed and the string isn't blank
        if( ( event.key.keysym.sym == SDLK_BACKSPACE ) && ( buffer_len > 0 ) )
        {
            //Remove a character from the end
            //str.erase( str.length() - 1 );
            buffer_len -= 1;
            buffer[buffer_len] = 0;
            printf("remove a character from the end: %d\n", buffer_len);
        }
        printf("after buffer len: %d\n---\n", buffer_len);
        //If the string was changed
        if (buffer_len != tmp_len)
        //if( str != temp )
        {
            printf("the string was changed\n");
            //Free the old surface
            SDL_FreeSurface( text );

            //Render a new text surface
            //text = TTF_RenderText_Solid( font, str.c_str(), textColor );
            text = TTF_RenderUNICODE_Blended(unicode_font, buffer, textColor);
        }
    }
}

void StringInput::show_centered()
{
    //If the surface isn't blank
    if( text != NULL )
    {
        //Show the name
        apply_surface( ( SCREEN_WIDTH - text->w ) / 2, ( SCREEN_HEIGHT - text->h ) / 2, text, screen );
    }
}

int main( int argc, char* args[] )
{
    //Quit flag
    bool quit = false;

    //Keep track if whether or not the user has entered their name
    bool nameEntered = false;

    //Initialize
    if( init() == false )
    {
        return 1;
    }

    //The gets the user's name
    StringInput name;

    //Load the files
    if( load_files() == false )
    {
        return 1;
    }

    
    //Set the message
    message = TTF_RenderText_Solid( font, "New High Score! Enter Name:", textColor );

    //While the user hasn't quit
    while( quit == false )
    {
        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //If the user hasn't entered their name yet
            if( nameEntered == false )
            {
                //Get user input
                name.handle_input();

                //If the enter key was pressed
                if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_RETURN ) )
                {
                    //Change the flag
                    nameEntered = true;

                    //Free the old message surface
                    SDL_FreeSurface( message );

                    //Change the message
                    message = TTF_RenderText_Solid( font, "Rank: 1st", textColor );
                }
            }

            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }

        //Apply the background
        apply_surface( 0, 0, background, screen );

        //Show the message
        apply_surface( ( SCREEN_WIDTH - message->w ) / 2, ( ( SCREEN_HEIGHT / 2 ) - message->h ) / 2, message, screen );

        //Show the name on the screen
        name.show_centered();

        //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }
    }

    //Clean up
    clean_up();

    return 0;
}
