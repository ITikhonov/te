#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "pngwrite/IMG_savepng.h"

struct tile { struct color { uint8_t r,g,b,a; } c[255]; uint8_t p[32*32]; } tiles[400];

struct tile *tile=0;
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

int mx,my;

void display_big_image() {
	int i,j;
	SDL_Rect s={0,0,8,8};
	for(i=0;i<32;i++) {
		for(j=0;j<32;j++) {
			struct color *c=tile->c+tile->p[i*32+j];
			SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,c->r,c->g,c->b,c->a));
			s.x+=8;
		}
		s.x-=256;
		s.y+=8;
	}
}

#if 0

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


#endif

void display_palette() {
	SDL_Rect r={256+32,0,256,256};
	SDL_FillRect(sc,&r,0xffffff);
	int c;
	for(c=0;c<256;c++) {
		SDL_Rect s={256+32+16*(c%16)+1,16*(c/16)+1,15,15};
		struct color *d=tile->c+c;
		SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,d->r,d->g,d->b,d->a));
	}
}

void display_palette_current_color()
{
	SDL_Rect s={256+32+16*(ccolor%16)-8,16*(ccolor/16)-8,16+16,16+16};
	SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,255,0,0,0xff));
	s.x+=1; s.y+=1; s.w-=2; s.h-=2;

	struct color *d=tile->c+ccolor;
	SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,d->r,d->g,d->b,d->a));
}


void display_palette_hover_color()
{
	if(mx<256+32 || mx>=256*2+32 || my>256) return;
	int ocolor=(my/16)*16 +((mx-256-32)/16);
	SDL_Rect s={256+32+16*(ocolor%16)-8,16*(ocolor/16)-8,16+16,16+16};

	SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,255,255,0,0xff));
	s.x+=1; s.y+=1; s.w-=2; s.h-=2;

	struct color *d=tile->c+ocolor;
	SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,d->r,d->g,d->b,d->a));
}

void display_hue_bar()
{
	SDL_Rect r={256+32,256+32,1,32};
	int i; for(i=0;i<256;i++) {
		SDL_FillRect(sc,&r,HSL(i,255,128));
		r.x++;
	}
}

void display_hue_current()
{
	SDL_Rect s={256+32+ch-4,256+32,8,32};
	SDL_FillRect(sc,&s,HSL(0xff&(ch+128),255,128));
	s.x+=1; s.y+=1; s.w-=2; s.h-=2;
	SDL_FillRect(sc,&s,HSL(ch,255,128));
}


void display_satlum_rect()
{
	SDL_Rect r={256+32,256+32+32,1,1};
	int i,j;
	for(i=0;i<256;i++) {
		for(j=0;j<256;j++) {
			SDL_FillRect(sc,&r,HSL(ch,j,i));
			r.x++;
		}
		r.y++;
		r.x-=256;
	}
}

void display_satlum_current()
{
	SDL_Rect s={256+32+cs-4,256+32+32+cl-4,8,8};
	SDL_FillRect(sc,&s,HSL(0xff&(ch+128),255-cs,255-cl));
	s.x+=1; s.y+=1; s.w-=2; s.h-=2;
	SDL_FillRect(sc,&s,HSL(ch,cs,cl));
}

void display_draw_cursor() {
	if(mx<256 && my<256) {
		SDL_Rect s={8*(mx/8),8*(my/8),8,8};

		struct color *d=tile->c+ccolor;
		SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,~d->r,~d->g,~d->b,d->a));
		s.x+=1; s.y+=1; s.w-=2; s.h-=2;
		SDL_FillRect(sc,&s,SDL_MapRGBA(sc->format,d->r,d->g,d->b,d->a));
		SDL_ShowCursor(0);
	} else {
		SDL_ShowCursor(1);
	}
}

void display_tile(int x, int y, struct tile *t) {
	uint32_t pixels[32*32];
	int i;
	for(i=0;i<32*32;i++) {
		struct color *c=t->c+t->p[i];
		pixels[i]=SDL_MapRGBA(sc->format,c->r,c->g,c->b,128);
	}

	SDL_Surface *s=SDL_CreateRGBSurfaceFrom(pixels,32,32,32,32*4,
				sc->format->Rmask,sc->format->Gmask,sc->format->Bmask,sc->format->Amask);

	SDL_Rect r={x,y,32,32};
	SDL_FillRect(sc,&r,0xffff00);
	SDL_BlitSurface(s,0,sc,&r);
	SDL_FreeSurface(s);
}

