#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>

#include <stdio.h>
#include <unistd.h>

int width = 800;
int height = 800;
int cam_x =-15, cam_y = 2, cam_z= 0;

float helicamx = 0;
float helicamy = 15;
float helicamz = 0;
int camorient = 0;

int brickplay = 0;
int warpplay = 0;
int dropped = 0;

float eyes1 = cam_x, eyes2 = cam_y , eyes3 = cam_z,t1 = 0 ,t2 = 0,t3 = 0,u1 = 0,u2 = 1,u3 = 0;
float pu1 = 1, pu2 = 0 , pu3 = 0;
float pt1 , pt2 , pt3;
int premove = 0;
int movement ;

using namespace std;
  GLuint textureID1 ;
  GLuint textureID2 ;
  GLuint textureID3 ;
  GLuint textureID4 ;
  GLuint textureID5 ;
  GLuint textureID51 ;
  GLuint textureID52 ;
  GLuint textureID6 ;
  GLuint textureID7 ;
  GLuint textureID8 ;
  GLuint textureID9 ;
  GLuint textureID10 ;
  GLuint textureID11 ;
  GLuint textureID12 ;
  GLuint textureID13 ;


double last_update_time;
double current_time;
double gamestarttime;
int timespent;


glm::mat4 MVP;  // MVP = Projection * View * Model
glm::mat4 VP;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLuint TextureBuffer;
    GLuint TextureID;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
  GLuint TexMatrixID; // For use with texture shader
} Matrices;

GLuint programID, textureProgramID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;
  vao->TextureID = textureID;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

  glBindVertexArray (vao->VertexArrayID); // Bind the VAO
  glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
  glVertexAttribPointer(
              0,                  // attribute 0. Vertices
              3,                  // size (x,y,z)
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
              );

  glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
  glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
              2,                  // attribute 2. Textures
              2,                  // size (s,t)
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
              );

  return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
  // Change the Fill Mode for this object
  glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

  // Bind the VAO to use
  glBindVertexArray (vao->VertexArrayID);

  // Enable Vertex Attribute 0 - 3d Vertices
  glEnableVertexAttribArray(0);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

  // Bind Textures using texture units
  glBindTexture(GL_TEXTURE_2D, vao->TextureID);

  // Enable Vertex Attribute 2 - Texture
  glEnableVertexAttribArray(2);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

  // Draw the geometry !
  glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

  // Unbind Textures to be safe
  glBindTexture(GL_TEXTURE_2D, 0);
}


/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
  GLuint TextureID;
  // Generate Texture Buffer
  glGenTextures(1, &TextureID);
  // All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
  glBindTexture(GL_TEXTURE_2D, TextureID);
  // Set our texture parameters
  // Set texture wrapping to GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Set texture filtering (interpolation)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Load image and create OpenGL texture
  int twidth, theight;
  unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
  SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
  glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

  return TextureID;
}
  
/**************************
 * Customizable functions *
 **************************/

// int cam_x =-9, cam_y = 11  , cam_z= 0;
float cube_x = -0, cube_y = 2.25  , cube_z= 0, cube_angle = 0;
float cube_yv = 2.25;
float cube_yh = 1.25;

float cube_xnow , cube_ynow,  cube_znow ; 
float before_cube_angle = 0;
int copied_angle = 0;
int moving_up = 0;
int moving_down = 0;
int moving_left = 0;
int moving_right = 0;
int onsomething = 1;

int followmode = 1;

float heli_x = 0;
float heli_y = 0;
float heli_z = 0;

int level = 2;

long steps = 0;
int gamestart = 0;
int gameover = 0;
double overtime ;


/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {

          case GLFW_KEY_LEFT:
                if(camorient == 5)
                  {
                    helicamz--;   
                    break;
                  }
                  cam_x -= 1;
                  break;

          case GLFW_KEY_RIGHT:
                if(camorient == 5)
                  {
                    helicamz++;   
                    break;
                  }                      
                  cam_x += 1;
                  break;

          case GLFW_KEY_UP:
                if(camorient == 5)
                  {
                    helicamx++;   
                    break;
                  }          
                  cam_y += 1;
                  break;

          case GLFW_KEY_DOWN:
                if(camorient == 5)
                  {
                    helicamx--;   
                    break;
                  }

                  cam_y -= 1;                  
                  break;

          case GLFW_KEY_Z:
                  cam_z -= 1;
                  break;

          case GLFW_KEY_X:
                  cam_z += 1;
                  break;

          case GLFW_KEY_P:
                  cout << cam_x << " " << cam_y << " " << cam_z << endl; 
                  break;

          case GLFW_KEY_R:
                  cam_x = -1;
                  cam_y = 10;
                  cam_z = 5;
                  break;

          case GLFW_KEY_A:
                  // cube_x -= 2;
                 movement = moving_left + moving_down + moving_up + moving_right ;
                 if(glfwGetKey(window, GLFW_KEY_SPACE))
                 {
                    cube_z -= 2;
                    break;
                 }
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_left = 1;
                  brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");
                }
                break;

          case GLFW_KEY_S:
                  // cube_z += 2;
                 movement = moving_left + moving_down + moving_up + moving_right ;
                 if(glfwGetKey(window, GLFW_KEY_SPACE))
                 {
                    cube_x -= 2;
                    break;
                 }                 
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_down = 1;
                  brickplay = 1;                  
        int playing = system ("mpg123 tap1.mp3 &");
                }                  
                  break;

          case GLFW_KEY_D:
                  // cube_x += 2;
                 movement = moving_left + moving_down + moving_up + moving_right ;
                 if(glfwGetKey(window, GLFW_KEY_SPACE))
                 {
                    cube_z += 2;
                    break;
                 }                   
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_right = 1;
                  brickplay = 1;                  
        int playing = system ("mpg123 tap1.mp3 &");
                }              
                break;
          case GLFW_KEY_W:
                 movement = moving_left + moving_down + moving_up + moving_right ;
                 if(glfwGetKey(window, GLFW_KEY_SPACE))
                 {
                    cube_x += 2;
                    break;
                 }                  
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_up = 1;
                  brickplay = 1;                  
                  int playing = system ("mpg123 tap1.mp3 &");
                }                        
                  break;

          case GLFW_KEY_1:
                camorient = (camorient == 1)?0:1;
                break;
          
          case GLFW_KEY_2:
                camorient = (camorient == 2)?0:2;
                break;

          case GLFW_KEY_3:
                camorient = (camorient == 3)?0:3;
                break;                

          case GLFW_KEY_4:
                camorient = (camorient == 4)?0:4;
                break; 

          case GLFW_KEY_5:
                camorient = (camorient == 5)?0:5;
                break; 

          case GLFW_KEY_6:
                camorient = (camorient == 6)?0:6;
                break; 

          case GLFW_KEY_LEFT_ALT:
                  switch(followmode)
                  {
                    case 1:
                          followmode = 2;
                          break;
                    case 2: 
                          followmode = 3;
                          break;
                    case 3:
                          followmode = 1;
                          break;
                  }
                  break;

          case GLFW_KEY_SPACE:
               if(gamestart != 1)
                {
                  gamestart = 1;
                  gamestarttime = glfwGetTime();
                  camorient = 1;
                }
                break;
                
          case GLFW_KEY_ESCAPE :
                glfwSetWindowShouldClose(window, GL_TRUE);
            default:
                break;
        }
    }
    else if(action == GLFW_REPEAT)
    {
        switch (key) {

          case GLFW_KEY_LEFT:
                if(camorient == 5)
                  {
                    helicamz--;   
                    break;
                  }
                  cam_x -= 1;
                  break;

          case GLFW_KEY_RIGHT:
                if(camorient == 5)
                  {
                    helicamz++;   
                    break;
                  }                      
                  cam_x += 1;
                  break;

          case GLFW_KEY_UP:
                if(camorient == 5)
                  {
                    helicamx++;   
                    break;
                  }          
                  cam_y += 1;
                  break;

          case GLFW_KEY_DOWN:
                if(camorient == 5)
                  {
                    helicamx--;   
                    break;
                  }

                  cam_y -= 1;                  
                  break;

          case GLFW_KEY_Z:
                  cam_z -= 1;
                  break;

          case GLFW_KEY_X:
                  cam_z += 1;
                  break;

          case GLFW_KEY_P:
                  cout << cam_x << " " << cam_y << " " << cam_z << endl; 
                  break;

          case GLFW_KEY_R:
                  cam_x = -1;
                  cam_y = 10;
                  cam_z = 5;
                  break;

          case GLFW_KEY_A:
                  // cube_x -= 2;
                 movement = moving_left + moving_down + moving_up + moving_right ;
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_left = 1;
                  brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");
                }
                break;

          case GLFW_KEY_S:
                  // cube_z += 2;
                 movement = moving_left + moving_down + moving_up + moving_right ;
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_down = 1;
                  brickplay = 1;                  
        int playing = system ("mpg123 tap1.mp3 &");
                }                  
                  break;

          case GLFW_KEY_D:
                  // cube_x += 2;
                 movement = moving_left + moving_down + moving_up + moving_right ;
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_right = 1;
                  brickplay = 1;                  
        int playing = system ("mpg123 tap1.mp3 &");
                }              
                break;
          case GLFW_KEY_W:
                 movement = moving_left + moving_down + moving_up + moving_right ;
                if((cube_y == 1.25 || cube_y == 2.25 ) && onsomething && gamestart && movement == 0)
                {
                  steps++;
                  moving_up = 1;
                  brickplay = 1;                  
                  int playing = system ("mpg123 tap1.mp3 &");
                }                        
                  break;

          case GLFW_KEY_1:
                camorient = (camorient == 1)?0:1;
                break;
          
          case GLFW_KEY_2:
                camorient = (camorient == 2)?0:2;
                break;

          case GLFW_KEY_3:
                camorient = (camorient == 3)?0:3;
                break;                

          case GLFW_KEY_4:
                camorient = (camorient == 4)?0:4;
                break; 

          case GLFW_KEY_5:
                camorient = (camorient == 5)?0:5;
                break; 

          case GLFW_KEY_6:
                camorient = (camorient == 6)?0:6;
                break; 

          case GLFW_KEY_LEFT_ALT:
                  switch(followmode)
                  {
                    case 1:
                          followmode = 2;
                          break;
                    case 2: 
                          followmode = 3;
                          break;
                    case 3:
                          followmode = 1;
                          break;
                  }
                  break;

          case GLFW_KEY_SPACE:
               if(gamestart != 1)
                {
                  gamestart = 1;
                  gamestarttime = glfwGetTime();
                  camorient = 1;
                }
                break;

            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {

            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {

    default:
      break;
  }
}

double mousex;
double mousey;
double mousexp;
double mouseyp;
double onmousex;
double onmousey;

void getMousePosition(GLFWwindow* window)
{
        glfwGetCursorPos(window, &mousex, &mousey);
        mousexp = mousex;
        mouseyp = mousey;
        mousex =  (mousex / 100);
        mousey =  (mousey / 100);
}

bool lbutton_down;
bool rbutton_down;

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if(GLFW_PRESS == action)
                lbutton_down = true;
            else if(GLFW_RELEASE == action)
                lbutton_down = false;
        }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if(GLFW_PRESS == action)
                rbutton_down = true;
            else if(GLFW_RELEASE == action)
                rbutton_down = false;
        }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(camorient == 0)
  {
    if(yoffset > 0)
    {
        if(cam_y + 1 <= 20)
            cam_y += 1; 
    }
    if(yoffset < 0)
    {
        if(cam_y - 1 >= -20)
            cam_y -= 1; 
    }
  }
  else if(camorient == 5)
  {
    if(yoffset > 0)
    {
        if(helicamy + 1 <= 20)
            helicamy += 1; 
    }
    if(yoffset < 0)
    {
        if(helicamy - 1 >= -20)
            helicamy -= 1; 
    }
  }

}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
   // glMatrixMode (GL_PROJECTION);
   //   glLoadIdentity ();
   //   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); 
  // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views  
    Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // // Ortho projection for 2D views
    // Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}


glm::mat4 translateMatrix(double x,double y,double z)
{
    glm::mat4 translateShape = glm::translate (glm::vec3(x, y, z));        // glTranslatef
    return translateShape;
}

