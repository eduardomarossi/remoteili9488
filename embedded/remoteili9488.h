/*
 * remoteili9488.h
 *
 * Created: 22/05/2020 17:36:14
 *  Author: eduardomarossi
 */ 


#ifndef REMOTEILI9488_H_
#define REMOTEILI9488_H_
#include <asf.h>

const extern int ILI9488_LCD_WIDTH;
const extern int ILI9488_LCD_HEIGHT;

#define xstr(a) str(a)
#define str(a) #a

#define ili9488_draw_pixmap(x, y, w, h, C) ili9488_draw_pixmap1(x, y, w, h, xstr(C))
#define font_draw_text(f, t, x, y, s) font_draw_text1(xstr(f), t, x, y, s)
#define COLOR_CONVERT(c) c

#define ILI9488_LCD_WIDTH 320
#define ILI9488_LCD_HEIGHT 480

struct mxt_device {
	
} ;

/**
 * Input parameters when initializing ili9488 driver.
 */
struct ili9488_opt_t{
	uint32_t ul_width;          //!< lcd width in pixel
	uint32_t ul_height;         //!< lcd height in pixel
	uint32_t foreground_color;  //!< lcd foreground color
	uint32_t background_color;  //!< lcd background color
};

uint32_t ili9488_init(struct ili9488_opt_t *p_opt);

void mxt_init(struct mxt_device *device);

int mxt_is_message_pending(struct mxt_device *device);

void mxt_handler(struct mxt_device *device, uint *x, uint *y);

void ili9488_set_foreground_color(uint32_t color);

void ili9488_draw_filled_circle(uint32_t ul_x, uint32_t ul_y, uint32_t ul_r);

void ili9488_draw_circle(uint32_t ul_x, uint32_t ul_y, uint32_t ul_r);

void ili9488_draw_pixel(uint32_t ul_x, uint32_t ul_y);

void ili9488_draw_line(uint32_t ul_x, uint32_t ul_y);

void ili9488_draw_rectangle(uint32_t ul_x1, uint32_t ul_y1, uint32_t ul_x2, uint32_t ul_y2);

void ili9488_draw_filled_rectangle(uint32_t ul_x1, uint32_t ul_y1, uint32_t ul_x2, uint32_t ul_y2);

void font_draw_text1(const char *font, const char *text, int x, int y, int spacing);

void ili9488_draw_pixmap1(uint32_t ul_x, uint32_t ul_y, uint32_t ul_width, uint32_t ul_height, const char *p_ul_pixmap);

void task_uartRx(void *pvParameters);
void task_Process(void *pvParameters);
void USART1_init(void);


/* RGB 24-bits color table definition (RGB888). */
#define COLOR_BLACK          (0x000000u)
#define COLOR_WHITE          (0xFFFFFFu)
#define COLOR_BLUE           (0x0000FFu)
#define COLOR_GREEN          (0x00FF00u)
#define COLOR_RED            (0xFF0000u)
#define COLOR_NAVY           (0x000080u)
#define COLOR_DARKBLUE       (0x00008Bu)
#define COLOR_DARKGREEN      (0x006400u)
#define COLOR_DARKCYAN       (0x008B8Bu)
#define COLOR_CYAN           (0x00FFFFu)
#define COLOR_TURQUOISE      (0x40E0D0u)
#define COLOR_INDIGO         (0x4B0082u)
#define COLOR_DARKRED        (0x800000u)
#define COLOR_OLIVE          (0x808000u)
#define COLOR_GRAY           (0x808080u)
#define COLOR_SKYBLUE        (0x87CEEBu)
#define COLOR_BLUEVIOLET     (0x8A2BE2u)
#define COLOR_LIGHTGREEN     (0x90EE90u)
#define COLOR_DARKVIOLET     (0x9400D3u)
#define COLOR_YELLOWGREEN    (0x9ACD32u)
#define COLOR_BROWN          (0xA52A2Au)
#define COLOR_DARKGRAY       (0xA9A9A9u)
#define COLOR_SIENNA         (0xA0522Du)
#define COLOR_LIGHTBLUE      (0xADD8E6u)
#define COLOR_GREENYELLOW    (0xADFF2Fu)
#define COLOR_SILVER         (0xC0C0C0u)
#define COLOR_LIGHTGREY      (0xD3D3D3u)
#define COLOR_LIGHTCYAN      (0xE0FFFFu)
#define COLOR_VIOLET         (0xEE82EEu)
#define COLOR_AZUR           (0xF0FFFFu)
#define COLOR_BEIGE          (0xF5F5DCu)
#define COLOR_MAGENTA        (0xFF00FFu)
#define COLOR_TOMATO         (0xFF6347u)
#define COLOR_GOLD           (0xFFD700u)
#define COLOR_ORANGE         (0xFFA500u)
#define COLOR_SNOW           (0xFFFAFAu)
#define COLOR_YELLOW         (0xFFFF00u)

#endif /* REMOTEILI9488_H_ */