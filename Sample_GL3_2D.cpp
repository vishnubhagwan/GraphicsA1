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

using namespace glm;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

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
} Matrices;

GLuint programID;

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
  exit(EXIT_SUCCESS);
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

/**************************
 * Customizable functions *
 **************************/

 float cannon_rot_dir = 1;
 bool cannon_rot_status = false;
 bool flag = false;
 bool shot = false;
 float speed=.03;
 float delta_t = 0;
 float gravity = -.01;
 float camera_rotation_angle = 90;
 float cannon_rotation = 0;
 float ballx_initial = 0;
 float bally_initial = 1;
 float speedx_initial = 0;
 float speedy_initial = 0;
 float speedy_final = 0;
 float radius = 0.1;
 float yoffset = 1.06;
 float xoffset = 3.1;
 bool flg[3] = {false};
 int score = 0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
 void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
 {
     // Function is called first on GLFW_PRESS.

  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_C:
      cannon_rot_status = false;
      break;
      case GLFW_KEY_P:
      cannon_rot_status = false;
      break;
      case GLFW_KEY_F:
      speed += .01;
      break;
      case GLFW_KEY_S:
      speed -= .01;
      break;
      case GLFW_KEY_SPACE:
      shot = true;
      break;
      // case GLFW_KEY_Z:
      // zoom += .5;
      // break;
      // case GLFW_KEY_X:
      // zoom -= .5;
      // break;
      default:
      break;
    }
  }
  else if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_C:
      cannon_rot_status = true;
      cannon_rot_dir = 1;
      break;
      case GLFW_KEY_P:
      cannon_rot_status = true;
      cannon_rot_dir = -1;
      break;
      case GLFW_KEY_ESCAPE:
      quit(window);
      break;
      default:
      break;
    }
  }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
 switch (key) {
  case 'Q':
  case 'q':
  std::cout << "Your score is " << score << '\n';
  quit(window);
  break;
  default:
  break;
}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  // switch (button) {
  //   case GLFW_MOUSE_BUTTON_LEFT:
  //   if (action == GLFW_RELEASE)
  //     triangle_rot_dir *= -1;
  //   break;
  //   case GLFW_MOUSE_BUTTON_RIGHT:
  //   if (action == GLFW_RELEASE) {
  //     rectangle_rot_dir *= -1;
  //   }
  //   break;
  //   default:
  //   break;
  // }
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
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
     Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
     // Matrices.projection = glm::ortho(-4.0f * ((float)width/(float)height), 4.0f * ((float)width/(float)height), -4.0f, 4.0f, 0.1f, 500.0f);
   }

   VAO *triangle, *rectangle, *cannon, *pivot, *ball, *t_ball1, *t_ball2, *t_ball3, *t_rectangle, *t_triangle, *t_trep, *blueball;

