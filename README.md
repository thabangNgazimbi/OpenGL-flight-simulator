OpenGL-flight-simulator
========================

Simple OpenGL flight simulator 

The goal for this project was to use the openGL skills and techniques to design an interactive 3 dimensional graphical scene. For this task I decided to simulate the flight of an aircraft through a scene while showing different points of view of the scene from the aircraft.
Implementation:
I modeled the scene using wings 3d; I used a flat rectangular surface which we deformed into a scene with mountains canyons and a city. The models for the scene and the plane were loaded as .obj files. To move the plane I used openGL transformations namely glTranslatef (), the use of these transformations allowed movement of the aircraft through the scene with respect to the x, y, and z axis there by simulating flight.
To change the point of view of the camera when looking at the scene I used the gluLookAt () function, this in a sense allowed for the positioning of the scene camera at different points. One camera was positioned directly above the scene and this was the global camera which showed the entire scene and was static. Another point of view was also above the scene; this camera was not static but moved with the plane. I also incorporated a first person point of view that simulates the view from the cockpit of the plane. The last point of view I implemented is of various views of the plane as it flies.
I also implemented some light using a sphere that I colored yellow and attached a light to. I rotated this sphere around the scene to have an effect of the sun rising and setting
Functionality
I also added some functionality that allows users to control the movement of the plane and the various points view. To this users would use the following keys:
W - Changes the planes elevation and allows the plane to flying upwards.
S - Moves the plane down.
A - Turns the plane left.
D - Turns the plane right.
G - Changes the point of view to global.
B - Changes the point of view to above the plane.
V - Is first person view.
N - Shows various views from outside the plane.
Stereo:
We also implement the project in side by side stereo that allow us to view the scene in stereo mode