glm::mat4 rotateMatrix(double angle, double x1, double y1, double z1)
{
    glm::mat4 rotateShape;
    rotateShape = glm::rotate((float)(angle * M_PI/180.0f), glm::vec3(x1,y1,z1)); // rotate about vector (-1,1,1)
    return rotateShape;
}

glm::mat4 scaleMatrix(double x,double y,double z)
{
    glm::mat4 scaleShape = glm::scale (glm::vec3(x, y, z));        // glTranslatef
    return scaleShape;
}


glm::mat4 tmax = translateMatrix(cube_x,cube_y,cube_z);
glm::vec4 final;
glm::vec4 origin = glm::vec4( glm::vec3( 0.0 , 0.0 , 0.0 ), 1.0 );

static const GLfloat tile_vertex_buffer_data[] = {
   
    1.0f, 0.25f, 1.0f,
    -1.0f, 0.25f,-1.0f,
    -1.0f, 0.25f, 1.0f,
    1.0f, 0.25f, 1.0f,
    1.0f, 0.25f,-1.0f,
    -1.0f, 0.25f,-1.0f,

    1.0f,-0.25f, 1.0f,
    -1.0f,-0.25f,-1.0f,
    1.0f,-0.25f,-1.0f,
    1.0f,-0.25f, 1.0f,
    -1.0f,-0.25f, 1.0f,
    -1.0f,-0.25f,-1.0f,

    -1.0f, 0.25f, 1.0f, // triangle 1 : end
    -1.0f,-0.25f,-1.0f, // triangle 1 : begin
    -1.0f,-0.25f, 1.0f,
    -1.0f, 0.25f, 1.0f,
    -1.0f, 0.25f,-1.0f,
    -1.0f,-0.25f,-1.0f,

    1.0f, 0.25f, 1.0f, // triangle 1 : end
    1.0f,-0.25f,-1.0f, // triangle 1 : begin
    1.0f,-0.25f, 1.0f,
    1.0f, 0.25f, 1.0f,
    1.0f, 0.25f,-1.0f,
    1.0f,-0.25f,-1.0f,

    1.0f, 0.25f,-1.0f, // triangle 2 : begin
    -1.0f,-0.25f,-1.0f,
    -1.0f, 0.25f,-1.0f, // triangle 2 : end
    1.0f, 0.25f,-1.0f,
    1.0f,-0.25f,-1.0f,
    -1.0f,-0.25f,-1.0f,

    -1.0f, -0.25f, 1.0f,
    1.0f, 0.25f, 1.0f,
    -1.0f,0.25f, 1.0f,
    -1.0f, -0.25f, 1.0f,
    1.0f,-0.25f, 1.0f,
    1.0f,0.25f, 1.0f,

};

// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
  static const GLfloat tile_texture_buffer_data [] = {
    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,1, // TexCoord 1 - bot left
    0,0, // TexCoord 2 - bot right
    0,1, // TexCoord 3 - top right
    1,1, // TexCoord 3 - top right
    1,0, // TexCoord 4 - top left
    0,0 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

  };

static const GLfloat arrow_vertex_buffer_data[] = {
   
    0.5f, 0.25f, 0.5f,
    -0.5f, 0.25f,-0.5f,
    -0.5f, 0.25f, 0.5f,
    0.5f, 0.25f, 0.5f,
    0.5f, 0.25f,-0.5f,
    -0.5f, 0.25f,-0.5f,

    0.5f,-0.25f, 0.5f,
    -0.5f,-0.25f,-0.5f,
    0.5f,-0.25f,-0.5f,
    0.5f,-0.25f, 0.5f,
    -0.5f,-0.25f, 0.5f,
    -0.5f,-0.25f,-0.5f,

    -0.5f, 0.25f, 0.5f, // triangle 0 5 end
    -0.5f,-0.25f,-0.5f, // triangle 0 5 begin
    -0.5f,-0.25f, 0.5f,
    -0.5f, 0.25f, 0.5f,
    -0.5f, 0.25f,-0.5f,
    -0.5f,-0.25f,-0.5f,

    0.5f, 0.25f, 0.5f, // triangle 0 5 end
    0.5f,-0.25f,-0.5f, // triangle 0 5 begin
    0.5f,-0.25f, 0.5f,
    0.5f, 0.25f, 0.5f,
    0.5f, 0.25f,-0.5f,
    0.5f,-0.25f,-0.5f,

    0.5f, 0.25f,-0.5f, // triangle 2 : begin
    -0.5f,-0.25f,-0.5f,
    -0.5f, 0.25f,-0.5f, // triangle 2 : end
    0.5f, 0.25f,-0.5f,
    0.5f,-0.25f,-0.5f,
    -0.5f,-0.25f,-0.5f,

    -0.5f, -0.25f, 0.5f,
    0.5f, 0.25f, 0.5f,
    -0.5f,0.25f, 0.5f,
    -0.5f, -0.25f, 0.5f,
    0.5f,-0.25f, 0.5f,
    0.5f,0.25f, 0.5f,

};

static const GLfloat tile_n_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f, 
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
};

static const GLfloat tile_s_color_buffer_data[] = {
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
        0.902f, 0.902f, 0.980f,
};

static const GLfloat tile_bridge_color_buffer_data[] = {
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
        0.467f, 0.533f, 0.600f ,
};

static const GLfloat tile_switch_color_buffer_data[] = {
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
    0.000f, 1.000f, 1.000f,
};


static const GLfloat tile_finish_color_buffer_data[] = {
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
    0.000f, 0.000f, 0.000f,
};

class Tile
{
    public:
      VAO *tile_n;
      VAO *tile_s;
      VAO *tile_switch;
      VAO *tile_finish;
      VAO *tile_bridge;
      VAO *tile_up;
      VAO *tile_mid;
      VAO *tile_view1;
      VAO *tile_view2;
      VAO *tile_view3;
      VAO *tile_view4;
      VAO *tile_view5;
      VAO *tile_light;
      VAO *tile_port;

      void generateShapeData();
      void drawShape(glm::mat4, int);
      float posx, posy, posz;
};

void Tile::drawShape(glm::mat4 tmatrix, int col)
{
  Matrices.model = glm::mat4(1.0f); 
  Matrices.model *= tmatrix;
  MVP = VP * Matrices.model;
  
  if(col >= 6)
  {
    glUseProgram (programID);
    VP = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 500.0f)  * glm::lookAt( glm::vec3( 0, 15, 0 ), glm::vec3( 0, 0, 0 ), glm::vec3( 1, 0, 0 ) );
    Matrices.model = glm::mat4(1.0f); 

    Matrices.model *= tmatrix;
    MVP = VP * Matrices.model;
  }
  if(col == -1)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_light);
  }
  else if(col == 0)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_port);
  }   
  else if(col == 1)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_n);
  }
  else if(col == 2)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_s);
  }
  else if(col == 3)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_switch);
  }
  else if(col == 4)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_finish);
  }
  else if(col == 5)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(tile_bridge);
  }
    else if(col >= 6)
  {
  glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
    switch(col)
    {
      case 6:
          draw3DTexturedObject(tile_up);
          break;
      case 7:
          draw3DTexturedObject(tile_mid);
          break;
      case 8:
          draw3DTexturedObject(tile_view1);
          break;
      case 9:
          draw3DTexturedObject(tile_view2);
          break;
      case 10:
          draw3DTexturedObject(tile_view3);
          break;
      case 11:
          draw3DTexturedObject(tile_view4);
          break; 
      case 12:
          draw3DTexturedObject(tile_view5);
          break;                                                 
    }
  }
}

void Tile::generateShapeData()
{
  // tile_n = create3DObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_n_color_buffer_data, GL_FILL);
  tile_n = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID6, GL_FILL);
  tile_s = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID1, GL_FILL);
  tile_switch = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID5, GL_FILL);
  // tile_switch = create3DObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_switch_color_buffer_data, GL_FILL);
  // tile_finish = create3DObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_finish_color_buffer_data, GL_FILL);
  tile_finish = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID4, GL_FILL);
  tile_bridge = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID2, GL_FILL);
  tile_up = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID7, GL_FILL);
  tile_mid = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID8, GL_FILL);
 // tile_bridge = create3DObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_bridge_color_buffer_data, GL_FILL);
  tile_view1 = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID9, GL_FILL);
  tile_view2 = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID10, GL_FILL);
  tile_view3 = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID11, GL_FILL);
  tile_view4 = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID12, GL_FILL);
  tile_view5 = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID13, GL_FILL);

  tile_light = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID51, GL_FILL);
  tile_port = create3DTexturedObject(GL_TRIANGLES, 12*3, tile_vertex_buffer_data, tile_texture_buffer_data, textureID52, GL_FILL);

}


static const GLfloat cube_color_buffer_data[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
};

static const GLfloat vcube_texture_buffer_data[] = {
    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,1, // TexCoord 1 - bot left
    0,0, // TexCoord 2 - bot right
    0,1, // TexCoord 3 - top right
    1,1, // TexCoord 3 - top right
    1,0, // TexCoord 4 - top left
    0,0 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left

    1,0, // TexCoord 1 - bot left
    0,1, // TexCoord 2 - bot right
    0,0, // TexCoord 3 - top right
    1,0, // TexCoord 3 - top right
    1,1, // TexCoord 4 - top left
    0,1 , // TexCoord 1 - bot left
};


static const GLfloat vcube_vertex_buffer_data[] = {
    1.0f, 2.0f, 1.0f,
    -1.0f, 2.0f,-1.0f,
    -1.0f, 2.0f, 1.0f,
    1.0f, 2.0f, 1.0f,
    1.0f, 2.0f,-1.0f,
    -1.0f, 2.0f,-1.0f,

    1.0f,-2.0f, 1.0f,
    -1.0f,-2.0f,-1.0f,
    1.0f,-2.0f,-1.0f,
    1.0f,-2.0f, 1.0f,
    -1.0f,-2.0f, 1.0f,
    -1.0f,-2.0f,-1.0f,

    -1.0f, 2.0f, 1.0f, // triangle 1 : end
    -1.0f,-2.0f,-1.0f, // triangle 1 : begin
    -1.0f,-2.0f, 1.0f,
    -1.0f, 2.0f, 1.0f,
    -1.0f, 2.0f,-1.0f,
    -1.0f,-2.0f,-1.0f,

    1.0f, 2.0f, 1.0f, // triangle 1 : end
    1.0f,-2.0f,-1.0f, // triangle 1 : begin
    1.0f,-2.0f, 1.0f,
    1.0f, 2.0f, 1.0f,
    1.0f, 2.0f,-1.0f,
    1.0f,-2.0f,-1.0f,

    1.0f, 2.0f,-1.0f, // triangle 2 : begin
    -1.0f,-2.0f,-1.0f,
    -1.0f, 2.0f,-1.0f, // triangle 2 : end
    1.0f, 2.0f,-1.0f,
    1.0f,-2.0f,-1.0f,
    -1.0f,-2.0f,-1.0f,

    -1.0f, -2.0f, 1.0f,
    1.0f, 2.0f, 1.0f,
    -1.0f,2.0f, 1.0f,
    -1.0f, -2.0f, 1.0f,
    1.0f,-2.0f, 1.0f,
    1.0f,2.0f, 1.0f,
};

static const GLfloat hcube_vertex_buffer_data[] = {
    -1.0f,-1.0f,-2.0f, // triangle 1 : begin
    -1.0f,-1.0f, 2.0f,
    -1.0f, 1.0f, 2.0f, // triangle 1 : end
    1.0f, 1.0f,-2.0f, // triangle 2 : begin
    -1.0f,-1.0f,-2.0f,
    -1.0f, 1.0f,-2.0f, // triangle 2 : end
    1.0f,-1.0f, 2.0f,
    -1.0f,-1.0f,-2.0f,
    1.0f,-1.0f,-2.0f,
    1.0f, 1.0f,-2.0f,
    1.0f,-1.0f,-2.0f,
    -1.0f,-1.0f,-2.0f,
    -1.0f,-1.0f,-2.0f,
    -1.0f, 1.0f, 2.0f,
    -1.0f, 1.0f,-2.0f,
    1.0f,-1.0f, 2.0f,
    -1.0f,-1.0f, 2.0f,
    -1.0f,-1.0f,-2.0f,
    -1.0f, 1.0f, 2.0f,
    -1.0f,-1.0f, 2.0f,
    1.0f,-1.0f, 2.0f,
    1.0f, 1.0f, 2.0f,
    1.0f,-1.0f,-2.0f,
    1.0f, 1.0f,-2.0f,
    1.0f,-1.0f,-2.0f,
    1.0f, 1.0f, 2.0f,
    1.0f,-1.0f, 2.0f,
    1.0f, 1.0f, 2.0f,
    1.0f, 1.0f,-2.0f,
    -1.0f, 1.0f,-2.0f,
    1.0f, 1.0f, 2.0f,
    -1.0f, 1.0f,-2.0f,
    -1.0f, 1.0f, 2.0f,
    1.0f, 1.0f ,2.0f,
    -1.0f, 1.0f, 2.0f,
    1.0f,-1.0f, 2.0f
};

