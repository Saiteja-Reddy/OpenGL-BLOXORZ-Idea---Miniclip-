Font Library - FTGL3
--------------------
* Download and Install freetype2 library from
  http://download.savannah.gnu.org/releases/freetype/freetype-2.6.2.tar.gz
* Download and install ftgl3 library from 
  https://github.com/lukexi/ftgl3


Simple OpenGL Image Library - SOIL (Textures)
---------------------------------------------
* Download and Install SOIL library from
  http://www.lonesock.net/soil.html


Sample Code - Changes (Fonts)
-----------------------------
* initGL and draw functions have been modified to include font rendering
* Additional camera defined for fonts in draw() which stays fixed while
  the original camera keeps rotating.
* Vertex shader code for rendering font is a bit different from the 
  vertex shader code for rendering other geometry.
* FTExtrude style font rendering is used in example. Other styles can be
  found at:
  http://ftgl.sourceforge.net/docs/html/ftgl-tutorial.html
* Font can be animated and colors changed every frame.


Sample Code - Changes (Textures)
--------------------------------
* initGL and draw functions have been modified to include loading textures
  and corresponding texture shaders
* create3DTexturedObject and draw3DTexturedObject use texture buffers 
  instead of color buffer.
* Vertex and fragment shaders for textured rendering are very different 
  from the shaders of normal rendering.
* NOTE width and height of images used for textures should be power of 2 on
  some graphic cards. (beach2.png - power of two image)
