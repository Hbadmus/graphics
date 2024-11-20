**Note**: I have updated the build.py in this starter to work from this directory. If you move the starter code, you may need to change your build.py

# Starter Project - Brick

Provided in this folder is a separate standalone project for getting normal mapping up and running. If your vertex(shaders/vert.glsl) and fragment shader(shaders/frag.glsl) work in this project, they should not need to be modified to work with your .obj loader.

There should not be any changes needed to the c++ support code, only the [./shaders/vert.glsl](./shaders/vert.glsl) and [./shaders/frag.glsl](./shaders/frag.glsl) (though you may need to refer to the support code to understand how things are working).

Completing this portion is worth 95% of the assignment.

## Other FAQ

- If you tried to load another PPM image (other than brick)
  - Q: My image shows up but is not the appropriate color
  - OpenGL likes square size textures (32x32, 64x64, etc)
    - Why is that? Even dimensions are easier to handle, and useful to create things like mipmaps
    - Also, my PPM parser is very simple! (I used GIMP to export an ASCII PPM and removed any comments in the file)

- Q: Why is the brick half blue and half brick
  - I am demonstrating how to load and display part of a texture. I had to start you with something :) Change this as you need! Observe in the fragment shader why this is.