static const GLfloat hacube_vertex_buffer_data[] = {
    -2.0f,-1.0f,-1.0f, // triangle 1 : begin
    -2.0f,-1.0f, 1.0f,
    -2.0f, 1.0f, 1.0f, // triangle 1 : end
    2.0f, 1.0f,-1.0f, // triangle 2 : begin
    -2.0f,-1.0f,-1.0f,
    -2.0f, 1.0f,-1.0f, // triangle 2 : end
    2.0f,-1.0f, 1.0f,
    -2.0f,-1.0f,-1.0f,
    2.0f,-1.0f,-1.0f,
    2.0f, 1.0f,-1.0f,
    2.0f,-1.0f,-1.0f,
    -2.0f,-1.0f,-1.0f,
    -2.0f,-1.0f,-1.0f,
    -2.0f, 1.0f, 1.0f,
    -2.0f, 1.0f,-1.0f,
    2.0f,-1.0f, 1.0f,
    -2.0f,-1.0f, 1.0f,
    -2.0f,-1.0f,-1.0f,
    -2.0f, 1.0f, 1.0f,
    -2.0f,-1.0f, 1.0f,
    2.0f,-1.0f, 1.0f,
    2.0f, 1.0f, 1.0f,
    2.0f,-1.0f,-1.0f,
    2.0f, 1.0f,-1.0f,
    2.0f,-1.0f,-1.0f,
    2.0f, 1.0f, 1.0f,
    2.0f,-1.0f, 1.0f,
    2.0f, 1.0f, 1.0f,
    2.0f, 1.0f,-1.0f,
    -2.0f, 1.0f,-1.0f,
    2.0f, 1.0f, 1.0f,
    -2.0f, 1.0f,-1.0f,
    -2.0f, 1.0f, 1.0f,
    2.0f, 1.0f ,1.0f,
    -2.0f, 1.0f, 1.0f,
    2.0f,-1.0f, 1.0f
};

class Cube
{
    public:
      VAO *hcube;
      VAO *hacube;
      VAO *vcube;
      void generateShapeData();
      void drawShape(glm::mat4, int);
      float posx, posy, posz;
      int state;
};

void Cube::drawShape(glm::mat4 tmatrix, int state)
{
  glUseProgram (programID);

  Matrices.model = glm::mat4(1.0f); 
  Matrices.model *= tmatrix;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(state == 1)
  {
      glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(vcube);
    // draw3DObject(vcube);
  }
  else if(state == 2)
  {
      glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(hcube);
    // draw3DObject(hcube);
  }
  else
  {
      glUseProgram(textureProgramID);
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(hacube);
    // draw3DObject(hacube);
  }
}

void Cube::generateShapeData()
{
    state = 1;
    // hcube = create3DObject(GL_TRIANGLES, 12*3, hcube_vertex_buffer_data, cube_color_buffer_data, GL_FILL);
    // hvcube = create3DObject(GL_TRIANGLES, 12*3, hcube_vertex_buffer_data, cube_color_buffer_data, GL_FILL);
    // hacube = create3DObject(GL_TRIANGLES, 12*3, hacube_vertex_buffer_data, cube_color_buffer_data, GL_FILL);
    // hacube = create3DObject(GL_TRIANGLES, 12*3, hacube_vertex_buffer_data, cube_color_buffer_data, GL_FILL);
    hcube = create3DTexturedObject(GL_TRIANGLES, 12*3, hcube_vertex_buffer_data, vcube_texture_buffer_data, textureID3, GL_FILL);
    hacube = create3DTexturedObject(GL_TRIANGLES, 12*3, hacube_vertex_buffer_data, vcube_texture_buffer_data, textureID3, GL_FILL);
    vcube = create3DTexturedObject(GL_TRIANGLES, 12*3, vcube_vertex_buffer_data, vcube_texture_buffer_data, textureID3, GL_FILL);
    // vcube = create3DObject(GL_TRIANGLES, 12*3, vcube_vertex_buffer_data, cube_color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;

Tile *tile = new Tile();
Cube *cube = new Cube();


void moveByV()
{
    if(moving_down)
  {
    if(cube_angle <= 90)
    {
        cube_angle += 4;
        tmax = translateMatrix(cube_x - 1,0.25,cube_z) * rotateMatrix(cube_angle,0,0,1) * translateMatrix(1,2,0);
        cube->drawShape(tmax , 1);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];
        cube_znow = final[2];
    }
    else
      {
        tmax = translateMatrix(cube_x - 1,0.25,cube_z) * rotateMatrix(90,0,0,1) * translateMatrix(1,2,0) ;
        final = tmax * origin ;
        // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube->state = (cube->state == 1)?3:1;
        cube->drawShape( tmax , 1);
        cube_angle = 0 ;
        moving_down = 0;
      }
  }
  else if(moving_up)
  {
    if(cube_angle >= -90)
    {
      cube_angle -= 4;
      tmax = translateMatrix(cube_x + 1,0.25,cube_z) * rotateMatrix(cube_angle,0,0,1) * translateMatrix(-1,2,0) ;
      cube->drawShape(tmax , 1);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];
        cube_znow = final[2];      
    }
  else
    {
      tmax = translateMatrix(cube_x + 1,0.25,cube_z) * rotateMatrix(-90,0,0,1) * translateMatrix(-1,2,0) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
      cube->drawShape( tmax , 1);
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube->state = (cube->state == 1)?3:1;
        cube_angle = 0 ;
      moving_up = 0;
    }
  }
  else if(moving_left)
  {
    if(cube_angle >= -90)
    {
      cube_angle -= 4;
      tmax = translateMatrix(cube_x,0.25,cube_z - 1) * rotateMatrix(cube_angle,1,0,0) * translateMatrix(0,2,1);
      cube->drawShape(tmax , 1);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];
        cube_znow = final[2];       
    }
    else
     {
      tmax = translateMatrix(cube_x,0.25,cube_z - 1) * rotateMatrix(-90,1,0,0) * translateMatrix(0,2,1) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = (cube->state == 1)?2:1;
      cube->drawShape( tmax , 1);      
      moving_left = 0;
     }
  }
  else if(moving_right)
  {
    if(cube_angle <= 90) 
    {
      cube_angle += 4;
      tmax = translateMatrix(cube_x,0.25,cube_z + 1) * rotateMatrix(cube_angle,1,0,0) * translateMatrix(0,2,-1);
      cube->drawShape(tmax , 1);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];
        cube_znow = final[2];        

    }
  else
     {
      tmax = translateMatrix(cube_x,0.25,cube_z + 1) * rotateMatrix(90,1,0,0) * translateMatrix(0,2,-1) ;
      final = tmax * origin ;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];        
        cube_angle = 0 ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube->state = (cube->state == 1)?2:1;
      cube->drawShape( tmax , 1);      
      moving_right = 0;
     }
  }
  else
  {
      cube->drawShape(translateMatrix(cube_x,cube_y,cube_z),1);
      cube_xnow = cube_x;
      cube_ynow = cube_y;      
      cube_znow = cube_z;
  }
}

void moveByH()
{
  if(moving_down)
  {
    if(cube_angle <= 90)
    {
        cube_angle += 4;
        tmax = translateMatrix(cube_x - 1,0.25,cube_z) * rotateMatrix(cube_angle,0,0,1) * translateMatrix(1,1,0) ;
        cube->drawShape(tmax, 2);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];         
    }
    else
      {
        tmax = translateMatrix(cube_x - 1,0.25,cube_z) * rotateMatrix(90,0,0,1) * translateMatrix(1,1,0) ;
        final = tmax * origin ;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube->state = 2;
        cube->drawShape( tmax , 2);
        moving_down = 0;
      }
  }
  else if(moving_up)
  {
    if(cube_angle >= -90)
    {
      cube_angle -= 4;
      tmax = translateMatrix(cube_x + 1,0.25,cube_z) * rotateMatrix(cube_angle,0,0,1) * translateMatrix(-1,1,0);
      cube->drawShape(tmax , 2);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];
        cube_znow = final[2];       
    }
  else
    {
      tmax = translateMatrix(cube_x + 1,0.25,cube_z) * rotateMatrix(-90,0,0,1) * translateMatrix(-1,1,0) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = 2;
      cube->drawShape( tmax , 2);
      moving_up = 0;
    }
  }
  else if(moving_left)
  {
    if(cube_angle >= -90)
    {
      cube_angle -= 4;
      tmax = translateMatrix(cube_x,0.25,cube_z - 2) * rotateMatrix(cube_angle,1,0,0) * translateMatrix(0,1,2);
      cube->drawShape(tmax , 2);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];       
    }
    else
     {

      tmax = translateMatrix(cube_x,0.25,cube_z - 2) * rotateMatrix(-90,1,0,0) * translateMatrix(0,1,2) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = (cube->state == 1)?2:1;
      cube->drawShape( tmax , 2);      
      moving_left = 0;
     }
  }
  else if(moving_right)
  {
    if(cube_angle <= 90)
    {
      cube_angle += 4;
      tmax = translateMatrix(cube_x,0.25,cube_z + 2) * rotateMatrix(cube_angle,1,0,0) * translateMatrix(0,1,-2);
      cube->drawShape(tmax , 2);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];   
    }  
  else
     {
      tmax = translateMatrix(cube_x,0.25,cube_z + 2) * rotateMatrix(90,1,0,0) * translateMatrix(0,1,-2) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = (cube->state == 1)?2:1;
      cube->drawShape( tmax , 2);      
      moving_right = 0;
     }
  }
  else
  {
      cube->drawShape(translateMatrix(cube_x,cube_yh,cube_z),2);
      cube_xnow = cube_x;
      cube_ynow = cube_yh;      
      cube_znow = cube_z;      
  }
}

void moveByHA()
{
  if(moving_down)
  {
    if(cube_angle <= 90)
    {
        cube_angle += 4;
        tmax = translateMatrix(cube_x - 2,0.25,cube_z) * rotateMatrix(cube_angle,0,0,1) * translateMatrix(2,1,0) ;
        cube->drawShape(tmax, 3);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];           
    }
    else
      {
        tmax = translateMatrix(cube_x - 2,0.25,cube_z) * rotateMatrix(90,0,0,1) * translateMatrix(2,1,0) ;
        final = tmax * origin ;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube->state = 1;
        cube->drawShape( tmax , 3);
        moving_down = 0;
      }
  }
  else if(moving_up)
  {
    if(cube_angle >= -90)
     { 
      cube_angle -= 4;
      tmax = translateMatrix(cube_x + 2,0.25,cube_z) * rotateMatrix(cube_angle,0,0,1) * translateMatrix(-2,1,0);
      cube->drawShape(tmax , 3);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];         
     }
  else
    {
      tmax = translateMatrix(cube_x + 2,0.25,cube_z) * rotateMatrix(-90,0,0,1) * translateMatrix(-2,1,0) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = 1;
      cube->drawShape( tmax , 3);
      moving_up = 0;
    }
  }
  else if(moving_left)
  {
    if(cube_angle >= -90)
     {
      cube_angle -= 4;
      tmax = translateMatrix(cube_x,0.25,cube_z - 1) * rotateMatrix(cube_angle,1,0,0) * translateMatrix(0,1,1);
      cube->drawShape(tmax , 3);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];         
     }
  else
     {
      tmax = translateMatrix(cube_x,0.25,cube_z - 1) * rotateMatrix(-90,1,0,0) * translateMatrix(0,1,1) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = 3;
      cube->drawShape( tmax , 3);      
      moving_left = 0;
     }
  }
  else if(moving_right)
  {
    if(cube_angle <= 90)
    {
      cube_angle += 4;
      tmax = translateMatrix(cube_x,0.25,cube_z + 1) * rotateMatrix(cube_angle,1,0,0) * translateMatrix(0,1,-1);
      cube->drawShape(tmax , 3);
        final = tmax * origin ;
        cube_xnow = final[0];
        cube_ynow = final[1];        
        cube_znow = final[2];         
    } 
  else
     {
      tmax = translateMatrix(cube_x,0.25,cube_z + 1) * rotateMatrix(90,1,0,0) * translateMatrix(0,1,-1) ;
      final = tmax * origin ;
      // cout << final[0] << " " << final[1] << " " << final[2] << endl;
        cube_x = final[0];
        cube_y = final[1];
        cube_z = final[2];
        cube_angle = 0 ;
        cube->state = 3;
      cube->drawShape( tmax , 3);      
      moving_right = 0;
     }
  }
  else
  {
      cube->drawShape(translateMatrix(cube_x,cube_yh,cube_z),3);
      cube_xnow = cube_x;
      cube_ynow = cube_yh;      
      cube_znow = cube_z;
  }
}

