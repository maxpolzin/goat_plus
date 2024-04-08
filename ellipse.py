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

def compute_ellipse_properties(a1, a2, circumference):
    
    # Compute ellipse parameters
    def equation(a, b):
        return np.pi * (3 * (a + b) - np.sqrt((3 * a + b) * (a + 3 * b))) - circumference

    initial_guess = a1
    b1, = fsolve(equation, initial_guess, args=(a1,))

    initial_guess = a2
    b2, = fsolve(equation, initial_guess, args=(a2,))

    e1 = Ellipse(Point(0, 0), a1, b1)
    e2 = Ellipse(Point(0, 0), b2, a2)

    # Compute intersection points
    pts = e1.intersection(e2)

    pt_top_right = pts[3]
    pt_top_left = pts[1]
    pt_bottom_right = pts[2]
    pt_bottom_left = pts[0]

    # Compute arc lengths
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

    # Compute angle between tangents
    slope1 = -b1 * pt_top_right.x/(a1*sqrt(a1**2 - pt_top_right.x**2))
    line1 = Line2D(pt_top_right,slope=slope1)

    slope2 = -a2 * pt_top_right.x/(b2*sqrt(b2**2 - pt_top_right.x**2))
    line2 = Line2D(pt_top_right,slope=slope2)

    theta_radians = line1.angle_between(line2)
    theta_degrees = deg(theta_radians)

    return {
        "a1": a1,
        "b1": b1,
        "a2": a2,
        "b2": b2,
        "e1_circumference": e1_circumference,
        "e2_circumference": e2_circumference,
        "pt_top_right": pt_top_right,
        "pt_top_left": pt_top_left,
        "pt_bottom_left": pt_bottom_left,
        "pt_bottom_right": pt_bottom_right,
        "arc_length_top_e1": arc_length_top_e1,
        "arc_length_side_e1": arc_length_side_e1,
        "arc_length_top_e2": arc_length_top_e2,
        "arc_length_side_e2": arc_length_side_e2,
        "delta1": delta1,
        "delta2": delta2,
        "distance1": distance1,
        "distance2": distance2,
        "line1": line1,
        "line2": line2,
        "theta_degrees": theta_degrees,
    }


def visusalize_ellipse_properties(props):

    a1, a2, b1, b2 = props["a1"], props["a2"], props["b1"], props["b2"]
    e1_circumference, e2_circumference = props["e1_circumference"], props["e2_circumference"]
    pt_top_right, pt_top_left, pt_bottom_left, pt_bottom_right = props["pt_top_right"], props["pt_top_left"], props["pt_bottom_left"], props["pt_bottom_right"]
    arc_length_top_e1, arc_length_side_e1 = props["arc_length_top_e1"], props["arc_length_side_e1"]
    arc_length_top_e2, arc_length_side_e2 = props["arc_length_top_e2"], props["arc_length_side_e2"]
    delta1, delta2 = props["delta1"], props["delta2"]
    distance1, distance2 = props["distance1"], props["distance2"]
    line1, line2 = props["line1"], props["line2"]
    theta_degrees = props["theta_degrees"]

    e1_eq = Eq(x**2 / a1**2 + y**2 / b1**2, 1)
    e2_eq = Eq(x**2 / b2**2 + y**2 / a2**2, 1)
    line1_eq = line1.equation(x=x, y=y)
    line2_eq = line2.equation(x=x, y=y)

    limit = max(a1, b1, a2, b2) * 1.1

    p1 = plot_implicit(e1_eq, (x, -limit, limit), (y, -limit, limit), show=False, line_color='blue')
    p2 = plot_implicit(e2_eq, (x, -limit, limit), (y, -limit, limit), show=False, line_color='red')

    p1.extend(p2)

    p3 = plot_implicit(line1_eq, (x, -limit, limit), (y, -limit, limit), line_color='black', show=False)
    p4 = plot_implicit(line2_eq, (x, -limit, limit), (y, -limit, limit), line_color='black', show=False)

    p1.extend(p3)
    p1.extend(p4)


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

circumference = 2.0
a1=0.39
a2=0.355


#objective = (props.get("distance1") - props.get("distance2"))**2 + (props.get("arc_length_top_e1") - circumference/4.0)**2+ (props.get("arc_length_side_e2") - circumference/4.0)**2

props = compute_ellipse_properties(a1, a2, circumference)
visusalize_ellipse_properties(props)

