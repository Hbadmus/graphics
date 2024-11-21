# Optional - Screenshot (You are not required to do this)

# Resources to Help

- https://learnopengl.com/Advanced-OpenGL/Framebuffers

# Description

Recall that we have now learned about framebuffer Objects (FBO) and the ability to create them. By default, a framebuffer object is created when we first initialize the OpenGL context. A framebuffer consists of multiple 'attachements' -- and we most often have a 'color' attachement which can store the pixels of our framebuffer. In this portion of the assignment, **as a completely optional task**, you will write a small function to 'dump' out the contents of the framebuffer and capture a screenshot.

## glReadPixels

Reading of pixels is relatively easy with the OpenGL function [glReadPixels](https://docs.gl/gl4/glReadPixels). The methodology is to first bind to whichever buffer you want to capture from, and then you will call glReadPixels, which will store pixels in a pre-allocated buffer.

```cpp
unsigned char* buffer = new unsigned char[framebuffer_width*framebuffer_height * 3];
glReadPixels(0,0,framebuffer_width,framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
```

## Challenges

- Keep in mind when you write out the pixels, they will again appear 'upside down' (Remembeer in our graphics API, y points up, whereas in our ppm images, y points down.).
  - Write a routine to 'flip the pixels. Ideally you can do this one row at a time.
- Capturing a movie
  - Capturing a movie requires taking lots of screenshots (e.g. 30 screenshots per second). This poses an interesting performance challenge.
  - One way you can improve this, is by pre-allocating a very large buffer (e.g. [framebuffer_width*framebuffer_height * 3 * 30) // where 30 represents 30 frames being captured).
  - You'll then need some other program or encoder to take the 30 images and write them to a video format or .gif. You can use terminal tools for this (e.g. ffmpeg)
  - Another performance consideration however -- is that you'll usually only want to write the data after the program runs -- as writing the data while the program is running may slow things down even further.
      - For those who want an additional challenge, you can have a separate thread asynchronously writing out the data.

# Deliverable

(Optionally) Enhance your programs by being able to press the <kbd>p</kbd> key to capture a screenshot and write out a 'screenshot.ppm' in the current directory.

