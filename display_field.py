#!/usr/bin/env python
#------------------------------Magnetic Field Visualizer------------------------------

import sys, random, math, pygame, time
from random import randint
from pygame.locals import *
from math import sin, cos

#Constants: ----------------------------------------------------------------
window = [600, 600]#sets the size of the window
tau = 6.28318530717958#Equal to 2pi. One full turn in radians.

# these vectors determine the viewing plane:
offset_vec = (0., 0., 5.)
plane_u = (1., 0., 0.0)
plane_v = (0., 1., 0.0)

# integration step:
ISTEP = 0.03
# track constants:
TRACKLEN = 50

#---------------------------------------------------------------------------

def add(v1, v2):
    return (v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2])

def sub(v1, v2):
    return (v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2])

def constmul(a, v):
    return (a*v[0], a*v[1], a*v[2])

def dot(v1, v2):
    return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2])


def proj(vec):
    v = sub(vec, offset_vec)
    return (dot(v, plane_u), dot(v, plane_v))


# convert a 2d vector to pixel locations
def convert_to_pix(v):
    return (int(window[0] * (v[0] + 0.5)), int(window[1] * (v[1] + 0.5)))


def render_tracks(screen, tracks):
    screen.fill((0, 0, 0))
    for track in tracks:
        for i in range(1, len(track)):
            alpha = (255 * i) / len(track)
            x_s, y_s = proj(track[i - 1])
            x_f, y_f = proj(track[i])
            pygame.draw.line(screen, (alpha, 255 - alpha, 250),
                                convert_to_pix(proj(track[i - 1])),
                                convert_to_pix(proj(track[i])))


# magnetic field as a function of radius
def B(r):
    return (r[1], -r[0], 0.)


# grow a track by one
def grow_track(track):
    predict = add(track[-1], constmul(ISTEP, B(track[-1])))
    correct = add(track[-1], constmul(ISTEP, B(predict)))
    track.append(constmul(0.5, add(predict, correct)))

# shrink a track by amount
def shrink_track(track):
    track.pop(0)

# grow all tracks in a list of tracks
def evolve_all_tracks(tracks):
    for track in tracks:
        grow_track(track)
        shrink_track(track)


# makes a single track, starting from a particular point
def make_track(r):
    ans = [r]
    for i in range(0, TRACKLEN):
        grow_track(ans)
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
        evolve_all_tracks(tracks)
        if not random.randint(0, 100):
            tracks.append(make_track(randvec()))
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
        time.sleep(0.03)


main()