void display_tiles() {
	int i;
	for(i=0;i<400;i++) {
		display_tile(256*2+32*2,i*32,tiles+i);
	}
}

void select_color() {
	if(mx>256+32 && mx<2*256+32 && my<256) {
		ccolor=(my/16)*16 +((mx-256-32)/16);
		struct color *d=tile->c+ccolor;
		RGB(d->r,d->g,d->b,&ch,&cs,&cl);
	}
}

void select_hue() {
	if(mx>256+32 && mx<256+32+256 && my>256+32 && my<256+64) {
		ch=mx-256-32;
		uint32_t c=HSL(ch,cs,cl);
		tile->c[ccolor].r=(c>>16)&0xff;
		tile->c[ccolor].g=(c>> 8)&0xff;
		tile->c[ccolor].b=(c    )&0xff;
	}
}

void select_satlum() {
	if(mx>256+32 && mx<256+32+256 && my>256+32+32 && my<256+32+32+256) {
		cs=mx-256-32;
		cl=my-256-32-32;
		uint32_t c=HSL(ch,cs,cl);
		tile->c[ccolor].r=(c>>16)&0xff;
		tile->c[ccolor].g=(c>> 8)&0xff;
		tile->c[ccolor].b=(c    )&0xff;
	}
}


void set_edit_tile(int n) {
	tile=tiles+n;
	ccolor=0;
}

void select_tile() {
	if(mx>256*2+32*2) {
		set_edit_tile(my/32);
	}
}

uint8_t fill_palette(struct color *p, uint8_t r, uint8_t g, uint8_t b) {
	int i,e=0;
	for(i=1;i<256;i++) {
		struct color *c=p+i;
		if(c->a==0 && !e) {e=i; continue; }
		if(c->r==r && c->g==g && c->b==b && c->a!=0) return i;
	}

	p[e].r=r;
	p[e].g=g;
	p[e].b=b;
	p[e].a=0xff;
	return e;
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	sc=SDL_SetVideoMode(800,600,32,SDL_HWSURFACE|SDL_DOUBLEBUF);

	if(argc>1) {
		SDL_Surface *img=IMG_Load(argv[1]);
		int i,j;
		SDL_LockSurface(img);
		for(i=0;i<400;i++) {
			memset(tiles[i].c,0,sizeof(tiles[i].c));
			for(j=0;j<32*32;j++) {
				uint32_t c=((uint32_t*)(img->pixels))[
					(i%20)*32 + (j%32) + ((i/20)*32 + j/32)*640
				];
				uint8_t r=(c&img->format->Rmask)>>(img->format->Rshift),
					g=(c&img->format->Gmask)>>(img->format->Gshift),
					b=(c&img->format->Bmask)>>(img->format->Bshift),
					a=(c&img->format->Amask)>>(img->format->Ashift);

				tiles[i].p[j]=a?fill_palette(tiles[i].c,r,g,b):0;
			}
		}
		SDL_UnlockSurface(img);
	}

	set_edit_tile(0);

	for(;;) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type==SDL_QUIT) goto end;
			if(e.type==SDL_KEYDOWN) {
				if(e.key.keysym.sym==SDLK_s) {
					//IMG_SavePNG(argv[1],tile,0);
				}
			}
			if(e.type==SDL_MOUSEBUTTONDOWN) {
				mx=e.button.x;
				my=e.button.y;

				select_color();
				select_hue();
				select_satlum();
				select_tile();
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

		display_big_image();

		display_draw_cursor();
#if 0
		display_small_tiles();
#endif
		display_palette();
		display_palette_current_color();
		display_palette_hover_color();

		display_hue_bar();
		display_hue_current();

		display_satlum_rect();
		display_satlum_current();

		display_tiles();


		SDL_Flip(sc);
	}

end:	SDL_Quit();
	return 0;
}

