#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "pngwrite/IMG_savepng.h"


SDL_Surface *tile=0;
SDL_Surface *sc;

int ccolor=0;
int ch=0,cs=0,cl=128;

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

float max(float x, float y) { return x>y?x:y; }
float min(float x, float y) { return x<y?x:y; }

// http://www.student.kuleuven.be/~m0216922/CG/color.html#RGB_to_HSL_
void RGB(uint8_t r0, uint8_t g0, uint8_t b0, int *hue, int *sat, int *lum) {
    float r, g, b, h, s, l;
    r = r0 / 256.0; 
    g = g0 / 256.0; 
    b = b0 / 256.0; 
    float maxColor = max(r, max(g, b)); 
    float minColor = min(r, min(g, b));
    if(minColor==maxColor) {   
        h = 0.0; //it doesn't matter what value it has       
        s = 0.0;       
        l = r; //doesn't matter if you pick r, g, or b   
    } else {
        l = (minColor + maxColor) / 2;     
        
        if(l < 0.5) s = (maxColor - minColor) / (maxColor + minColor);
        else s = (maxColor - minColor) / (2.0 - maxColor - minColor);
        
        if(r == maxColor) h = (g - b) / (maxColor - minColor);
        else if(g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
        else h = 4.0 + (r - g) / (maxColor - minColor);
        
        h /= 6; //to bring it to a number between 0 and 1
        if(h < 0) h ++;
    }

    *hue=h*255.0;
    *sat=s*255.0;
    *lum=l*255.0;
}

SDL_Color colors[256];

int mx,my;

void display_small_tiles() {
	SDL_Rect r={0,256+32,32,32};
	int i,j;
	for(i=0;i<8;i++) {
		for(j=0;j<8;j++) {
			SDL_BlitSurface(tile,0,sc,&r);
			if(mx<256 && my<256) {
				SDL_Rect s={r.x+mx/8,r.y+my/8,1,1};
				uint8_t r,g,b,a;
				SDL_GetRGBA(ccolor,tile->format,&r,&g,&b,&a);
				SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,r,g,b,a));
			}
			r.x+=32;
		}
		r.x-=256;
		r.y+=32;
	}

	if(mx<256 && my<256) {
		int i;
		for(i=0;i<8;i++) {
			SDL_Rect r={mx/8+i*32,256+32-2,1,2};
			SDL_FillRect(sc,&r,SDL_MapRGBA(sc->format,255,255,255,255));

			SDL_Rect s={256,256+32+my/8+32*i,2,1};
			SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,255,255,255,255));
		}
	}
}

