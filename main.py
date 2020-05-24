#!/usr/bin/python3
# Author: Eduardo Marossi
import os
import socket
import sys
from datetime import datetime
import serial
import argparse
import pygame
import yaml
import logging

VERSION = '1.1.0'


def get_rgb_from_int(RGBint):
    blue =  RGBint & 255
    green = (RGBint >> 8) & 255
    red =   (RGBint >> 16) & 255
    return red, green, blue


class RemoteScreen:
    def __init__(self):
        self.screen = None
        self.cur_color = (255, 255, 255)
        self.bitmaps = {}
        self.fonts = {}
        self.fonts_color = {}
        self.touch_function = None

    def load_fonts(self):
        with open('fonts.yaml', 'r') as f:
            data = yaml.safe_load_all(f.read())

        fonts_config = next(data)
        for k, v in fonts_config.items():
            if v['name'].lower().endswith('.ttf'):
                self.fonts[k] = pygame.font.Font(os.path.join('fonts', v['name']), v['size'])
            else:
                self.fonts[k] = pygame.font.SysFont(v['name'], v['size'])
            colors = []
            for c in v['color'].split(','):
                colors.append(int(c))
            self.fonts_color[k] = colors
        logging.info('Fonts loaded: {}'.format(self.fonts))

    def load_bitmaps(self):
        (_, _, filenames) = next(os.walk('bitmaps'))
        for f in filenames:
            path = os.path.join('bitmaps', f)
            basename = os.path.basename(path)
            name = os.path.splitext(basename)[0]

            self.bitmaps[name] = pygame.image.load(path).convert_alpha()
        logging.info('Bitmaps loaded: {}'.format(self.bitmaps))

    def start_screen(self):
        self.screen = pygame.display.set_mode((320, 480))
        self.screen.fill((0, 0, 0))
        self.running = True

    def update_screen(self):
        pygame.display.update()

    def update_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.MOUSEBUTTONUP:
                pos = pygame.mouse.get_pos()
                if self.touch_function is not None:
                    self.touch_function(pos[0], pos[1], 0)
            elif event.type == pygame.MOUSEBUTTONDOWN:
                pos = pygame.mouse.get_pos()
                if self.touch_function is not None:
                    self.touch_function(pos[0], pos[1], 1)

    def draw_rectangle(self, x1, y1, x2, y2):
        logging.debug('ili9488_draw_rectangle:{},{},{},{}'.format(x1,x2,y1,y2))
        rect = pygame.Rect(int(x1), int(y1), int(x2)-int(x1), int(y2)-int(y1))
        pygame.draw.rect(self.screen, self.cur_color, rect, 1)

    def draw_filled_rectangle(self, x1, y1, x2, y2):
        logging.debug('ili9488_draw_filled_rectangle:{},{},{},{}'.format(x1,x2,y1,y2))
        rect = pygame.Rect(int(x1), int(y1), int(x2)-int(x1), int(y2)-int(y1))
        pygame.draw.rect(self.screen, self.cur_color, rect, 0)

    def draw_filled_circle(self, x, y, r):
        logging.debug('ili9488_draw_filled_circle:{},{},{}'.format(x, y, r))
        cords = (int(x), int(y))
        pygame.draw.circle(self.screen, self.cur_color, cords, int(r), 0)

    def draw_circle(self, x, y, r):
        logging.debug('ili9488_draw_circle:{},{},{}'.format(x, y, r))
        cords = (int(x), int(y))
        pygame.draw.circle(self.screen, self.cur_color, cords, int(r), 1)

    def draw_pixmap(self, x, y, w, h, name):
        name = name.replace('&', '')
        logging.debug('ili9488_draw_pixmap:{},{},{},{},{}'.format(x, y, w, h, name))
        if name not in self.bitmaps:
            logging.error('Tried to draw not found bitmap {}'.format(name))
            return

        bitmap = self.bitmaps[name]
        self.screen.blit(bitmap, (int(x), int(y)))

    def set_foreground_color(self, color):
        logging.debug('ili9488_set_foreground_color:{}'.format(color))
        r, g, b = get_rgb_from_int(int(color))
        logging.debug('setting color to r:{} g:{} b:{}'.format(r, g, b))
        self.cur_color = (int(r), int(g), int(b))

    def quit(self):
        self.running = False

    def font_draw_text(self, font, text, x, y, spacing):
        font = font.replace('&', '')
        logging.debug('font_draw_text:{},{},{},{},{}'.format(font, text, x, y, spacing))
        if font not in self.fonts:
            logging.error('Tried to draw not found font {}'.format(font))
            return

        f = self.fonts[font]
        fc = self.fonts_color[font]
        px = int(x)
        for c in text:
            surf = f.render(str(c), False, tuple(fc))
            self.screen.blit(surf, (px, int(y)))
            px += f.size(str(c))[0] + int(spacing)

    def draw_pixel(self, x, y):
        pygame.draw.circle(self.screen, self.cur_color, (int(x), int(y)), 0)

    def process_command(self, line):
        line = line.replace('#', '')
        if ':' in line:
            args = line[line.find(':')+1:].split(',')
            command = line[:line.find(':')]
        else:
            args = []
            command = line

        logging.debug('Command: {} -- with args: {}'.format(command, args))

        fs = {'ili9488_draw_filled_rectangle': {'func': self.draw_filled_rectangle, 'args': 4},
              'ili9488_set_foreground_color':  {'func': self.set_foreground_color, 'args': 1},
              'font_draw_text': {'func': self.font_draw_text, 'args': 5},
              'ili9488_draw_rectangle': {'func': self.draw_rectangle, 'args': 4},
              'ili9488_draw_filled_circle': {'func': self.draw_filled_circle, 'args': 3},
              'ili9488_draw_circle': {'func': self.draw_circle, 'args': 3},
              'ili9488_draw_pixel': {'func': self.draw_pixel, 'args': 2},
              'ili9488_draw_pixmap': {'func': self.draw_pixmap, 'args': 5},
              'quit': {'func': self.quit, 'args': 0}}

        if command not in fs.keys():
            logging.error('Command not found {}'.format(command))
            return

        func = fs[command]['func']
        func_argcount = fs[command]['args']

        if func is not None and func_argcount == len(args):
            if func_argcount == 5:
                func(args[0], args[1], args[2], args[3], args[4])
            elif func_argcount == 4:
                func(args[0], args[1], args[2], args[3])
            elif func_argcount == 3:
                func(args[0], args[1], args[2])
            elif func_argcount == 2:
                func(args[0], args[1])
            elif func_argcount == 1:
                func(args[0])
            elif func_argcount == 0:
                func()


