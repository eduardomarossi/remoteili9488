#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define xstr(a) str(a)
#define str(a) #a

#define ili9488_draw_pixmap(x, y, w, h, C) ili9488_draw_pixmap1(x, y, w, h, xstr(C))
#define font_draw_text(f, t, x, y, s) font_draw_text1(xstr(f), t, x, y, s)
#define COLOR_CONVERT(c) c

#define ILI9488_LCD_WIDTH 320
#define ILI9488_LCD_HEIGHT 480

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

typedef struct {
   int x;
   int y;
} touchData;
typedef char* tFont;

int g_sockfd;

void error(const char *msg) {
    perror(msg);
    exit(0);
}
void tcpwritestr(int sockfd, char *msg) {
    int n = write(sockfd, msg, strlen(msg));
    if (n < 0)
         error("ERROR writing to socket");
}
int tcpinit() {
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;


    portno = 10000;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname("localhost");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    g_sockfd = sockfd;
    return sockfd;
}
int tcpreadstr(int sockfd, char* buffer, int buffer_size) {
    int count;
    ioctl(sockfd, FIONREAD, &count);
    if(count == 0) {
       return count;
    }

    if(count + 1 > buffer_size) {
       count = buffer_size-1;
    }

    count = read(sockfd, buffer, count);
    buffer[count] = '\0';
    return count;
}
void tcp_ensure_one(char *buffer, int len) {
    char *p = buffer;
    while(len--) {
       if(*p == '\n') {
           *p = '\0';
           return;
       }
       p++;
    }
}
int tcp_coords(char *rxMSG, touchData *t) {
    int x, y;
    char *token = strtok(rxMSG, ",");
	if(token != NULL) {
	    token++;
	    x = strtol(token, 0, 10);
		token = strtok(NULL, ",");
	    if(token != NULL){
		    y = strtol(token, 0, 10);
			token = strtok(NULL, ",");
			if(token != NULL) {
				t->x = x;
				t->y = y;
				return 1;
			}
		}
	}
    return 0;
}
int getcoords(touchData *t) {
   char buffer[256];
   int read;
   read = tcpreadstr(g_sockfd, buffer, 256);
   if(read) {
      tcp_ensure_one(buffer, 256);
      return tcp_coords(buffer, t);
    }
   return 0;
}
void ili9488_set_foreground_color(uint32_t color) {
    char buffer[128];
	sprintf(buffer, "#ili9488_set_foreground_color:%u#\r\n", color);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_filled_circle(uint32_t ul_x, uint32_t ul_y, uint32_t ul_r) {
    char buffer[128];
	sprintf(buffer, "#ili9488_draw_filled_circle:%d,%d,%d#\r\n", ul_x, ul_y, ul_r);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_circle(uint32_t ul_x, uint32_t ul_y, uint32_t ul_r) {
    char buffer[128];
	sprintf(buffer, "#ili9488_draw_circle:%d,%d,%d#\r\n", ul_x, ul_y, ul_r);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_pixel(uint32_t ul_x, uint32_t ul_y) {
    char buffer[128];
	printf("#ili9488_draw_pixel:%d,%d#\r\n", ul_x, ul_y);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_line(uint32_t ul_x, uint32_t ul_y) {
    char buffer[128];
	sprintf(buffer, "#ili9488_draw_line:%d,%d#\r\n", ul_x, ul_y);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_rectangle(uint32_t ul_x1, uint32_t ul_y1, uint32_t ul_x2, uint32_t ul_y2) {
    char buffer[128];
	sprintf(buffer, "#ili9488_draw_rectangle:%d,%d,%d,%d#\r\n", ul_x1, ul_y1, ul_x2, ul_y2);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_filled_rectangle(uint32_t ul_x1, uint32_t ul_y1, uint32_t ul_x2, uint32_t ul_y2) {
    char buffer[128];
	sprintf(buffer, "#ili9488_draw_filled_rectangle:%d,%d,%d,%d#\r\n", ul_x1, ul_y1, ul_x2, ul_y2);
	tcpwritestr(g_sockfd, buffer);
}
void font_draw_text1(char *font, const char *text, int x, int y, int spacing) {
    char buffer[256];
	sprintf(buffer, "#font_draw_text:%s,%s,%d,%d,%d#\r\n", font, text, x, y, spacing);
	printf("%s", buffer);
	tcpwritestr(g_sockfd, buffer);
}
void ili9488_draw_pixmap1(uint32_t ul_x, uint32_t ul_y, uint32_t ul_width, uint32_t ul_height, const char *p_ul_pixmap) {
    char buffer[256];
	sprintf(buffer, "#ili9488_draw_pixmap:%d,%d,%d,%d,%s#\r\n", ul_x, ul_y, ul_width, ul_height, p_ul_pixmap);
	tcpwritestr(g_sockfd, buffer);
}

////////////
// START HERE
/////////////

#define SCREEN_SETUP  0
#define SCREEN_ADD_PASSWORD 1
#define SCREEN_LOCKED  2
#define SCREEN_UNLOCKED  3
#define SCREEN_LIST_PASSWORDS  4

uint32_t g_current_screen;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    int cur_pressed;
    char* text;
    void (*pressed)();
    void (*released)();
} m_button;

void m_button_init(m_button *b) {
    memset(b, 0, sizeof(m_button));
}

void m_buttons_check(m_button buttons[], int button_count, int x, int y) {
    for(int i = 0; i < button_count; i++) {
       m_button *b = &buttons[i];
       if(x >= b->x && x <= b->x + b->w && y >= b->y && y <= b->y + b->h) {
           if(b->cur_pressed) {
              if(b->released != NULL) {
              (*b->released)();
             }
             b->cur_pressed = 0;
           } else {
             if(b->pressed != NULL) {
              (*b->pressed)();
             }
             b->cur_pressed = 1;
           }
       }
    }
}

void clear_screen(void) {
    ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
    ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH, ILI9488_LCD_HEIGHT);
}

void m_button_draw_numpad(m_button buttons[], int button_count) {
    for(int i = 0; i < button_count; i++) {
        m_button *b = &buttons[i];
        font_draw_text(&arial_52, b->text, b->x, b->y, 1);
    }
}

void m_screen_setup() {
    ili9488_set_foreground_color(COLOR_CONVERT(COLOR_DARKRED));
    ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, 60);
    ili9488_draw_filled_rectangle(0, 420, ILI9488_LCD_WIDTH-1, 480);
    font_draw_text(&arial_24, "Seja bem vindo", 30, 50,1 );
    ili9488_draw_pixmap(70, 125, 180, 230, &lock);
}

void m_start_screen(uint32_t screen) {
    clear_screen();

    switch(screen) {
        case SCREEN_SETUP:
          m_screen_setup();
          break;
    }

    g_current_screen = screen;
}

int main()
{
    touchData touch;
    tcpinit();

    m_button buttons[1];
    buttons[0].x = 30;
    buttons[0].y = 30;
    buttons[0].w = 50;
    buttons[0].h = 50;
    buttons[0].text = "1";


    m_start_screen(SCREEN_SETUP);

    while(1) {
        if(getcoords(&touch)) {
           printf("touch x:%d y:%d\n", touch.x, touch.y);
        }

        usleep(100 * 1000);
    }

    close(g_sockfd);
    return 0;
}

