#include <SDL2/SDL.h>
#include <endian.h>
#include <assert.h>

#include <display.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *canvas = NULL;

gbuf_t *fb;

// TODO: Refactor odroid go library to also return int?
static int display_init2(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
		return -1;
	}
	window = SDL_CreateWindow("ogo-shell", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * 4, DISPLAY_HEIGHT * 4,
				  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		fprintf(stderr, "error creating SDL window: %s\n", SDL_GetError());
		return -1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr, "error creating SDL renderer: %s\n", SDL_GetError());
		return -1;
	}
	// TODO: Not sure if this works with RGB565
	canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	SDL_RenderSetLogicalSize(renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	fb = gbuf_new(DISPLAY_WIDTH, DISPLAY_HEIGHT, 2, false);
	return 0;
}

void display_init(void) { display_init2(); }

void display_poweroff(void)
{
	gbuf_free(fb);
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void display_clear(uint16_t color)
{
	// TODO: Clear canvas
	display_update();
}

// Copy gbuf to SDL canvas and update screen
void display_update(void)
{
	int pitch;
	void *pixels;
	SDL_LockTexture(canvas, NULL, &pixels, &pitch);
	memcpy(pixels, fb->data, fb->width * fb->height * fb->bytes_per_pixel);
	SDL_UnlockTexture(canvas);
	SDL_RenderCopy(renderer, canvas, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void display_update_rect(rect_t r)
{
	// TODO: Simulate or visualize Partial update?
	display_update();
}

void display_drain() {}

void display_screenshot(const char *path)
{
	const size_t fb_size = fb->width * fb->height * fb->bytes_per_pixel;
	void *pixels = malloc(fb_size);
	memcpy(pixels, fb->data, fb_size);
	SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, fb->width, fb->height, 16, fb->width * 2, SDL_PIXELFORMAT_RGB565);
	assert(surf != NULL);
	SDL_SaveBMP(surf, path);
	SDL_FreeSurface(surf);
}