struct pos
{
  double x;
  double y; 
  double z;
  int color;
};

double camposxf ;
int copiedcam = 0;
double camposx ;
bool camhelicopied = 0;
vector<pos> positions;
vector<pos> positions1;
float scaledown = 1;
/* Render the scene with openGL */
/* Edit this function according to your assignment */


struct Switch
{
  float x;
  float y;
  float z;
  float bx;
  float by;
  float bz;
  int color;
  int flag;
  int ischanged;
  float number;
};

vector <Switch> switches;
vector <Switch> switches1;



class mydigit
{
    public:
      VAO *digit;
      GLfloat vertex_buffer_data [90];
      GLfloat color_buffer_data [90] ;
      int sides;
      double sizes;
      void generateShapeData(double, double, double, double);
      void drawShape(glm::mat4);
      mydigit();
};

mydigit::mydigit(void)
{
    sides = 4;
}

void mydigit::drawShape(glm::mat4 tmatrix)
{
  glUseProgram (programID);

  VP = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 500.0f)  * glm::lookAt( glm::vec3( 0, 15, 0 ), glm::vec3( 0, 0, 0 ), glm::vec3( 1, 0, 0 ) );

  Matrices.model = glm::mat4(1.0f); 

  Matrices.model *= tmatrix;
  MVP = VP * Matrices.model;
  
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(digit);
}