// Creates the triangle object used in this sample code
 // triangle
   void createTriangle ()
   {
    static const GLfloat vertex_buffer_data [] = {
    2, -3,0, // vertex 0
    3,-3,0, // vertex 1
    2.5,-2,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    .2,.2,.2, // color 0
    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
  };

  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// pivot
void create_pivot ()
{
  static const GLfloat vertex_buffer_data [] = {
   0, -0.4, 0,
   -0.2, -.8, 0,
   0.2, -.8, 0
 };

 static const GLfloat color_buffer_data [] = {
  1.0, 0.0, 0.0,
  1.0, 0.0, 0.0,
  1.0, 0.0, 0.0
};

pivot = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// rectangle
void createRectangle()
{
  // GL3 accepts only Triangles. Quads are not supported static
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// cannon
void create_cannon()
{
  static const GLfloat vertex_buffer_data [] = {
    .1, 0.5,  0,
    -.1, 0.5,  0,
    -.2,  -0.5,  0,

    -.2,  -0.5,  0,
    .2,  -0.5,  0,
    .1, 0.5,  0,
  };

  static const GLfloat color_buffer_data [] = {
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,

    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
  };

  cannon = create3DObject(GL_TRIANGLES, 9, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void create_blueball()
{
  GLfloat  PI = 3.141592654;
  GLfloat angle = 0.0;
  int points = 100;
  static GLfloat vertex_buffer_data[300];
  static GLfloat color_buffer_data[300];
  /* code for circle*/
  for(int i = 0; i < 3*points;){
    angle = PI * i / points;
    vertex_buffer_data[i++] = .2*(float)cos(angle);
    vertex_buffer_data[i++] = .2*(float)sin(angle);
    vertex_buffer_data[i++] = 0;
  }
  /* code for circle*/
  for(int i = 0 ; i < 3*points ; ){
    color_buffer_data[i++] = 0;
    color_buffer_data[i++] = 0;
    color_buffer_data[i++] = 1;
  }
  blueball = create3DObject(GL_TRIANGLE_FAN, points, vertex_buffer_data, color_buffer_data, GL_FILL);

}

// ball
void create_ball()
{
  GLfloat  PI = 3.141592654;
  GLfloat angle = 0.0;
  int points = 100;
  static GLfloat vertex_buffer_data[300];
  static GLfloat color_buffer_data[300];
  /* code for circle*/
  for(int i = 0; i < 3*points;){
    angle = PI * i / points;
    vertex_buffer_data[i++] = radius*(float)cos(angle);
    vertex_buffer_data[i++] = radius*(float)sin(angle);
    vertex_buffer_data[i++] = 0;
  }
  /* code for circle*/
  for(int i = 0 ; i < 3*points ; ){
    color_buffer_data[i++] = 0;
    color_buffer_data[i++] = 1;
    color_buffer_data[i++] = 0;
  }
  ball = create3DObject(GL_TRIANGLE_FAN, points, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// target balls
void target_ball1()
{
  GLfloat  PI = 3.141592654;
  GLfloat angle = 0.0;
  int points = 100;
  static GLfloat vertex_buffer_data[300];
  static GLfloat color_buffer_data[300];
  /* code for circle*/
  for(int i = 0; i < 3*points;){
    angle = PI * i / points;
    vertex_buffer_data[i++] = .5*(float)cos(angle);
    vertex_buffer_data[i++] = .5*(float)sin(angle);
    vertex_buffer_data[i++] = 0;
  }
  /* code for circle*/
  for(int i = 0 ; i < 3*points ; ){
    color_buffer_data[i++] = .6;
    color_buffer_data[i++] = .56;
    color_buffer_data[i++] = .21;
  }
  t_ball1 = create3DObject(GL_TRIANGLE_FAN, points, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void target_ball2()
{
  GLfloat  PI = 3.141592654;
  GLfloat angle = 0.0;
  int points = 100;
  static GLfloat vertex_buffer_data[300];
  static GLfloat color_buffer_data[300];
  /* code for circle*/
  for(int i = 0; i < 3*points;){
    angle = PI * i / points;
    vertex_buffer_data[i++] = .7*(float)cos(angle);
    vertex_buffer_data[i++] = .7*(float)sin(angle);
    vertex_buffer_data[i++] = 0;
  }
  /* code for circle*/
  for(int i = 0 ; i < 3*points ; ){
    color_buffer_data[i++] = .78;
    color_buffer_data[i++] = .2323;
    color_buffer_data[i++] = .321;
  }
  t_ball2 = create3DObject(GL_TRIANGLE_FAN, points, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void target_ball3()
{
  GLfloat  PI = 3.141592654;
  GLfloat angle = 0.0;
  int points = 100;
  static GLfloat vertex_buffer_data[300];
  static GLfloat color_buffer_data[300];
  /* code for circle*/
  for(int i = 0; i < 3*points;){
    angle = PI * i / points;
    vertex_buffer_data[i++] = .2*(float)cos(angle);
    vertex_buffer_data[i++] = .2*(float)sin(angle);
    vertex_buffer_data[i++] = 0;
  }
  /* code for circle*/
  for(int i = 0 ; i < 3*points ; ){
    color_buffer_data[i++] = .6231;
    color_buffer_data[i++] = .42;
    color_buffer_data[i++] = .6767;
  }
  t_ball3 = create3DObject(GL_TRIANGLE_FAN, points, vertex_buffer_data, color_buffer_data, GL_FILL);

}
// target balls

// target rectangle
void target_rectangle()
{
  static const GLfloat vertex_buffer_data [] = {
    2,0,0,
    2.5,0,0, 
    2.5,3,0,

    2.5,3,0, 
    2,3,0,
    2,0,0 
  };

  static const GLfloat color_buffer_data [] = {
    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 3
    0.2,0.2,0.2, // color 4
    .2,.2,.2  // color 1
  };
  t_rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// target trapezium
void target_trepezium()
{
  static const GLfloat vertex_buffer_data [] = {
    3.7,2,0,
    4.7,2,0, 
    4.5,2.5,0,

    4.5,2.5,0, 
    4.1,2.5,0,
    3.7,2,0 
  };

  static const GLfloat color_buffer_data [] = {
    .4,.4,.4, // color 1
    .4,.4,.4, // color 2
    .4,.4,.4, // color 3

    .4,.4,0, // color 3
    0.4,0.4,0.4, // color 4
    .4,.4,.4  // color 1
  };
  t_trep = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// target triangle
void target_triangle()
{
  static const GLfloat vertex_buffer_data [] = {
   7, 0, 0,
   7.6, 0, 0,
   7.3, 1, 0
 };

 static const GLfloat color_buffer_data [] = {
  .5, .5, .5,
  .5, .5, .5,
  .5, .5, .5
};
t_triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  float a[][3] = {
    {4.3,3,0.5},{2.25,3.7,0.7},{7.3,1.2,0.2}
  };
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (programID);
  vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  vec3 target (0, 0, 0);
  vec3 up (0, 1, 1);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  mat4 VP = Matrices.projection * Matrices.view;
  mat4 MVP;
  mat4 translateAxes = translate(vec3(-4,-4,0));

  // target triangle
  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(t_triangle);

  // target rectangle
  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(t_rectangle);
  // draw3DTexturedObject(rectangle);

  // target trapezium
  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(t_trep);

  mat4 translateBall;

  // Target balls
  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  translateBall = translate(vec3(4.3,3,0));
  MVP *= translateBall;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(!flg[0])
    draw3DObject(t_ball1);

  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  translateBall = translate(vec3(2.25,3.7,0));
  MVP *= translateBall;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(!flg[1])
    draw3DObject(t_ball2);

  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  translateBall = translate(vec3(7.3,1.2,0));
  MVP *= translateBall;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(!flg[2])
    draw3DObject(t_ball3);
  // Target balls

  // cannon
  Matrices.model = mat4(1.0f);
  MVP = VP * translateAxes;
  mat4 translateCannon = translate (vec3(1, .3, 0));        // glTranslatef
  mat4 rotateCannon = rotate((float)(cannon_rotation*M_PI/180.0f), vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateCannon * rotateCannon);
  MVP *= Matrices.model;
  translateCannon = translate(vec3(0,.5,0));
  MVP *= translateCannon;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannon);

  // blueball
  translateCannon = translate(vec3(0,-.5,0));
  MVP *= translateCannon;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(blueball);

  // pivot
  Matrices.model = mat4(1.0f);
  MVP = VP * Matrices.model;
  MVP *= translateAxes;
  mat4 translatePivot = translate(vec3(1,.8,0));
  MVP *= translatePivot;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(pivot);

  // ball
  if(!shot)
  {
    Matrices.model = mat4(1.0f);
    MVP = VP * Matrices.model;
    MVP *= translateAxes;
    translateBall = translate (vec3(1, .3, 0));
    mat4 rotateBall = rotate((float)(cannon_rotation*M_PI/180.0f), vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateBall * rotateBall);
    MVP *= Matrices.model;
    translateBall = translate(vec3(0,.86,0));
    MVP *= translateBall;
    ballx_initial = -std::sin(cannon_rotation*M_PI/180.0f);
    bally_initial = std::cos(cannon_rotation*M_PI/180.0f);
    speedx_initial = -speed*std::sin(cannon_rotation*M_PI/180.f);
    speedy_initial = speed*std::cos(cannon_rotation*M_PI/180.f);
  }
  else
  {
    delta_t += .1;
    ballx_initial += speedx_initial * delta_t;
    bally_initial += speedy_initial * delta_t + .5 * gravity * delta_t * delta_t;
    speedy_final = speedy_initial + gravity * delta_t;
    for(int i = 0 ; i < 3 ; i++)
    {
      if(!flg[i])
      {
        float dx = a[i][0]-ballx_initial, dy = a[i][1]-bally_initial;
        float total_radius = a[i][2] + radius;
        if(dx*dx + dy*dy <= total_radius*total_radius)
        {
          flg[i] = true;
          score++;
        }
      }
    }

    // rebound x
    if(ballx_initial - xoffset > 4.0f || ballx_initial + 4.0f - xoffset < 0)
      speedx_initial = -1 * (speedx_initial - speedx_initial * (.05f));

    // rebound y
    if(bally_initial < -0.8f || bally_initial > 8.0f - 0.8f)
      speedy_initial = -1 * (speedy_final - speedy_final * (.05f));
    if(bally_initial < -2)
      return;
    // if(bally_initial + yoffset < 0 || bally_initial > 8.0 - yoffset)
    //   speedy_initial = -1 * (speedy_final - speedy_final * (.05f));

    translateBall = translate(vec3(ballx_initial, bally_initial, 0));
    MVP *= translateBall;
  }

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(ball);
  // Increment angles
  float increments;
  if(cannon_rotation > 90)
  {
    cannon_rotation = -90;
    increments = 0;
  }
  else if(cannon_rotation < -90)
  {
    cannon_rotation = 90;
    increments = 0;
  }
  else increments = 5;
  //camera_rotation_angle++; // Simulating camera rotation
  if(cannon_rot_status)
    cannon_rotation = cannon_rotation + increments*cannon_rot_dir*cannon_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
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

    return window;
  }

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
  void initGL (GLFWwindow* window, int width, int height)
  {
    /* Objects should be created before any other gl function and shaders */
	// Create the models

    create_cannon ();
    create_blueball ();
    create_pivot ();
    create_ball ();
    createTriangle ();
    target_rectangle ();
    target_triangle ();
    target_ball1 ();
    target_ball2 ();
    target_ball3 ();
    target_trepezium ();

	// Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.8f, 0.023f, 0.3f, 0.38431f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	std::cout << "VENDOR: " << glGetString(GL_VENDOR) << '\n';
	std::cout << "RENDERER: " << glGetString(GL_RENDERER) << '\n';
	std::cout << "VERSION: " << glGetString(GL_VERSION) << '\n';
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
}

int main (int argc, char** argv)
{
	int width = 1280;
	int height = 720;

  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
  while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
    draw();

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
      std::cout << score << '\n';
      exit(EXIT_SUCCESS);
    }
