#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "pngwrite/IMG_savepng.h"


SDL_Surface *tile;
SDL_Surface *sc;

int ccolor=0;
int ch,cs,cl;

// http://qscribble.blogspot.com/2008/06/integer-conversion-from-hsl-to-rgb.html
uint32_t HSL(int hue, int sat, int lum)
{
    int v,r,g,b;

    v = (lum < 128) ? (lum * (256 + sat)) >> 8 :
          (((lum + sat) << 8) - lum * sat) >> 8;
    if (v <= 0) {
        r = g = b = 0;
    } else {
        int m;
        int sextant;
        int fract, vsf, mid1, mid2;

        m = lum + lum - v;
        hue *= 6;
        sextant = hue >> 8;
        fract = hue - (sextant << 8);
        vsf = v * fract * (v - m) / v >> 8;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant) {
           case 0: r = v; g = mid1; b = m; break;
           case 1: r = mid2; g = v; b = m; break;
           case 2: r = m; g = v; b = mid1; break;
           case 3: r = m; g = mid2; b = v; break;
           case 4: r = mid1; g = m; b = v; break;
           case 5: r = v; g = m; b = mid2; break;
        }
    }
    return SDL_MapRGBA(sc->format,r,g,b,0xff);
}


int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	sc=SDL_SetVideoMode(800,600,32,SDL_HWSURFACE|SDL_DOUBLEBUF);

	if(argc>1) {
		tile=IMG_Load(argv[1]);
	} else {
		tile=SDL_CreateRGBSurface(SDL_SWSURFACE,32,32,8,0,0,0,0);
		SDL_Color colors[256];
		int i; for(i=0;i<256;i++){
			colors[i].r=i;
			colors[i].g=i;
			colors[i].b=i;
		}
		SDL_SetPalette(tile,SDL_LOGPAL|SDL_PHYSPAL,colors,0,256);
	}

	for(;;) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type==SDL_QUIT) goto end;
			if(e.type==SDL_KEYDOWN) goto end;
			if(e.type==SDL_MOUSEBUTTONDOWN) {
				int x=e.button.x,y=e.button.y;
				if(x<256 && y<256) {
					SDL_Rect r={8*(x/8),8*(y/8),8,8};
					SDL_FillRect(sc,&r,0x0000ff);
					SDL_ShowCursor(0);
				} else if(x>256+32 && x<2*256+32 && y<256) {
					ccolor=(y/16)*16 +((x-256-32)/16);
				}
			}
		}

		int x,y;
		SDL_GetMouseState(&x,&y);
		SDL_FillRect(sc,0,0x0);

		{
			SDL_Rect r={0,0,256,256};
			SDL_FillRect(sc,&r,0xffffff);
		}

		{
			SDL_Rect r={256+32,0,256,256};
			SDL_FillRect(sc,&r,0xffffff);
			int c;
			int ocolor=(y/16)*16 +((x-256-32)/16);
			for(c=0;c<tile->format->palette->ncolors;c++) {
				SDL_Rect s={256+32+16*(c%16)+1,16*(c/16)+1,15,15};
				uint8_t r,g,b,a;
				SDL_GetRGBA(c,tile->format,&r,&g,&b,&a);
				SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,r,g,b,a));
			}

			{
				SDL_Rect s={256+32+16*(ccolor%16)-8,16*(ccolor/16)-8,16+16,16+16};
				uint8_t r,g,b,a;
				SDL_GetRGBA(ccolor,tile->format,&r,&g,&b,&a);
				SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,255,0,0,a));
				s.x+=1; s.y+=1; s.w-=2; s.h-=2;
				SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,r,g,b,a));
			}

			if(ocolor>=0 && ocolor<256) {
				SDL_Rect s={256+32+16*(ocolor%16)-8,16*(ocolor/16)-8,16+16,16+16};
				uint8_t r,g,b,a;
				SDL_GetRGBA(ocolor,tile->format,&r,&g,&b,&a);
				SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,255,255,0,a));
				s.x+=1; s.y+=1; s.w-=2; s.h-=2;
				SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,r,g,b,a));
			}

		}

		{
			SDL_Rect a={256+32,256+32,256,256};
			SDL_FillRect(sc,&a,0xffffff);

			SDL_Rect r={256+32,256+32,1,32};
			int i; for(i=0;i<256;i++) {
				SDL_FillRect(sc,&r,HSL(i,255,128));
				r.x++;
			}
		}

		if(x<256 && y<256) {
			SDL_Rect r={8*(x/8),8*(y/8),8,8};
			SDL_FillRect(sc,&r,0x0000ff);
			SDL_ShowCursor(0);
		} else {
			SDL_ShowCursor(1);
		}

		SDL_Flip(sc);
	}




end:	SDL_Quit();
	return 0;
}