void mydigit::generateShapeData(double r1, double g1, double b1, double size)
{
    // GL3 accepts only Triangles. Quads are not supported
      int n = sides;
 
  sizes = size;

   GLfloat vertex_buffer_data [] = {
    -4,0,1, 
    4,0,1, 
    4,0,-1, 

    -4,0,1, 
    4,0,-1, 
    -4,0,-1,

    -4,0,1, 
    -6,0,0, 
    -4,0,-1,

    6,0,0,
    4,0,1, 
    4,0,-1, 

  };
      for (int i = 0; i < n*9; ++i)
       {
          // std::cout << vertex_buffer_data[i] << std::endl;;
          vertex_buffer_data[i] *= 1*size; 
       }

  double col[9] = {r1,g1,b1,r1,g1,b1,r1,g1,b1};
  int k = 0;
  for (int i = 0; i < n*9;++i)
      color_buffer_data[i] = col[i%9];

  // create3DObject creates and returns a handle to a VAO that can be used later
  digit = create3DObject(GL_TRIANGLES, n*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}


mydigit *dig1 = new mydigit();
mydigit *dig2 = new mydigit();

int data[10][7] = { 
                    {0,1,1,1,1,1,1},
                    {0,0,0,0,0,1,1},
                    {1,1,1,0,1,1,0},
                    {1,1,1,0,0,1,1},
                    {1,0,0,1,0,1,1},
                    {1,1,1,1,0,0,1},
                    {1,1,1,1,1,0,1},
                    {0,1,0,0,0,1,1},
                    {1,1,1,1,1,1,1},
                    {1,1,1,1,0,1,1},
                  };

void drawcharacter(int col,double dx,double dyy ,double dy, int d1,int d2, int d3, int d4,int d5,int d6, int d7)
{

  if(col == 1)
  {
    if(d1)
      dig1->drawShape(translateMatrix(dx + 0,dyy,dy) * rotateMatrix(90,0,1,0));
    if(d2)
      dig1->drawShape(translateMatrix(dx + 1.2,dyy,dy) * rotateMatrix(90,0,1,0));
    if(d3)
      dig1->drawShape(translateMatrix(dx + -1.2,dyy,dy) * rotateMatrix(90,0,1,0));
    if(d4)
      dig1->drawShape(translateMatrix(dx + 0.6,dyy,-0.6 + dy) );
    if(d5)
      dig1->drawShape(translateMatrix(dx + -0.6,dyy,-0.6 + dy) );
    if(d6)
      dig1->drawShape(translateMatrix(dx + 0.6,dyy,0.6 + dy) );
    if(d7)
      dig1->drawShape(translateMatrix(dx + -0.6,dyy,0.6 + dy) );
  }
  else if(col == 2)
  {
    if(d1)
      dig2->drawShape(translateMatrix(dx + 0,dyy,dy) * rotateMatrix(90,0,1,0));
    if(d2)
      dig2->drawShape(translateMatrix(dx + 1.2,dyy,dy) * rotateMatrix(90,0,1,0));
    if(d3)
      dig2->drawShape(translateMatrix(dx + -1.2,dyy,dy) * rotateMatrix(90,0,1,0));
    if(d4)
      dig2->drawShape(translateMatrix(dx + 0.6,dyy,-0.6 + dy) );
    if(d5)
      dig2->drawShape(translateMatrix(dx + -0.6,dyy,-0.6 + dy) );
    if(d6)
      dig2->drawShape(translateMatrix(dx + 0.6,dyy,0.6 + dy) );
    if(d7)
      dig2->drawShape(translateMatrix(dx + -0.6,dyy,0.6 + dy) );
  }

}


void drawcharacter( int col, std::string s,double dx, double dyy , double dy)
{
  int digit;
  for (int i = 0; i < s.length(); ++i)
  {
    if(i != 0)
      dy += 2;
    digit = (int)(s[i]-'0');
    drawcharacter(col , dx,dyy,dy, data[digit][0],data[digit][1],data[digit][2],data[digit][3],data[digit][4],data[digit][5],data[digit][6]);
  }
}

VAO* rectangle;

void createRectangle (GLuint textureID)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    0,-20,0, // vertex 1
    0,20,0, // vertex 2
    0, 20,20, // vertex 3

    0, 20,20, // vertex 3
    0, -20,20, // vertex 4
    0,-20,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
  static const GLfloat texture_buffer_data [] = {
    0,1, // TexCoord 1 - bot left
    1,1, // TexCoord 2 - bot right
    1,0, // TexCoord 3 - top right

    1,0, // TexCoord 3 - top right
    0,0, // TexCoord 4 - top left
    0,1  // TexCoord 1 - bot left
  };

  // create3DTexturedObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}

int startmessage[7][7] = {
                            {1,1,1,1,1,1,1}, 
                            {0,0,1,1,1,0,0}, 
                            {0,1,1,1,1,1,1}, 
                            {1,0,0,1,1,1,1}, 
                            {0,1,1,1,1,1,1}, 
                            {0,1,0,1,1,0,0}, 
                            {1,1,1,0,1,1,0}, 
                       }; 

int pressspace[11][7] = {
                          {1,1,0,1,1,1,0},
                          {0,1,0,1,1,0,0},
                          {1,1,1,1,1,0,0},
                          {1,1,1,1,0,0,1},
                          {1,1,1,1,0,0,1},
                          {0,0,0,0,0,0,0},
                          {1,1,1,1,0,0,1},
                          {1,1,0,1,1,1,0},
                          {1,1,0,1,1,1,1},
                          {0,1,1,1,1,0,0},
                          {1,1,1,1,1,0,0},
                       }; 


int endmessage[8][7] = {
                          {1,0,1,1,0,1,1},
                          {0,1,1,1,1,1,1},
                          {0,0,1,1,1,1,1},
                          {0,0,0,0,0,0,0},
                          {0,0,1,1,1,0,0},
                          {0,1,1,1,1,1,1},
                          {1,1,1,1,0,0,1},
                          {1,1,1,1,1,0,0},
                       }; 

int winmessage[8][7] = {
                          {1,0,1,0,1,1,1}, // D -
                          {0,1,1,1,1,1,1}, // O -
                          {0,1,0,1,1,1,1}, // N 
                          {1,1,1,1,1,0,0}, // E -
                       }; 

int countopen = 0;
int flag = 1;

void drawstartbanner(int col,double k1,double k3, double k2)
{
  double dx = -1.3 + k1;
  double dx1 = -4.2 + k1;
  double dy = k2;
  // cout << countopen << endl;
  if(countopen == 200)
    {
      flag = flag * -1;
      countopen = 0;
    }
  if(flag == 1)
   {
    for (int i = 0; i < 7; ++i)
    {
      if(i != 0)
        dx += 1.5;
      drawcharacter(1,dy,k3,dx, startmessage[i][0],startmessage[i][1],startmessage[i][2],startmessage[i][3],startmessage[i][4],startmessage[i][5],startmessage[i][6]);
    }
  }
  else
  {
    for (int i = 0; i < 11; ++i)
    {
      if(i != 0)
        dx1 += 1.5;
      drawcharacter(1,dy,k3,dx1, pressspace[i][0],pressspace[i][1],pressspace[i][2],pressspace[i][3],pressspace[i][4],pressspace[i][5],pressspace[i][6]);
    }
  }
}

void drawendbanner(double k1,double k3, double k2)
// void drawendbanner()
{
  double dx = -1.8 + k1;
  double dy = k2;
  for (int i = 0; i < 8; ++i)
  {
    if(i != 0)
      dx += 1.5;
    drawcharacter(1,dy,k3,dx, endmessage[i][0],endmessage[i][1],endmessage[i][2],endmessage[i][3],endmessage[i][4],endmessage[i][5],endmessage[i][6]);
  }
}

void drawwinbanner(double k1,double k3, double k2)
// void drawendbanner()
{
  double dx = -1.8 + k1;
  double dy = k2;
  for (int i = 0; i < 4; ++i)
  {
    if(i != 0)
      dx += 1.5;
    drawcharacter(1,dy,k3,dx, winmessage[i][0],winmessage[i][1],winmessage[i][2],winmessage[i][3],winmessage[i][4],winmessage[i][5],winmessage[i][6]);
  }
}

int alldown = 0;
int downflag = 0;


void reset()
{
    dropped = 0;
    warpplay = 0;
    brickplay = 0;
    cam_x =-15;
    cam_y = 2;
    cam_z= 0;
    helicamx = 0;
    helicamy = 15;
    helicamz = 0;
    camorient = 0;
    cube_x = -0;
    cube_y = 2.25;
    cube_z= 0;
    cube_angle = 0;
    cube_yv = 2.25;
    cube_yh = 1.25;
    cube->state = 1;
    cube_xnow = 0;
    cube_znow = 0 ; 
    before_cube_angle = 0;
    copied_angle = 0;
    moving_up = 0;
    moving_down = 0;
    moving_left = 0;
    moving_right = 0;
    onsomething = 1;
    followmode = 1;
    heli_x = 0;
    heli_y = 0;
    heli_z = 0;
    steps = 0;
    gamestart = 0;
    if(gameover != 1)
      level = (level == 1)?2:1;
    alldown = 0;
    downflag = 0;

vector <int> data = {
                  0 ,  0 ,  2,
                  2 ,  0 ,  /*rand()%2*/ 1,
                  -2 , 0 ,  /*rand()%2*/ 2,
                  2 ,  -2 ,  /*rand()%2*/ 2,
                  2 ,  2 ,  /*rand()%2*/ 2,
                  -4 , 0 ,  /*rand()%2*/ 2,
                  -4 , -2 ,  /*rand()%2*/ 2,
                  -4 , 2 ,  /*rand()%2*/ 2,
                  -6 , 2 ,  /*rand()%2*/ 2,
                  -6 , -2 ,  /*rand()%2*/ 2,
                  -6 , 0 ,  /*rand()%2*/ 2,
                  4 ,  0 ,  /*rand()%2*/ 2,
                  6 ,  0 ,  /*rand()%2*/ 2,
                  8 ,  0 ,  /*rand()%2*/ 2,
                  10 , 0 ,  /*rand()%2*/ 2,
                  12 , 0 ,  /*rand()%2*/ 2,
                  14 , 0 ,  /*rand()%2*/ 1,
                  14 , -2 ,  /*rand()%2*/ 1,
                  14 , -4 ,  /*rand()%2*/ 1,
                  14 , -6 ,  /*rand()%2*/ 1,
                  14 , -8 ,  /*rand()%2*/ 1,
                  14 , -10 ,  /*rand()%2*/ 1,
                  16 , -2 ,  /*rand()%2*/ 1,
                  16 , -4 ,  /*rand()%2*/ 1,
                  16 , -6 ,  /*rand()%2*/ 1,
                  16 , -8 ,  /*rand()%2*/ 1,
                  16 , -10 ,  /*rand()%2*/ 1,
                  16 , 0 ,  /*rand()%2*/ 1,
                  // 12 , -10 ,  /*rand()%2*/ 2,
                  10 , -10 ,  /*rand()%2*/ 2,
                  8 , -10 ,  /*rand()%2*/ 2,
                  // 6 , -10 ,  /*rand()%2*/ 2,
                  4 , -10 ,  /*rand()%2*/ 2,
                  2 , -10 ,  /*rand()%2*/ 2,
                  0 , -10 ,  /*rand()%2*/ 2,
                  -2 , -10 ,  /*rand()%2*/ 2,
                  0 , -12 ,  /*rand()%2*/ 2,
                  0 , -8 ,  /*rand()%2*/ 2,
                  -4 , -10 ,  /*rand()%2*/ 2,
                  -6 , -10 ,  /*rand()%2*/ 2,
                  -6 , -8 ,  /*rand()%2*/ 2,
                  -8 , -8 ,  /*rand()%2*/ 2,
                  -8 , -10 ,  /*rand()%2*/ 4,
                  -8 , -12 ,  /*rand()%2*/ 2,
                  -6 , -12 ,  /*rand()%2*/ 2,
                  -10 ,  -10 ,  /*rand()%2*/ 2,
                  -10 ,  -12 ,  /*rand()%2*/ 2,
                  -10 ,  -8 ,  /*rand()%2*/ 2,
                  // -10,-6 , 2,
                  // -10,2,2,

                };



vector<int> data1 =
                {
                  0,0,2,
                  -2,0,1,
                  -4,0,1,
                  -6,0,2,
                  -8,0,1,
                  -10,0,1,
                  -12,0,2,
                  -12,-2,1,
                  -12,-4,1,
                  -12,-6,2,
                  -12,-8,1,
                  -12,-10,2,
                  // -12,-12,2,

                  2,0,1,
                  4,0,1,
                  6,0,2,
                  8,0,1,
                  10,0,1,
                  12,0,2,
                  12,-2,1,
                  12,-4,1,
                  12,-6,2,
                  12,-8,1,
                  12,-10,2,
                  // 12,-12,2,                  

                  12,2,2,
                  12,4,2,
                  12,6,2,
                  12,8,2,
                  12,10,2,
                  // 12,12,2,  

                  // -12,2,2,
                  // -12,4,2,
                  // -12,6,2,
                  0,8,2,
                  0,10,2,
                  0,12,4,                  

                };

positions.clear();
positions1.clear();
switches1.clear();
switches.clear();

  pos position;
  position.y = 0;

  for (int i = 0; i < data.size(); i+=3)
  {
    position.x = data[i];
    position.y += 0.4;
    position.z = data[i+1];
    position.color = data[i+2];
    positions.push_back(position);  
  }

  position.y = 0;
  for (int i = 0; i < data1.size(); i+=3)
  {
    position.x = data1[i];
    position.y += 0.4;
    position.z = data1[i+1];
    position.color = data1[i+2];
    positions1.push_back(position);  
  }

  Switch myswitch;

  vector <int> myswitches1 =
      {
        -12,-12,3,0,2,2,
        12,-12,3,0,4,2,
        12, 12,3,0,6,2,
      };

vector <int> myswitches =
                    {
                      -10,0,3,-6,-6,1,
                      12,-10,3,6,-10,2,
                      -10,2,3,-10,-6,                      
                    };

  myswitch.y = 0;
  myswitch.by = 0;
  myswitch.flag = 0;
  myswitch.ischanged = 0;
  for (int i = 0; i < myswitches.size(); i+=6)
  {
    myswitch.x = myswitches[i];
    myswitch.z = myswitches[i+1];
    myswitch.y += 0.4;
    myswitch.color = myswitches[i+2];
    myswitch.bx = myswitches[i+3];
    myswitch.bz = myswitches[i+4];
    switches.push_back(myswitch);  
  }

  myswitch.y = 0;
  myswitch.by = 0;
  for (int i = 0; i < myswitches1.size(); i+=6)
  {
    myswitch.x = myswitches1[i];
    myswitch.z = myswitches1[i+1];
    myswitch.y += 0.4;
    myswitch.color = myswitches1[i+2];
    myswitch.bx = myswitches1[i+3];
    myswitch.bz = myswitches1[i+4];
    switches1.push_back(myswitch);  
  }


    gameover = 0;
    overtime = 0;


    camposxf = 0 ;
    copiedcam = 0;
    camposx = 0;
    camhelicopied = 0;
    scaledown = 1;
    onsomething = 1;
    cube_yh = 1.25;


}


void draw1 (GLFWwindow* window)
{

  int drawn = 0;
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);



  glDisable (GL_DEPTH_TEST);


    // Render with texture shaders now
  glUseProgram(textureProgramID);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  VP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 500.0f)  * glm::lookAt( glm::vec3( 0, 2, 0 ), glm::vec3( 15, 0, 0 ), glm::vec3( 0, 1, 0 ) );

  glm::mat4 translateRectangle = translateMatrix(15,-10,0) * rotateMatrix(-90,1,0,0);        // glTranslatef
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;

  // Copy MVP to texture shaders
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

  // Set the texture sampler to access Texture0 memory
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DTexturedObject(rectangle);

  glEnable (GL_DEPTH_TEST);

  if(!gamestart)
    camorient = 6;

  if(camorient == 1) //Follow Cam View - ALT to change views
  {
    if(followmode == 1)
    {
          eyes1 = cube_xnow;
          eyes2 = 10;
          eyes3 =  cube_znow;
          t1 = cube_xnow;
          t2 =  0;
          t3 =  cube_znow;
          u1 = 1;
          u2 =  0;
          u3 =  0;   
    }
    else if(followmode == 2)
    {
          eyes1 = cube_xnow - 1;
          eyes2 = 6;
          eyes3 =  cube_znow;
          t1 = cube_xnow + 1;
          t2 =  0;
          t3 =  cube_znow;
          u1 = 1;
          u2 =  0;
          u3 =  0;
    }
    else if(followmode == 3)
    {
          eyes1 = cube_xnow + 1;
          eyes2 = 6;
          eyes3 =  cube_znow;
          t1 = cube_xnow - 1;
          t2 =  0;
          t3 =  cube_znow;
          u1 = -1;
          u2 =  0;
          u3 =  0;
    }
  }
  else if(camorient == 2) // Top View
  {
    eyes1 = 0;
    eyes2 = 15;
    eyes3 =  0;
    t1 = 0;
    t2 =  0;
    t3 =  0;
    u1 = 1;
    u2 =  0;
    u3 =  0;
  }
  else if(camorient == 3) // Block View
  {
    eyes1 = cube_xnow + 1;
    // eyes2 = 5;
    eyes2 = cube_ynow + 3;    
    eyes3 =  cube_znow;
    t1 = cube_xnow + 3;
    t2 =  0;
    t3 =  cube_znow;      
    u1 = 1;
    u2 =  0;
    u3 =  0; 
    if(moving_left)
    {
        t1 = cube_xnow;
        t2 =  0;
        t3 =  cube_znow - 3;      
        u1 = 0;
        u2 =  0;
        u3 =  -1;   
        premove = 1;   
    }
    else if(moving_right)
    {
        t1 = cube_xnow;
        t2 =  0;
        t3 =  cube_znow + 3;      
        u1 = 0;
        u2 =  0;
        u3 =  1;  
        premove = 2;   
    }
    else if(moving_up)
    {
        t1 = cube_xnow + 3;
        t2 =  0;
        t3 =  cube_znow;      
        u1 = 1;
        u2 =  0;
        u3 =  0; 
        premove = 3;                    
    }
    else if(moving_down)
    {
        t1 = cube_xnow - 3;
        t2 =  0;
        t3 =  cube_znow;      
        u1 = -1;
        u2 =  0;
        u3 =  0; 
        premove = 4;                                     
    }
    else
    {
      switch(premove)
      {
        case 1:
                eyes1 = cube_xnow ; eyes2 = cube_ynow + 2; eyes3 = cube_znow - 2 ;        
                u1 = 0 ; u2 = 0 , u3 = -1;
                t1 = cube_xnow ; t2 = 0 , t3 = cube_znow - 3;
                break;
        case 2:
                eyes1 = cube_xnow ; eyes2 = cube_ynow + 2; eyes3 = cube_znow + 2 ;        
                u1 = 0; u2 = 0 , u3 = 1;
                t1 = cube_xnow ; t2 = 0 , t3 = cube_znow + 3;
                break;
        case 3:
                eyes1 = cube_xnow + 2 ; eyes2 = cube_ynow + 2; eyes3 = cube_znow ;        
                u1 = 1 ; u2 = 0 , u3 = 0;
                t1 = cube_xnow + 3 ; t2 = 0 , t3 = cube_znow;
                break;
        case 4:
                eyes1 = cube_xnow - 2 ; eyes2 = cube_ynow + 2; eyes3 = cube_znow ;        
                u1 = -1 ; u2 = 0 , u3 = 0;
                t1 = cube_xnow - 3 ; t2 = 0 , t3 = cube_znow;
                break;                                                

      }
    }
  }
  else if(camorient == 4) // Tower View
  {
    eyes1 = -3;
    eyes2 = 12;
    eyes3 =  5;
    t1 =  0;
    t2 =  0;
    t3 =  0;
    u1 = 0;
    u2 =  1;
    u3 =  0;   
  }
  else if(camorient == 5) // Helicopter Cam View
  {
    eyes1 = helicamx;
    eyes2 = helicamy;
    eyes3 =  helicamz;
    t1 =  heli_x;
    t2 =  heli_y;
    t3 =  heli_z;
    u1 = 1;
    u2 =  0;
    u3 =  0;
    if(lbutton_down)
    {
        getMousePosition(window);        

      if(camhelicopied == 0)
      {
        camhelicopied = 1;
        onmousex = mousex;
        onmousey = mousey;
      }
      else
      {
        heli_z = heli_z - (mousex - onmousex)/10;
        heli_x = heli_x - (mousey - onmousey)/10;
      }
    }
    else
    {
      camhelicopied = 0;
    }
      if(heli_x < 0)
        u1 = -1;
      else
        u1 = 1;
  }
  else if(camorient == 6)
  {
    eyes1 = -5*cos(camera_rotation_angle*M_PI/180.0f);
    eyes2 = 7;
    eyes3 =  5*sin(camera_rotation_angle*M_PI/180.0f);
    t1 =  0;
    t2 =  0;
    t3 =  0;
    u1 = 0;
    u2 =  1;
    u3 =  0; 
    camera_rotation_angle += 0.5;
  }

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( eyes1, eyes2, eyes3 );

  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (t1, t2, t3);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (u1, u2, u3);   

  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  // Matrices.view = glm::lookAt(glm::vec3(-1,11,0), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  if(!alldown && gamestart)
  {
      alldown = 1;
      for (int i = 0; i < positions1.size(); ++i)
      {
          if(positions1[i].y > 0)
          {
            positions1[i].y -= 0.2;
            alldown = 0;
          }
      }

      for (int i = 0; i < switches1.size(); ++i)
      {
          if(switches1[i].y > 0)
          {
            switches1[i].y -= 0.2;
            alldown = 0;
          }
      }

  }

  if(alldown && downflag !=1)
  {
    gamestarttime = glfwGetTime();
    downflag = 1;
  }

  if(onsomething == 0)
  {

      // cout << cube_yh << endl;
      if(cube->state == 1 && cube_y >= -6)
      {
            cube_y -= 0.5;
            cube_ynow -= 0.5;
      }
      if( (cube->state == 2 || cube->state == 3 ) && cube_yh >= -6)
      {
            cube_yh -= 0.5;
            cube_ynow -= 0.5;
      }
  }

  // if(cube->state == 1)
    onsomething = 0;

    for (int i = 0; i < switches1.size(); ++i)
    {
      if(!gamestart)
        tile->drawShape(translateMatrix(switches1[i].x,0, switches1[i].z), switches1[i].color);  
      else
        tile->drawShape(translateMatrix(switches1[i].x,switches1[i].y, switches1[i].z), switches1[i].color);  

        if(switches1[i].color == 3 && cube->state == 1 && abs(switches1[i].x - cube_x) <= 0.1 && abs(switches1[i].z - cube_z) <= 0.1 )
        {
          if(switches1[i].flag == 0) 
          {
            if(switches1[i].ischanged == 0)
            {
              switches1[i].ischanged = 1;
              switches1[i].flag = 1;
              switches1[i].number += 0.1;
            }
          }
          else
          {

            if(switches1[i].ischanged == 0)
            {
              switches1[i].ischanged = 1;
              switches1[i].flag = 0;
            } 
          }

        }
        else
        {
          switches1[i].ischanged = 0; 
        }
      if(switches1[i].flag == 1 || switches1[i].number == 0.1)
      {
        tile->drawShape(translateMatrix(switches1[i].bx,0, switches1[i].bz), 5);  
      }

      if(cube->state == 1 && abs(switches1[i].x - cube_x) <= 0.1 && abs(switches1[i].z - cube_z) <= 0.1 )
        onsomething = 1;
      else if(cube->state == 2 && ((abs(switches1[i].x - cube_x) <= 0.1 && abs(switches1[i].z + 1 - cube_z) <= 0.1) || ((abs(switches1[i].x - cube_x) <= 0.1 && abs(switches1[i].z - 1 - cube_z) <= 0.1)))  )
        onsomething = 1;
      else if(cube->state == 3 && ((abs(switches1[i].x + 1 - cube_x) <= 0.1 && abs(switches1[i].z - cube_z) <= 0.1) || ((abs(switches1[i].x - 1 - cube_x) <= 0.1 && abs(switches1[i].z - cube_z) <= 0.1)))  )
        onsomething = 1;

      if(switches1[i].flag == 1)
      {
        if(cube->state == 1 && abs(switches1[i].bx - cube_x) <= 0.1 && abs(switches1[i].bz - cube_z) <= 0.1 )
          onsomething = 1;
        else if(cube->state == 2 && ((abs(switches1[i].bx - cube_x) <= 0.1 && abs(switches1[i].bz + 1 - cube_z) <= 0.1) || ((abs(switches1[i].bx - cube_x) <= 0.1 && abs(switches1[i].bz - 1 - cube_z) <= 0.1)))  )
          onsomething = 1;
        else if(cube->state == 3 && ((abs(switches1[i].bx + 1 - cube_x) <= 0.1 && abs(switches1[i].bz - cube_z) <= 0.1) || ((abs(switches1[i].bx - 1 - cube_x) <= 0.1 && abs(switches1[i].bz - cube_z) <= 0.1)))  )
          onsomething = 1;
      }

    }

  for (int i = 0; i < positions1.size(); ++i)
  {
    if(positions1[i].color == 1 && cube->state == 1 && abs(positions1[i].x - cube_x) <= 0.1 && abs(positions1[i].z - cube_z) <= 0.1 )
    {
        if(positions1[i].y >= -6)
         {
            positions1[i].y -= 0.5;
            cube_y -= 0.5;
            cube_ynow -= 0.5;
         }
        else
        {
              if(!gameover)
              {
                overtime = (int)glfwGetTime();
                // cout << overtime << endl;
                gameover = 1;
              }
              else
              {
                current_time = (int)glfwGetTime();
                drawendbanner(-2.5,1,0);
                // cout << current_time - overtime << endl;
                if((current_time - overtime) >= 3)
                  {
                    reset();
                  }
              }          
        }
    }
    if(positions1[i].color == 4 && cube->state == 1 && abs(positions1[i].x - cube_x) <= 0.1 && abs(positions1[i].z - cube_z) <= 0.1 )
    {
        if(cube_y >= 0)
         {
            drawn = 1;
            // positions[i].y -= 0.5;
            if(scaledown > 0)
            {
              scaledown -= 0.1;
              // cout << scaledown << endl;
            }
            if(scaledown >= 0)
              cube->drawShape(translateMatrix(cube_x,0.26 + 2*scaledown,cube_z) * scaleMatrix(1,scaledown,1),1);

            if(abs(scaledown) <= 0.01)
            {
              if(!gameover)
              {
                overtime = (int)glfwGetTime();
                // cout << overtime << endl;
                gameover = 1;
              }
              else
              {
                current_time = (int)glfwGetTime();
                // cout << current_time - overtime << endl;
                drawwinbanner(-2.5,1,0);                
                  if(!warpplay)
                    {
                      int playing = system ("mpg123 warp.mp3 &"); 
                      warpplay = 1;
                    }                
                if((current_time - overtime) >= 3)
                {
                  gameover = 0;                  
                  reset();
                }
              }
            }
          } 
    }
    if(cube->state == 1 && abs(positions1[i].x - cube_x) <= 0.1 && abs(positions1[i].z - cube_z) <= 0.1 )
      onsomething = 1;
    else if(cube->state == 2 && ((abs(positions1[i].x - cube_x) <= 0.1 && abs(positions1[i].z + 1 - cube_z) <= 0.1) || ((abs(positions1[i].x - cube_x) <= 0.1 && abs(positions1[i].z - 1 - cube_z) <= 0.1)))  )
      onsomething = 1;
    else if(cube->state == 3 && ((abs(positions1[i].x + 1 - cube_x) <= 0.1 && abs(positions1[i].z - cube_z) <= 0.1) || ((abs(positions1[i].x - 1 - cube_x) <= 0.1 && abs(positions1[i].z - cube_z) <= 0.1)))  )
      onsomething = 1;
    if(!gamestart)
      tile->drawShape(translateMatrix(positions1[i].x,0, positions1[i].z), positions1[i].color);
    else
      tile->drawShape(translateMatrix(positions1[i].x,positions1[i].y, positions1[i].z), positions1[i].color);
  }

