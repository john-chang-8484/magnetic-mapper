#!/usr/bin/env python
#------------------------------Magnetic Field Visualizer------------------------------

import sys, random, math, pygame
from random import randint
from pygame.locals import *
from math import sin, cos

#Constants: ----------------------------------------------------------------
window = [600, 600]#sets the size of the window
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


# magnetic field as a function of radius
def B(r):
    return (r[1], -r[0], 0.)


# makes a single track, starting from a particular point
def make_track(r):
    print r
    ans = [r]
    for i in range(0, 20):
        ans.append(add(ans[-1], constmul(0.05, B(ans[-1]))))
    return ans

# random vector
def randvec():
    return ( random.randint(-100, 100) / 200.,
             random.randint(-100, 100) / 200.,
             random.randint(-100, 100) / 200. )


# makes a list of tracks:
def make_tracks():
    ans = []
    for i in range(0, 20):
        ans.append(make_track(randvec()))
    return ans


def main():
    pygame.init()
    pygame.display.set_caption("Magnetic Field Visualizer")
    screen = pygame.display.set_mode(window)
    
    tracks = make_tracks()
    
    done = False
    while not done:
        render_tracks(screen, tracks)
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