class StdinPort:
    def __init__(self):
        self.buff = ''
        pass

    def readline(self):
        try:
            self.buff += sys.stdin.read(1)
            if self.buff.endswith('\n'):
                aux = self.buff
                self.buff = ''
                return aux
        except KeyboardInterrupt:
            sys.stdout.flush()
            pass
        return ''

    def touch_function(self, x, y, down):
        print('#{},{},{}#'.format(x, y, down))

    def close(self):
        pass


class TcpPort:
    def __init__(self):
        self.buff = ''
        self.lines = []
        # Create a TCP/IP socket
        import socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        # Bind the socket to the port
        server_address = ('localhost', 10000)
        print('starting up on {} port {}'.format(*server_address))
        self.sock.bind(server_address)

        # Listen for incoming connections
        self.sock.listen(1)

        print('waiting for a connection')
        connection, client_address = self.sock.accept()
        print('connection from', client_address)
        self.conn = connection
        self.conn.setblocking(0)

    def readline(self):
        if len(self.lines) > 0:
            line = self.lines[0]
            self.lines.remove(line)
            return line

        try:
            self.buff += self.conn.recv(1024).decode('ascii')
            while '\n' in self.buff:
                pos = self.buff.find('\n')
                line = self.buff[:pos].strip()
                self.buff = self.buff[pos+1:]
                self.lines.append(line)

            if len(self.lines) > 0:
                line = self.lines[0]
                self.lines.remove(line)
                return line

            return ''
        except socket.error:
            return ''

    def touch_function(self, x, y, down):
        self.conn.send('#{},{},{}#\n'.format(x, y, down).encode('ascii'))

    def close(self):
        try:
            self.conn.close()
        except:
            pass
        try:
            self.sock.shutdown()
        except:
            pass