if(!gamestart)
  moveByV();

if(drawn != 1 && gamestart && alldown)
{
    if(cube->state == 1)
      moveByV();
    else if(cube->state == 2)
      moveByH();
    else
      moveByHA();
}

  if(lbutton_down)
  {
    getMousePosition(window);
    // cout << mousex << " " << mousey << endl;


    if(mousex >= 7.5 * width/800 && mousex <= 8 * width/800 && mousey >= 7 * height/800 && mousey <= 7.5 * height/800 && gamestart)
    {
      if(moving_right == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");        
        steps++;
        moving_right = 1;
      }
    }
    else if(mousex >= 6.5 * width/800 && mousex <= 7 * width/800 && mousey >= 7 * height/800 && mousey <= 7.5 * height/800 && gamestart)
    {
      if(moving_left == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");        
        steps++;
        moving_left = 1;
      }
    }
    else if(mousex >= 7 * width/800 && mousex <= 7.5 * width/800 && mousey >= 6.5 * height/800 && mousey <= 7 * height/800 && gamestart)
    {
      if(moving_up == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");        
        steps++;
        moving_up = 1;
      }
    }
    else if(mousex >= 7 * width/800 && mousex <= 7.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
    {
      if(moving_down == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");            
        steps++;
        moving_down = 1; 
      }
    }



    else if(mousex >= 0 * width/800 && mousex <= 0.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 1)?0:1;
    else if(mousex >= 0.5 * width/800 && mousex <= 1 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 2)?0:2;
    else if(mousex >= 1 * width/800 && mousex <= 1.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 3)?0:3;
    else if(mousex >= 1.5 * width/800 && mousex <= 2 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 4)?0:4;
    else if(mousex >= 2 * width/800 && mousex <= 2.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 5)?0:5;
  }
    

    if(!gamestart)
    {
      countopen++;
      drawstartbanner(1,-2.5,1,0);
    }

    if(gamestart && alldown)
    {
      timespent = (int)(glfwGetTime() - gamestarttime);
      drawcharacter(2,std::to_string(timespent),12,1,-13);
      drawcharacter(1,std::to_string(steps),12,1,11);
      
      // Draw HUD
      tile->drawShape(translateMatrix(-12,0,14) * rotateMatrix(0,0,1,0), 6);
      tile->drawShape(translateMatrix(-10,0,12) * rotateMatrix(90,0,1,0), 6);
      tile->drawShape(translateMatrix(-12,0,10) * rotateMatrix(180,0,1,0), 6);
      tile->drawShape(translateMatrix(-14,0,12) * rotateMatrix(-90,0,1,0), 6);
      tile->drawShape(translateMatrix(-12,0,12) * rotateMatrix(-90,0,1,0), 7);

      tile->drawShape(translateMatrix(-14,0,-14) * rotateMatrix(90,0,1,0), 8);
      tile->drawShape(translateMatrix(-14,0,-12) * rotateMatrix(90,0,1,0), 9);
      tile->drawShape(translateMatrix(-14,0,-10) * rotateMatrix(90,0,1,0), 10);
      tile->drawShape(translateMatrix(-14,0,-8) * rotateMatrix(90,0,1,0), 11);
      tile->drawShape(translateMatrix(-14,0,-6) * rotateMatrix(90,0,1,0), 12);
    }

      if((((cube->state == 2 || cube->state == 3) && cube_yh <= -6) || ( cube->state == 1 && cube_y <= -6)))
      {
        dropped++;
        drawendbanner(-2.5,1,0);        
        if(dropped == 200)
            reset();         
      } 
}

