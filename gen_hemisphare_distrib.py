from numpy import pi, cos, sin, arccos, arange, column_stack
import matplotlib.pyplot as pp

num_pts = 20
indices = arange(0, num_pts, dtype=float) + 0.5

phi = arccos(1 - indices*0.85 / num_pts)
theta = pi * (1 + 5**0.5) * indices

x, y, z = cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi)

for dir in column_stack((x,y,z)):
    print(f"vec3({dir[0]},{dir[2]},{dir[1]}),")


pp.figure().add_subplot(111, projection='3d').scatter(x, y, z)
pp.show()
