#!/usr/bin/env python
#------------------------------Magnetic Field Visualizer------------------------------

import sys, random, math, pygame
from random import randint
from pygame.locals import *
from math import sin, cos

#Constants: ----------------------------------------------------------------
window = [600, 600]#sets the size of the window
zoom = 1
visibility_distance = 100000.0
black = 0,0,0#Colour
boxsize = 10.0#Side Length of Voxles
bh = boxsize/2#Half ofSide Lenght of Voxles
tau = 6.28318530717958#Equal to 2pi. One full turn in radians.
#---------------------------------------------------------------------------

def add(v1, v2):
    return (v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2])

def sub(v1, v2):
    return (v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2])

def constmul(a, v):
    return (a*v[0], a*v[1], a*v[2])

def dot(v1, v2):
    return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2])

# these vectors determine the viewing plane:
offset_vec = (0., 0., 5.)
plane_u = (1., 0., 0.)
plane_v = (0., 1., 0.)

def proj(vec):
    v = sub(vec, offset_vec)
    return (dot(v, plane_u), dot(v, plane_v))


# convert a 2d vector to pixel locations
def convert_to_pix(v):
    return (int(window[0] * (v[0] + 0.5)), int(window[1] * (v[1] + 0.5)))


def render_tracks(screen, tracks):
    for track in tracks:
        for i in range(1, len(track)):
            x_s, y_s = proj(track[i - 1])
            x_f, y_f = proj(track[i])
            pygame.draw.line(screen, (250, 250, 250),
                                convert_to_pix(proj(track[i - 1])),
                                convert_to_pix(proj(track[i])))


def main():
    pygame.init()
    pygame.display.set_caption("Magnetic Field Visualizer")
    screen = pygame.display.set_mode(window)
    
    done = False
    while not done:
        render_tracks(screen, [[(0.1, 0.1, 0.1), (0.3, 0.2, -0.2)]])
        #General Pygame Management.------------------------
        #(And Key Commands)
        for event in pygame.event.get():
            if event.type == QUIT or (event.type == KEYUP and event.key == K_ESCAPE):#End Program?
                done = True
                break

        pygame.display.update()#Update Screen
        pygame.display.flip()
        #--------------------------------------------------


main()