void draw (GLFWwindow* window)
{



  int drawn = 0;
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);



  glDisable (GL_DEPTH_TEST);


    // Render with texture shaders now
  glUseProgram(textureProgramID);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  VP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 500.0f)  * glm::lookAt( glm::vec3( 0, 2, 0 ), glm::vec3( 15, 0, 0 ), glm::vec3( 0, 1, 0 ) );

  glm::mat4 translateRectangle = translateMatrix(15,-10,0) * rotateMatrix(-90,1,0,0);        // glTranslatef
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;

  // Copy MVP to texture shaders
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

  // Set the texture sampler to access Texture0 memory
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DTexturedObject(rectangle);

  glEnable (GL_DEPTH_TEST);

  float eyes1 = cam_x, eyes2 = cam_y , eyes3 = cam_z,t1 = 0 ,t2 = 0,t3 = 0,u1 = 0,u2 = 1,u3 = 0;

  if(!gamestart)
    camorient = 6;

  if(camorient == 1) //Follow Cam View - ALT to change views
  {
    if(followmode == 1)
    {
          eyes1 = cube_xnow;
          eyes2 = 10;
          eyes3 =  cube_znow;
          t1 = cube_xnow;
          t2 =  0;
          t3 =  cube_znow;
          u1 = 1;
          u2 =  0;
          u3 =  0;   
    }
    else if(followmode == 2)
    {
          eyes1 = cube_xnow - 1;
          eyes2 = 6;
          eyes3 =  cube_znow;
          t1 = cube_xnow + 1;
          t2 =  0;
          t3 =  cube_znow;
          u1 = 1;
          u2 =  0;
          u3 =  0;
    }
    else if(followmode == 3)
    {
          eyes1 = cube_xnow + 1;
          eyes2 = 6;
          eyes3 =  cube_znow;
          t1 = cube_xnow - 1;
          t2 =  0;
          t3 =  cube_znow;
          u1 = -1;
          u2 =  0;
          u3 =  0;
    }
  }
  else if(camorient == 2) // Top View
  {
    eyes1 = 0;
    eyes2 = 15;
    eyes3 =  0;
    t1 = 0;
    t2 =  0;
    t3 =  0;
    u1 = 1;
    u2 =  0;
    u3 =  0;
  }
  else if(camorient == 3) // Block View
  {
    if(premove == 0)
    {
    t1 = cube_xnow + 3;
    t2 =  0;
    t3 =  cube_znow;      
    u1 = 1;
    u2 =  0;
    u3 =  0;
    } 
    eyes1 = cube_xnow + 1;
    eyes2 = cube_ynow + 3;    
    eyes3 =  cube_znow;    
    if(moving_left)
    {
        t1 = cube_xnow;
        t2 =  0;
        t3 =  cube_znow - 3;      
        u1 = 0;
        u2 =  0;
        u3 =  -1;   
        premove = 1;   
    }
    else if(moving_right)
    {
        t1 = cube_xnow;
        t2 =  0;
        t3 =  cube_znow + 3;      
        u1 = 0;
        u2 =  0;
        u3 =  1;  
        premove = 2;   
    }
    else if(moving_up)
    {
        t1 = cube_xnow + 3;
        t2 =  0;
        t3 =  cube_znow;      
        u1 = 1;
        u2 =  0;
        u3 =  0; 
        premove = 3;                    
    }
    else if(moving_down)
    {
        t1 = cube_xnow - 3;
        t2 =  0;
        t3 =  cube_znow;      
        u1 = -1;
        u2 =  0;
        u3 =  0; 
        premove = 4;                                     
    }
    else
    {
      switch(premove)
      {
        case 1:
                eyes1 = cube_xnow ; eyes2 = cube_ynow + 2; eyes3 = cube_znow - 2 ;        
                u1 = 0 ; u2 = 0 , u3 = -1;
                t1 = cube_xnow ; t2 = 0 , t3 = cube_znow - 3;
                break;
        case 2:
                eyes1 = cube_xnow ; eyes2 = cube_ynow + 2; eyes3 = cube_znow + 2 ;        
                u1 = 0; u2 = 0 , u3 = 1;
                t1 = cube_xnow ; t2 = 0 , t3 = cube_znow + 3;
                break;
        case 3:
                eyes1 = cube_xnow + 2 ; eyes2 = cube_ynow + 2; eyes3 = cube_znow ;        
                u1 = 1 ; u2 = 0 , u3 = 0;
                t1 = cube_xnow + 3 ; t2 = 0 , t3 = cube_znow;
                break;
        case 4:
                eyes1 = cube_xnow - 2 ; eyes2 = cube_ynow + 2; eyes3 = cube_znow ;        
                u1 = -1 ; u2 = 0 , u3 = 0;
                t1 = cube_xnow - 3 ; t2 = 0 , t3 = cube_znow;
                break;                                                

      }
    }
    // t1 = pt1;
    // t2 =  pt2;
    // t3 =  pt3;
    // u1 = pu1;
    // u2 =  pu2;
    // u3 =  pu3;
  }
  else if(camorient == 4) // Tower View
  {
    eyes1 = -3;
    eyes2 = 12;
    eyes3 =  5;
    t1 =  0;
    t2 =  0;
    t3 =  0;
    u1 = 0;
    u2 =  1;
    u3 =  0;   
  }
  else if(camorient == 5) // Helicopter Cam View
  {
    eyes1 = helicamx;
    eyes2 = helicamy;
    eyes3 =  helicamz;
    t1 =  heli_x;
    t2 =  heli_y;
    t3 =  heli_z;
    u1 = 1;
    u2 =  0;
    u3 =  0;
    if(lbutton_down)
    {
        getMousePosition(window);        

      if(camhelicopied == 0)
      {
        camhelicopied = 1;
        onmousex = mousex;
        onmousey = mousey;
      }
      else
      {
        heli_z = heli_z - (mousex - onmousex)/10;
        heli_x = heli_x - (mousey - onmousey)/10;
      }
    }
    else
    {
      camhelicopied = 0;
    }
      if(heli_x < 0)
        u1 = -1;
      else
        u1 = 1;
  }
  else if(camorient == 6)
  {
    eyes1 = -5*cos(camera_rotation_angle*M_PI/180.0f);
    eyes2 = 7;
    eyes3 =  5*sin(camera_rotation_angle*M_PI/180.0f);
    t1 =  0;
    t2 =  0;
    t3 =  0;
    u1 = 0;
    u2 =  1;
    u3 =  0; 
    camera_rotation_angle += 0.5;
  }

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( eyes1, eyes2, eyes3 );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (t1, t2, t3);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (u1, u2, u3);   

  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  // Matrices.view = glm::lookAt(glm::vec3(-1,11,0), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  if(!alldown && gamestart)
  {
      alldown = 1;
      for (int i = 0; i < positions.size(); ++i)
      {
          if(positions[i].y > 0)
          {
            positions[i].y -= 0.2;
            alldown = 0;
          }
      }

      for (int i = 0; i < switches.size(); ++i)
      {
          if(switches[i].y > 0)
          {
            switches[i].y -= 0.2;
            alldown = 0;
          }
      }

  }

  if(alldown && downflag !=1)
  {
    gamestarttime = glfwGetTime();
    downflag = 1;
  }

  if(onsomething == 0)
  {
      if(cube->state == 1 && cube_y >= -6)
      {
            cube_y -= 0.5;
            cube_ynow -= 0.5;
      }
      if( (cube->state == 2 || cube->state == 3 ) && cube_yh >= -6)
      {
            cube_yh -= 0.5;
            cube_ynow -= 0.5;
      }          
  }

  
  // if(cube->state == 1)
    onsomething = 0;

    int curpos = switches.size() - 1;
    if(!gamestart)
    {
        tile->drawShape(translateMatrix(switches[curpos].x,0, switches[curpos].z), -1);  
    }
      else
      {
        tile->drawShape(translateMatrix(switches[curpos].x,switches[curpos].y, switches[curpos].z), -1);  
        tile->drawShape(translateMatrix(switches[curpos].bx,switches[curpos].y, switches[curpos].bz), 0);  
      }
        if(switches[curpos].color == 3 && cube->state == 1 && abs(switches[curpos].x - cube_x) <= 0.1 && abs(switches[curpos].z - cube_z) <= 0.1 )
        {
          cube_x = switches[curpos].bx;
          cube_z = switches[curpos].bz;
          onsomething = 1;
        }

      else if(cube->state == 2 && ((abs(switches[curpos].x - cube_x) <= 0.1 && abs(switches[curpos].z + 1 - cube_z) <= 0.1) || ((abs(switches[curpos].x - cube_x) <= 0.1 && abs(switches[curpos].z - 1 - cube_z) <= 0.1)))  )
        onsomething = 1;
      else if(cube->state == 3 && ((abs(switches[curpos].x + 1 - cube_x) <= 0.1 && abs(switches[curpos].z - cube_z) <= 0.1) || ((abs(switches[curpos].x - 1 - cube_x) <= 0.1 && abs(switches[curpos].z - cube_z) <= 0.1)))  )
        onsomething = 1;

        if(cube->state == 1 && abs(switches[curpos].bx - cube_x) <= 0.1 && abs(switches[curpos].bz - cube_z) <= 0.1 )
          onsomething = 1;       

    for (int i = 0; i < switches.size() - 1; ++i)
    {
      if(!gamestart)
        tile->drawShape(translateMatrix(switches[i].x,0, switches[i].z), switches[i].color);  
      else
        tile->drawShape(translateMatrix(switches[i].x,switches[i].y, switches[i].z), switches[i].color);  

        if(switches[i].color == 3 && cube->state == 1 && abs(switches[i].x - cube_x) <= 0.1 && abs(switches[i].z - cube_z) <= 0.1 )
        {
          if(switches[i].flag == 0) 
          {
            if(switches[i].ischanged == 0)
            {
              switches[i].ischanged = 1;
              switches[i].flag = 1;
              switches[i].number += 0.1;
            }
          }
          else
          {

            if(switches[i].ischanged == 0)
            {
              switches[i].ischanged = 1;
              switches[i].flag = 0;
            } 
          }

        }
        else
        {
          switches[i].ischanged = 0; 
        }
      if(switches[i].flag == 1 || switches[i].number == 0.1)
      {
        tile->drawShape(translateMatrix(switches[i].bx,0, switches[i].bz), 5);  
      }

      if(cube->state == 1 && abs(switches[i].x - cube_x) <= 0.1 && abs(switches[i].z - cube_z) <= 0.1 )
        onsomething = 1;
      else if(cube->state == 2 && ((abs(switches[i].x - cube_x) <= 0.1 && abs(switches[i].z + 1 - cube_z) <= 0.1) || ((abs(switches[i].x - cube_x) <= 0.1 && abs(switches[i].z - 1 - cube_z) <= 0.1)))  )
        onsomething = 1;
      else if(cube->state == 3 && ((abs(switches[i].x + 1 - cube_x) <= 0.1 && abs(switches[i].z - cube_z) <= 0.1) || ((abs(switches[i].x - 1 - cube_x) <= 0.1 && abs(switches[i].z - cube_z) <= 0.1)))  )
        onsomething = 1;

      if(switches[i].flag == 1)
      {
        if(cube->state == 1 && abs(switches[i].bx - cube_x) <= 0.1 && abs(switches[i].bz - cube_z) <= 0.1 )
          onsomething = 1;
        else if(cube->state == 2 && ((abs(switches[i].bx - cube_x) <= 0.1 && abs(switches[i].bz + 1 - cube_z) <= 0.1) || ((abs(switches[i].bx - cube_x) <= 0.1 && abs(switches[i].bz - 1 - cube_z) <= 0.1)))  )
          onsomething = 1;
        else if(cube->state == 3 && ((abs(switches[i].bx + 1 - cube_x) <= 0.1 && abs(switches[i].bz - cube_z) <= 0.1) || ((abs(switches[i].bx - 1 - cube_x) <= 0.1 && abs(switches[i].bz - cube_z) <= 0.1)))  )
          onsomething = 1;
      }

    }

  for (int i = 0; i < positions.size(); ++i)
  {
    if(positions[i].color == 1 && cube->state == 1 && abs(positions[i].x - cube_x) <= 0.1 && abs(positions[i].z - cube_z) <= 0.1 )
    {
        if(positions[i].y >= -6)
         {
            positions[i].y -= 0.5;
            cube_y -= 0.5;
            cube_ynow -= 0.5;
         }
         else
          {
              if(!gameover)
              {
                overtime = (int)glfwGetTime();
                // cout << overtime << endl;
                gameover = 1;
              }
              else
              {
                current_time = (int)glfwGetTime();
                drawendbanner(-2.5,1,0);
                // cout << current_time - overtime << endl;
                if((current_time - overtime) >= 3)
                {
                    reset();                  
                }
              }            
          }
    }
    if(positions[i].color == 4 && cube->state == 1 && abs(positions[i].x - cube_x) <= 0.1 && abs(positions[i].z - cube_z) <= 0.1 )
    {
        if(cube_y >= 0)
         {
            drawn = 1;
            // positions[i].y -= 0.5;
            if(scaledown > 0)
            {
              scaledown -= 0.1;
              // cout << scaledown << endl;
            }
            if(scaledown >= 0)
              cube->drawShape(translateMatrix(cube_x,0.26 + 2*scaledown,cube_z) * scaleMatrix(1,scaledown,1),1);

            if(abs(scaledown) <= 0.01)
            {
              if(!gameover)
              {
                overtime = (int)glfwGetTime();
                // cout << overtime << endl;
                gameover = 1;

              }
              else
              {
                current_time = (int)glfwGetTime();
                // cout << current_time - overtime << endl;
                drawwinbanner(-2.5,1,0);                                
                  if(!warpplay)
                    {
                      int playing = system ("mpg123 warp.mp3 &"); 
                      warpplay = 1;
                    }                
                if((current_time - overtime) >= 3)
                {
                  gameover = 0;
                  reset();
                }
              }
            } 

         }
    }
    if(cube->state == 1 && abs(positions[i].x - cube_x) <= 0.1 && abs(positions[i].z - cube_z) <= 0.1 )
      onsomething = 1;
    else if(cube->state == 2 && ((abs(positions[i].x - cube_x) <= 0.1 && abs(positions[i].z + 1 - cube_z) <= 0.1) || ((abs(positions[i].x - cube_x) <= 0.1 && abs(positions[i].z - 1 - cube_z) <= 0.1)))  )
      onsomething = 1;
    else if(cube->state == 3 && ((abs(positions[i].x + 1 - cube_x) <= 0.1 && abs(positions[i].z - cube_z) <= 0.1) || ((abs(positions[i].x - 1 - cube_x) <= 0.1 && abs(positions[i].z - cube_z) <= 0.1)))  )
      onsomething = 1;
    if(!gamestart)
      tile->drawShape(translateMatrix(positions[i].x,0, positions[i].z), positions[i].color);
    else
      tile->drawShape(translateMatrix(positions[i].x,positions[i].y, positions[i].z), positions[i].color);
  }