void display_big_image() {
	int i,j;
	SDL_Rect s={0,0,8,8};
	SDL_LockSurface(tile);
	for(i=0;i<32;i++) {
		for(j=0;j<32;j++) {
			uint8_t r,g,b,a;
			SDL_GetRGBA(((uint8_t *)(tile->pixels))[j+i*32],tile->format,&r,&g,&b,&a);
			SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,r,g,b,a));
			s.x+=8;
		}
		s.x-=256;
		s.y+=8;
	}
	SDL_UnlockSurface(tile);
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	sc=SDL_SetVideoMode(800,600,32,SDL_HWSURFACE|SDL_DOUBLEBUF);

	if(argc>1) {
		tile=IMG_Load(argv[1]);
	}

	if(!tile) {
		tile=SDL_CreateRGBSurface(SDL_SWSURFACE,32,32,8,0,0,0,0);
		int i; for(i=0;i<256;i++){
			colors[i].r=i;
			colors[i].g=i;
			colors[i].b=i;
		}
		SDL_SetPalette(tile,SDL_LOGPAL|SDL_PHYSPAL,colors,0,256);
	} else {
		int i; for(i=0;i<tile->format->palette->ncolors;i++){
			colors[i]=tile->format->palette->colors[i];
		}
	}

	{
		uint8_t r,g,b,a;
		SDL_GetRGBA(0,tile->format,&r,&g,&b,&a);
		RGB(r,g,b,&ch,&cs,&cl);
	}

	for(;;) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type==SDL_QUIT) goto end;
			if(e.type==SDL_KEYDOWN) {
				if(e.key.keysym.sym==SDLK_s) {
					IMG_SavePNG(argv[1],tile,0);
				}
			}
			if(e.type==SDL_MOUSEBUTTONDOWN) {
				int x=e.button.x,y=e.button.y;
				if(x<256 && y<256) {
					SDL_Rect r={x/8,y/8,1,1};
					SDL_FillRect(tile,&r,ccolor);
					SDL_ShowCursor(0);
				} else if(x>256+32 && x<2*256+32 && y<256) {
					ccolor=(y/16)*16 +((x-256-32)/16);
					uint8_t r,g,b,a;
					SDL_GetRGBA(ccolor,tile->format,&r,&g,&b,&a);
					RGB(r,g,b,&ch,&cs,&cl);
				} else if(x>256+32 && x<256+32+256 && y>256+32 && y<256+64) {
					ch=x-256-32;
					uint32_t c=HSL(ch,cs,cl);
					colors[ccolor].r=(c>>16)&0xff;
					colors[ccolor].g=(c>> 8)&0xff;
					colors[ccolor].b=(c    )&0xff;
					SDL_SetPalette(tile,SDL_LOGPAL|SDL_PHYSPAL,colors,0,256);
				} else if(x>256+32 && x<256+32+256 && y>256+32+32 && y<256+32+32+256) {
					cs=x-256-32;
					cl=y-256-32-32;
					uint32_t c=HSL(ch,cs,cl);
					colors[ccolor].r=(c>>16)&0xff;
					colors[ccolor].g=(c>> 8)&0xff;
					colors[ccolor].b=(c    )&0xff;
					SDL_SetPalette(tile,SDL_LOGPAL|SDL_PHYSPAL,colors,0,256);
				}
			}
		}

		int x,y;
		SDL_GetMouseState(&x,&y);
		mx=x; my=y;
		SDL_FillRect(sc,0,0x0);

		{
			SDL_Rect r={0,0,256,256};
			SDL_FillRect(sc,&r,0xffffff);
		}

		display_small_tiles();
		display_big_image();

		{
			SDL_Rect r={256+32,0,256,256};
			SDL_FillRect(sc,&r,0xffffff);
			int c;
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

			int ocolor=(y/16)*16 +((x-256-32)/16);
			if(x<256+32 || x>=256*2+32) ocolor=-1;
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

			{	SDL_Rect r={256+32,256+32,1,32};
				int i; for(i=0;i<256;i++) {
					SDL_FillRect(sc,&r,HSL(i,255,128));
					r.x++;
				}
			}

			{
				SDL_Rect s={256+32+ch-4,256+32,8,32};
				SDL_FillRect(sc,&s,HSL(0xff&(ch+128),255,128));
				s.x+=1; s.y+=1; s.w-=2; s.h-=2;
				SDL_FillRect(sc,&s,HSL(ch,255,128));
			}

			{	SDL_Rect r={256+32,256+32+32,1,1};
				int i,j;
				for(i=0;i<256;i++) {
					for(j=0;j<256;j++) {
						SDL_FillRect(sc,&r,HSL(ch,j,i));
						r.x++;
					}
					r.y++;
					r.x-=256;
				}

				SDL_Rect s={256+32+cs-4,256+32+32+cl-4,8,8};
				SDL_FillRect(sc,&s,HSL(0xff&(ch+128),255-cs,255-cl));
				s.x+=1; s.y+=1; s.w-=2; s.h-=2;
				SDL_FillRect(sc,&s,HSL(ch,cs,cl));
			}
		}

		if(x<256 && y<256) {
			SDL_Rect s={8*(x/8),8*(y/8),8,8};
			uint8_t r,g,b,a;
			SDL_GetRGBA(ccolor,tile->format,&r,&g,&b,&a);
			SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,~r,~g,~b,a));
			s.x+=1; s.y+=1; s.w-=2; s.h-=2;
			SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,r,g,b,a));
			SDL_ShowCursor(0);
		} else {
			SDL_ShowCursor(1);
		}

		SDL_Flip(sc);
	}




end:	SDL_Quit();
	return 0;
}

