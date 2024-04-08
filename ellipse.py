#!/usr/bin/env python3

# %%

from sympy import Eq, Ellipse, Point, sqrt, pi, atan, deg, diff, atan2,symbols, solve
from sympy.plotting import plot_implicit, plot
from sympy.plotting.plot import MatplotlibBackend
from sympy.abc import x, y
from sympy.geometry.line import Line2D

import math
import numpy as np
from scipy.integrate import quad
from scipy.optimize import fsolve

# %%

circumference = 2.0
a1=0.39
a2=0.355



# %%
def equation(a, b):
    return np.pi * (3 * (a + b) - np.sqrt((3 * a + b) * (a + 3 * b))) - circumference

initial_guess = a1
b1, = fsolve(equation, initial_guess, args=(a1,))

initial_guess = a2
b2, = fsolve(equation, initial_guess, args=(a2,))


# %%

e1 = Ellipse(Point(0, 0), a1, b1)
e1_eq = Eq(x**2 / a1**2 + y**2 / b1**2, 1)

e2 = Ellipse(Point(0, 0), b2, a2)
e2_eq = Eq(x**2 / b2**2 + y**2 / a2**2, 1)

limit = max(a1, b1, a2, b2) * 1.1

p1 = plot_implicit(e1_eq, (x, -limit, limit), (y, -limit, limit), show=False, line_color='blue')
p2 = plot_implicit(e2_eq, (x, -limit, limit), (y, -limit, limit), show=False, line_color='red')

p1.extend(p2)


# %%
pts = e1.intersection(e2)

pt = pts[3]

slope1 = -b1 * pt.x/(a1*sqrt(a1**2 - pt.x**2))
line1 = Line2D(pt,slope=slope1)
line1_eq = line1.equation(x=x, y=y)

slope2 = -a2 * pt.x/(b2*sqrt(b2**2 - pt.x**2))
line2 = Line2D(pt,slope=slope2)
line2_eq = line2.equation(x=x, y=y)

p3 = plot_implicit(line1_eq, (x, -limit, limit), (y, -limit, limit), line_color='black', show=False)
p4 = plot_implicit(line2_eq, (x, -limit, limit), (y, -limit, limit), line_color='black', show=False)

p1.extend(p3)
p1.extend(p4)

theta_radians = line1.angle_between(line2)
theta_degrees = deg(theta_radians)


# %%

pt_bottom_left = pts[0]
pt_top_left = pts[1]
pt_bottom_right = pts[2]
pt_top_right = pts[3]


theta1 = atan2(pt_top_left.y, pt_top_left.x)
theta2 = atan2(pt_top_right.y, pt_top_right.x)

def ds(theta, a, b):
    dx = -a * np.sin(theta)
    dy = b * np.cos(theta)
    return np.sqrt(dx**2 + dy**2)

arc_length_top_e1, _ = quad(ds, theta2, theta1, args=(b1, a1))
arc_length_top_e2, _ = quad(ds, theta2, theta1, args=(a2, b2))


theta3 = atan2(pt_bottom_right.y, pt_bottom_right.x)

arc_length_side_e1, _ = quad(ds, theta3, theta2, args=(b1, a1))
arc_length_side_e2, _ = quad(ds, theta3, theta2, args=(a2, b2))

e1_circumference = arc_length_top_e1 * 2 + arc_length_side_e1 * 2
e2_circumference = arc_length_top_e2 * 2 + arc_length_side_e2 * 2

distance1 = b2-a1
distance2 = b1-a2

delta1 = arc_length_top_e1 - arc_length_side_e1
delta2 = arc_length_top_e2 - arc_length_side_e2


# %%

backend = MatplotlibBackend(p1)
backend.process_series()

backend.fig.axes[0].plot(a1, 0, 'bx')
backend.fig.axes[0].plot(0, b1, 'bx')
backend.fig.axes[0].plot(b2, 0, 'rx')
backend.fig.axes[0].plot(0, a2, 'rx')



backend.fig.axes[0].set_aspect('equal', adjustable='box')

backend.fig.axes[0].text(-limit, limit, f'Input:\nCircumference: {circumference:.2f}\na1: {a1:.3f}, b1: {b1:.3f}\na2: {a2:.3f}, b2: {b2:.3f}')

backend.fig.axes[0].text(-limit, -limit, f'Circumference1 ≈ {e1_circumference:.2f}\nArc lengths:\nTop:{arc_length_top_e1:.2f}, Side:{arc_length_side_e1:.2f}\nDelta:{delta1:.2f}', va='top')
backend.fig.axes[0].text(limit, -limit, f'Circumference2 ≈ {e2_circumference:.2f}\nArc lengths:\nTop:{arc_length_top_e2:.2f}, Side:{arc_length_side_e2:.2f}\nDelta:{delta2:.2f}', va='top', ha='right')


backend.fig.axes[0].text((b2+a1)/2, limit/50, f'Distance: {abs(distance1):.3f}', ha='left')
backend.fig.axes[0].text(limit/50, (b1+a2)/2, f'Distance: {abs(distance2):.3f}', ha='left')


backend.fig.axes[0].text(pt.x+limit/25, pt.y+limit/25, f'Angle: {theta_degrees:.2f} degrees', ha='left')

backend.fig.axes[0].plot(pt_top_right.x.evalf(), pt_top_right.y.evalf(), 'ko')
backend.fig.axes[0].plot(pt_top_left.x.evalf(), pt_top_left.y.evalf(), 'ko')
backend.fig.axes[0].plot(pt_bottom_left.x.evalf(), pt_bottom_left.y.evalf(), 'ko')
backend.fig.axes[0].plot(pt_bottom_right.x.evalf(), pt_bottom_right.y.evalf(), 'ko')
backend.show()

# %%