if(!gamestart)
  moveByV();

if(drawn != 1 && gamestart && alldown)
{
    if(cube->state == 1)
      moveByV();
    else if(cube->state == 2)
      moveByH();
    else
      moveByHA();
}

  if(lbutton_down)
  {
    getMousePosition(window);
    // cout << mousex << " " << mousey << endl;


    if(mousex >= 7.5 * width/800 && mousex <= 8 * width/800 && mousey >= 7 * height/800 && mousey <= 7.5 * height/800 && gamestart)
    {
      if(moving_right == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");        
        steps++;
        moving_right = 1;
      }
    }
    else if(mousex >= 6.5 * width/800 && mousex <= 7 * width/800 && mousey >= 7 * height/800 && mousey <= 7.5 * height/800 && gamestart)
    {
      if(moving_left == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");        
        steps++;
        moving_left = 1;
      }
    }
    else if(mousex >= 7 * width/800 && mousex <= 7.5 * width/800 && mousey >= 6.5 * height/800 && mousey <= 7 * height/800 && gamestart)
    {
      if(moving_up == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");        
        steps++;
        moving_up = 1;
      }
    }
    else if(mousex >= 7 * width/800 && mousex <= 7.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
    {
      if(moving_down == 0)
      {  
        brickplay = 1;
        int playing = system ("mpg123 tap1.mp3 &");            
        steps++;
        moving_down = 1; 
      }
    }

    else if(mousex >= 0 * width/800 && mousex <= 0.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 1)?0:1;
    else if(mousex >= 0.5 * width/800 && mousex <= 1 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 2)?0:2;
    else if(mousex >= 1 * width/800 && mousex <= 1.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 3)?0:3;
    else if(mousex >= 1.5 * width/800 && mousex <= 2 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 4)?0:4;
    else if(mousex >= 2 * width/800 && mousex <= 2.5 * width/800 && mousey >= 7.5 * height/800 && mousey <= 8 * height/800 && gamestart)
        camorient = (camorient == 5)?0:5;
  }
    

    if(!gamestart)
    {
      countopen++;
      drawstartbanner(1,-2.5,1,0);
    }

    if(gamestart && alldown)
    {
      timespent = (int)(glfwGetTime() - gamestarttime);
      drawcharacter(2,std::to_string(timespent),12,1,-13);
      drawcharacter(1,std::to_string(steps),12,1,11);
      
      // Draw HUD
      tile->drawShape(translateMatrix(-12,0,14) * rotateMatrix(0,0,1,0), 6);
      tile->drawShape(translateMatrix(-10,0,12) * rotateMatrix(90,0,1,0), 6);
      tile->drawShape(translateMatrix(-12,0,10) * rotateMatrix(180,0,1,0), 6);
      tile->drawShape(translateMatrix(-14,0,12) * rotateMatrix(-90,0,1,0), 6);
      tile->drawShape(translateMatrix(-12,0,12) * rotateMatrix(-90,0,1,0), 7);

      tile->drawShape(translateMatrix(-14,0,-14) * rotateMatrix(90,0,1,0), 8);
      tile->drawShape(translateMatrix(-14,0,-12) * rotateMatrix(90,0,1,0), 9);
      tile->drawShape(translateMatrix(-14,0,-10) * rotateMatrix(90,0,1,0), 10);
      tile->drawShape(translateMatrix(-14,0,-8) * rotateMatrix(90,0,1,0), 11);
      tile->drawShape(translateMatrix(-14,0,-6) * rotateMatrix(90,0,1,0), 12);
    }

      if((cube_yh <= -6 || cube_y <= -6))
      {
        dropped++;
        drawendbanner(-2.5,1,0);        
        gameover = 1;
        if(dropped == 200)
            reset();              
      } 
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Bloxorz", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, scroll_callback);  // scrolling 
    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{

    // Load Textures
  // Enable Texture0 as current texture memory
  glActiveTexture(GL_TEXTURE0);
  // load an image file directly as a new OpenGL texture
  // GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
  GLuint textureID = createTexture("space.jpg");
  // check for an error during the load process
  if(textureID == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

 textureID1 = createTexture("steelabc.png");
  // check for an error during the load process
  if(textureID1 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;


 textureID2 = createTexture("container2.png");
  // check for an error during the load process
  if(textureID2 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

 textureID3 = createTexture("brick_texture1893.jpg");
  // check for an error during the load process
  if(textureID3 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

 textureID4 = createTexture("hole.jpg");
  // check for an error during the load process
  if(textureID4 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

 textureID5 = createTexture("switch.jpg");
  // check for an error during the load process
  if(textureID5 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

 textureID51 = createTexture("switch1.jpg");
  // check for an error during the load process
  if(textureID51 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;  

 textureID52 = createTexture("switch2.png");
  // check for an error during the load process
  if(textureID52 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;  


 textureID6 = createTexture("bridge.jpg");
  // check for an error during the load process
  if(textureID6 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;


 textureID7 = createTexture("up.png");
  // check for an error during the load process
  if(textureID7 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

   textureID8 = createTexture("loop.png");
  // check for an error during the load process
  if(textureID8 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  textureID9 = createTexture("topview.jpeg");
  // check for an error during the load process
  if(textureID9 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  textureID10 = createTexture("top.png");
  // check for an error during the load process
  if(textureID10 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  textureID11 = createTexture("blockart.png");
  // check for an error during the load process
  if(textureID11 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  textureID12 = createTexture("lighthouse.jpg");
  // check for an error during the load process
  if(textureID12 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  textureID13 = createTexture("heli.png");
  // check for an error during the load process
  if(textureID13 == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;  

  // Create and compile our GLSL program from the texture shaders
  textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


    /* Objects should be created before any other gl function and shaders */
  // Create the models
  
  createRectangle (textureID);

  tile->generateShapeData();
  cube->generateShapeData();

    dig1->generateShapeData(124, 252, 0,0.1);
    dig2->generateShapeData(0.863, 0.078, 0.235,0.1);

vector <int> data = {
                  0 ,  0 ,  2,
                  2 ,  0 ,  /*rand()%2*/ 1,
                  -2 , 0 ,  /*rand()%2*/ 2,
                  2 ,  -2 ,  /*rand()%2*/ 2,
                  2 ,  2 ,  /*rand()%2*/ 2,
                  -4 , 0 ,  /*rand()%2*/ 2,
                  -4 , -2 ,  /*rand()%2*/ 2,
                  -4 , 2 ,  /*rand()%2*/ 2,
                  -6 , 2 ,  /*rand()%2*/ 2,
                  -6 , -2 ,  /*rand()%2*/ 2,
                  -6 , 0 ,  /*rand()%2*/ 2,
                  4 ,  0 ,  /*rand()%2*/ 2,
                  6 ,  0 ,  /*rand()%2*/ 2,
                  8 ,  0 ,  /*rand()%2*/ 2,
                  10 , 0 ,  /*rand()%2*/ 2,
                  12 , 0 ,  /*rand()%2*/ 2,
                  14 , 0 ,  /*rand()%2*/ 1,
                  14 , -2 ,  /*rand()%2*/ 1,
                  14 , -4 ,  /*rand()%2*/ 1,
                  14 , -6 ,  /*rand()%2*/ 1,
                  14 , -8 ,  /*rand()%2*/ 1,
                  14 , -10 ,  /*rand()%2*/ 1,
                  16 , -2 ,  /*rand()%2*/ 1,
                  16 , -4 ,  /*rand()%2*/ 1,
                  16 , -6 ,  /*rand()%2*/ 1,
                  // 12 , -10 ,  /*rand()%2*/ 2,
                  10 , -10 ,  /*rand()%2*/ 2,
                  8 , -10 ,  /*rand()%2*/ 2,
                  // 6 , -10 ,  /*rand()%2*/ 2,
                  4 , -10 ,  /*rand()%2*/ 2,
                  2 , -10 ,  /*rand()%2*/ 2,
                  0 , -10 ,  /*rand()%2*/ 2,
                  -2 , -10 ,  /*rand()%2*/ 2,
                  0 , -12 ,  /*rand()%2*/ 2,
                  0 , -8 ,  /*rand()%2*/ 2,
                  -4 , -10 ,  /*rand()%2*/ 2,
                  -6 , -10 ,  /*rand()%2*/ 2,
                  -6 , -8 ,  /*rand()%2*/ 2,
                  -8 , -8 ,  /*rand()%2*/ 2,
                  -8 , -10 ,  /*rand()%2*/ 4,
                  -8 , -12 ,  /*rand()%2*/ 2,
                  -6 , -12 ,  /*rand()%2*/ 2,
                  -10 ,  -10 ,  /*rand()%2*/ 2,
                  -10 ,  -12 ,  /*rand()%2*/ 2,
                  -10 ,  -8 ,  /*rand()%2*/ 2,
                  // -10,-6 , 2,
                  -10,2,2,
                };



vector<int> data1 =
                {
                  0,0,2,
                  -2,0,1,
                  -4,0,1,
                  -6,0,2,
                  -8,0,1,
                  -10,0,1,
                  -12,0,2,
                  -12,-2,1,
                  -12,-4,1,
                  -12,-6,2,
                  -12,-8,1,
                  -12,-10,2,
                  // -12,-12,2,

                  2,0,1,
                  4,0,1,
                  6,0,2,
                  8,0,1,
                  10,0,1,
                  12,0,2,
                  12,-2,1,
                  12,-4,1,
                  12,-6,2,
                  12,-8,1,
                  12,-10,2,
                  // 12,-12,2,                  

                  12,2,2,
                  12,4,2,
                  12,6,2,
                  12,8,2,
                  12,10,2,
                  // 12,12,2,  

                  // -12,2,2,
                  // -12,4,2,
                  // -12,6,2,
                  0,8,2,
                  0,10,2,
                  0,12,4,                  

                };

  pos position;
  position.y = 0;

  for (int i = 0; i < data.size(); i+=3)
  {
    position.x = data[i];
    position.y += 0.4;
    position.z = data[i+1];
    position.color = data[i+2];
    positions.push_back(position);  
  }

  position.y = 0;
  for (int i = 0; i < data1.size(); i+=3)
  {
    position.x = data1[i];
    position.y += 0.4;
    position.z = data1[i+1];
    position.color = data1[i+2];
    positions1.push_back(position);  
  }

  Switch myswitch;

  vector <int> myswitches1 =
      {
        -12,-12,3,0,2,2,
        12,-12,3,0,4,2,
        12, 12,3,0,6,2,
      };

vector <int> myswitches =
                    {
                      -10,0,3,-6,-6,1,
                      12,-10,3,6,-10,2,
                    };

  myswitch.y = 0;
  myswitch.by = 0;
  for (int i = 0; i < myswitches.size(); i+=6)
  {
    myswitch.x = myswitches[i];
    myswitch.z = myswitches[i+1];
    myswitch.y += 0.4;
    myswitch.color = myswitches[i+2];
    myswitch.bx = myswitches[i+3];
    myswitch.bz = myswitches[i+4];
    switches.push_back(myswitch);  
  }

  myswitch.y = 0;
  myswitch.by = 0;
  for (int i = 0; i < myswitches1.size(); i+=6)
  {
    myswitch.x = myswitches1[i];
    myswitch.z = myswitches1[i+1];
    myswitch.y += 0.4;
    myswitch.color = myswitches1[i+2];
    myswitch.bx = myswitches1[i+3];
    myswitch.bz = myswitches1[i+4];
    switches1.push_back(myswitch);  
  }

  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

  
  reshapeWindow (window, width, height);

    // Background color of the scene
  glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void gameplay()
{
    cout << "Enter preferred width and height : ";
    cin >> width >> height;

    GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  int backplaying = 0;

    last_update_time = glfwGetTime();

    /* Draw in loop */
    reset();
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
      // cout << cube_yh << " " << cube_y << endl;
      if(level == 1)
        draw(window);
      else
        draw1(window);
      if(backplaying == 0)
      {
        backplaying = 1;
        int playing = system ("mpg123 back.mp3 &"); 
      }

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);  
}
int main (int argc, char** argv)
{

   gameplay();      

    return 0;
}