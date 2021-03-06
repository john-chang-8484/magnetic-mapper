#!/usr/bin/env python
# ------------------------------ << Magnetic Field Visualizer >> ------------------------------

import sys, random, math, pygame, time
from random import randint
from pygame.locals import *
from math import sin, cos
import numpy as np
import csv

#Constants: ----------------------------------------------------------------
window = [1000, 1000]#sets the size of the window
tau = 6.28318530717958#Equal to 2pi. One full turn in radians.

# integration step:
ISTEP = 0.003
# track constants:
TRACKLEN = 50

# microsecond of duty in pwm per degree
PWM_PER_DEG = (2330. - 550.) / 180.
# conversion factor from degrees to radians
RAD_PER_DEG = tau / 360.
# thus, 
RAD_PER_PWM = RAD_PER_DEG / PWM_PER_DEG


# physical parameters:

# zero point for altitude
ALT_0 = 950. # [pwm us]
# zero point for azimuth (not quite so important)
AZM_0 = 890. # [pwm us]

# the space sweepable by the arm looks like a torus
# a torus has two parameters, the major and minor radius
R_MAJ = 0.028 # [m]
R_MIN = 0.17  # [m]


# rendering parameters:

# maximum size a field vector can be drawn as
MAX_VEC = 0.01 # [m]

# radius of the balls
BALL_RAD = 0.0005 # [m]

#---------------------------------------------------------------------------

# a class that describes all the view parameters:
class View:
    # these vectors determine the viewing plane:
    def __init__(self):
        self.offset_vec = (0., 0., 3.)
        self.plane_u = (1., 0., 0.) # plane x direction
        self.plane_v = (0., 1., 0.) # plane y direction
        self.plane_w = (0., 0., -1.) # plane z direction (into screen)
        self.view_back = 0.5        # offset of viewpoint in the negative z direction (out of screen)
        self.zoom = 10.  # zoom factor


# some vector functions:

def add(v1, v2):
    return (v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2])

def sub(v1, v2):
    return (v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2])

def constmul(a, v):
    return (a*v[0], a*v[1], a*v[2])

def dot(v1, v2):
    return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2])

def length(v):
    return math.sqrt(dot(v, v))

# project a vector onto plane_u and plane_v (orthographic)
def proj(vec, view):
    v = sub(vec, view.offset_vec)
    return (dot(v, view.plane_u), dot(v, view.plane_v))


# project a vector onto plane_u and plane_v, with the viewpoint located back from
# the origin of the plane by the constant view_back
def point_proj(vec, view):
    r = sub(vec, view.offset_vec)
    u = dot(r, view.plane_u)
    v = dot(r, view.plane_v)
    w = dot(r, view.plane_w)
    if (w < 0.1): # crop things behind the viewing screen
        return None
    s = view.zoom * view.view_back / (w + view.view_back) # scale factor that accounts for forshortening
    return (u * s, v * s)


# convert a 2d vector to pixel locations
def convert_to_pix(v):
    return (int(window[0] * (v[0] + 0.5)), int(window[1] * (v[1] + 0.5)))


# set the projection function we use:
proj_fn = point_proj
def render(screen, view, field_points):
    screen.fill((0, 0, 0))
    
    max_field = max(map(lambda fp: length(fp.field), field_points)) # the maximum B field strength
    
    # draw vectors:
    for fp in field_points:
        tail = fp.point
        tip = add(tail, constmul(MAX_VEC / max_field, fp.field))
        p_s = proj_fn(tip, view)
        p_f = proj_fn(tail,     view)
        if p_s != None and p_f != None:
            pygame.draw.line(screen, (40, 160, 200),
                             convert_to_pix(p_s),
                             convert_to_pix(p_f))
     
    # draw bases of vectors:
    for fp in field_points:
        tail = fp.point
        p = proj_fn(tail, view)
        if p != None:
            screen.set_at(convert_to_pix(p), (255, 255, 255))


# make a view corresponding to these angles:
def get_view(phi, theta, offset=0.5):
    ans = View()
    w = ( math.cos(phi) * math.sin(theta), math.sin(phi) * math.sin(theta), math.cos(theta) )
    ans.offset_vec = constmul(-offset, w)
    ans.plane_w = w
    u = ( -math.sin(phi), math.cos(phi), 0. )
    ans.plane_u = u
    v = ( math.cos(phi) * math.cos(theta), math.sin(phi) * math.cos(theta), -math.sin(theta) )
    ans.plane_v = v
    return ans



# stuff to convert to cartesian:

# a field at a point in cartesian space:
class FieldPoint:
    def __init__(self, point, field):
        self.point = point
        self.field = field
    def __str__(self):
        fld = self.field; pnt = self.point
        return "<%f, %f, %f> @ (%f, %f, %f)" % (fld[0], fld[1], fld[2], pnt[0], pnt[1], pnt[2])

# produce a FieldPoint from a row of data
def field_point_from_data(data):
    azm = (data[0] - AZM_0) * RAD_PER_PWM
    alt = (data[1] - ALT_0) * RAD_PER_PWM
    B_r     = data[2]
    B_phi   = data[3]
    B_theta = data[4]
    
    pos = add(add(constmul(R_MAJ, (cos(azm), sin(azm), 0.)),
                  constmul(R_MIN, (cos(alt)*cos(azm), cos(alt)*sin(azm), sin(alt)))),
                  (-(R_MAJ + R_MIN), 0., 0.)) # offset to put readings more or less at origin
    
    B = (0., 0., 0.)
    B = add(B, constmul(B_r, (cos(alt)*cos(azm), cos(alt)*sin(azm), sin(alt))))
    B = add(B, constmul(B_phi, (sin(azm), -cos(azm), 0.)))
    B = add(B, constmul(B_theta, (-sin(alt)*cos(azm), -sin(alt)*sin(azm), cos(alt))))
    
    return FieldPoint(pos, B)



def main():
    print "reading file..."
    with open("out.csv", "rb") as f:
        reader = csv.reader(f)
        rows = list(reader)
    rows = rows[1:] # remove column labels
    
    rows = map(lambda l: map(lambda x: float(x), l), rows) # convert to floating point numbers
    
    field_points = map(field_point_from_data, rows)

    pygame.init()
    pygame.display.set_caption("Magnetic Field Visualizer")
    screen = pygame.display.set_mode(window)
    
    
    view = View()
    phi = 0.
    theta = tau/3
    
    done = False
    while not done:
        phi += 0.005
        #theta += 0.005
        view = get_view(phi, theta)
        render(screen, view, field_points)
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