class TestPort:
    def __init__(self):
        self.commands = ['#ili9488_set_foreground_color:16777215#', '#ili9488_draw_filled_rectangle:0,0,320,480#',
                         '#ili9488_set_foreground_color:0#', '#ili9488_draw_filled_rectangle:50,50,150,200#',
                         '#font_draw_text:arial_8,Oi mundo,20,10,1#']
        self.step_in_ms = 100
        self.step = 0
        self.last_time = datetime.today()

    def readline(self):
        cmd = ''
        now = datetime.today()
        delta = now - self.last_time
        if delta.total_seconds() * 1000 < self.step_in_ms:
            return cmd

        self.last_time = now

        if self.step < len(self.commands):
            cmd = self.commands[self.step]
            self.step += 1

        return cmd

    def touch_function(self, x, y, down):
        print('#{},{},{}#'.format(x, y, down))

    def close(self):
        pass


class TouchDevice:
    def __init__(self, serial):
        self.serial = serial

    def touch_function(self, x, y, down):
        self.serial.writelines(['#{},{},{}#\n'.format(x, y, down).encode('ascii')])


if __name__ == '__main__':
    pygame.init()
    parser = argparse.ArgumentParser(description='remoteili9488 v{}'.format(VERSION))
    parser.add_argument('-p', '--port', type=str, default=None)
    parser.add_argument('-s', '--stdin', default=False, action='store_true')
    parser.add_argument('-so', '--socket', default=False, action='store_true')
    parser.add_argument('-l', '--list-ports', default=False, action='store_true')
    parser.add_argument('-lf', '--list-fonts', default=False, action='store_true')
    parser.add_argument('-t', '--test-port', default=False, action='store_true')
    parser.add_argument('-b', '--baudrate', type=int, default=115200)
    parser.add_argument('-st', '--sleep-time', type=int, default=100)
    parser.add_argument('-d', '--debug', default=False, action='store_true')
    args = parser.parse_args()

    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    if args.list_ports:
        os.system('python3 -m serial.tools.list_ports')
        sys.exit(0)

    if args.stdin:
        pass

    if args.list_fonts:
        print(pygame.font.get_fonts())
        sys.exit(0)

    remote = RemoteScreen()

    if args.test_port:
        ser = TestPort()
        remote.touch_function = ser.touch_function
    elif args.socket:
        ser = TcpPort()
        remote.touch_function = ser.touch_function
    elif args.stdin:
        ser = StdinPort()
        remote.touch_function = ser.touch_function
    elif args.port is not None:
        ser = serial.Serial(args.port, args.baudrate, timeout=0)
        td = TouchDevice(ser)
        remote.touch_function = td.touch_function
    else:
        raise Exception('Please provide a port')

    remote.start_screen()
    remote.load_fonts()
    remote.load_bitmaps()

    remote.update_events()
    remote.update_screen()

    while remote.running:
        line = ser.readline()
        if type(line) is bytes:
            line = line.decode('ascii')
        if line != '':
            logging.debug(line)
        find1 = line.find('#')
        find2 = line.rfind('#')
        if find2 > find1:
            remote.process_command(line[find1+1:find2])
        else:
            pygame.time.delay(args.sleep_time)
        remote.update_events()
        remote.update_screen()

    ser.close()
    pygame.quit()

