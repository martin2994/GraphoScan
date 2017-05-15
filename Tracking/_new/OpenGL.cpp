//#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "Camera.hpp"
#include "Shader.hpp"
//#include "GraphoScan.cpp"


#include "OpenGL.hpp"

myOpenGL::myOpenGL(const GLuint width, const GLuint height)
{
  winWidth = WIDTH;
  winHeight = HEIGHT;
  //texture1.InitTexture("wall.jpg");
  //texture2.InitTexture("baboon.jpg", 1);
}

myOpenGL::~myOpenGL(){}

void myOpenGL::InitWindow(){
  //GLFW is used to deal with the context
  //initialize glfw(the window)
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  //create the window
  window = glfwCreateWindow(1600, 800, "GraphoScan", nullptr, nullptr);
  if (window == nullptr)
    {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return;
    }
  
  //bind the key call-back function key_callback with the window
  glfwSetKeyCallback(window, key_callback);
  
  //bind the mouse call-back function mouse_callback with the window
  glfwSetCursorPosCallback(window, mouse_callback);
  
  //bind the scroll call-back function scroll_callback with the window
  glfwSetScrollCallback(window, scroll_callback);
  
  //to inform the GLFW to create the window we defined
  glfwMakeContextCurrent(window);
  
  //this should be set GL_TURE for using the modern functions of opencv
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
    {
      std::cout << "Failed to initialize GLEW" << std::endl;
      return;
    }
  //call this function to define the window we look from
  glViewport(0, 0, 1600, 800);
}

void myOpenGL::InitVertex() {
  //VAO(Vertex Array Object), which is uesed to store different kinds of VBO and their corresponded vertex attribute
  glGenVertexArrays(1, &VAO1);
  glGenBuffers(1, &VBO1);
  //glGenBuffers(1, &EBO1);

  // Bind the Vertex Array Object first(VAO), then bind and set vertex buffer(s) and attribute pointer(s).
  glBindVertexArray(VAO1);

  //define and give the handle of VBO(Vertex Buffer Objects) 
  glBindBuffer(GL_ARRAY_BUFFER, VBO1);//bind VBO with the buffer:GL_ARRAY_BUFFER
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		
  /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

  //even if now we send the data to the GPU, it cannot understand the type of the data.  
  //the vertex shader accept any kind of the input, so we need to explain (Vertex Attribute) to OpenGL
  //param 1:indication of which vertex shader we are going to use(this time is the one whose postion is 0(layout(location = 0))
  //param 2:the size of the type(here it is vec3, so the size is 3)
  //param 3:the type of the data(GL_FLOAT here)
  //param 4:to indicate that if the data should be normalized between 0 and 1, now here it is GL_FALSE
  //param 5:the stride, which means the distance between two vertex Array. here we have x,y,z, totally 3 position.
  //param 6:offset, which position the program should start with.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
  //glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
  glEnableVertexAttribArray(2);

  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);//unbind the VAO

  //initialiser le VAO2, VBO2
  glGenVertexArrays(2, &VAO2);
  glGenBuffers(2, &VBO2);

  glBindVertexArray(VAO2);
  glBindBuffer(GL_ARRAY_BUFFER, VBO2);

  glBufferData(GL_ARRAY_BUFFER, sizeof(OfPlane), OfPlane, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  //setting
  glEnable(GL_DEPTH_TEST);

  //capture the cursor and never lose it
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

void myOpenGL::RunGL(const char* filename/*, const char* filename_bg*/) {
  vector<cv::Mat> v1;
  vector<cv::Mat> v2;
  //read the points from file
  ifstream fin;
  ifstream fin_bg;
  fin.open(filename);
  if (!fin.is_open())
    {
      cout << "Cannot open the file " << filename << endl;
      exit(1);
    }
  else
    {
      for (int i = 0; fin.good(); i++)
	{
	  float xyzw[4];
	  fin >> xyzw[0];
	  fin >> xyzw[1];
	  fin >> xyzw[2];
	  fin >> xyzw[3];

	  cv::Mat M = cv::Mat_<float>(4, 1, xyzw);
	  v1.push_back(M.clone());
	}
    }
  fin.clear();
  fin.close();

  /*fin.open(filename_bg);
    if (!fin.is_open())
    {
    cout << "Cannot open the file " << filename_bg << endl;
    exit(1);
    }
    else
    {
    for (int i = 0; fin.good(); i++)
    {
    float xyzw[4];
    fin >> xyzw[0];
    fin >> xyzw[1];
    fin >> xyzw[2];
    fin >> xyzw[3];

    cv::Mat M = cv::Mat_<float>(4, 1, xyzw);
    v2.push_back(M.clone());
    }
    }
    fin.clear();
    fin.close(); 
  */
  int frameCount = 0;
  int n = 0;

  //Texture2D texture1("wall.jpg");
  //Texture2D texture2("baboon.jpg", 1);
  Shader shader1("vShader.txt", "fShader.txt");
  Shader shader2("vShader.txt", "fShader.txt");
  while (!glfwWindowShouldClose(window))
    {
      //we want to handle the events before painting.
      glfwPollEvents();//call this function to check if there are the events, for exemple, the key, are they pressed?
			
      //press key
      do_movement();
			
      //clear the color in the buffer by color(RGB + alpha(transparent))
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			
      //indicate that we want to clear this buffer: GL_COLOR_BUFFER_BIT
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /*Texture1*/
      //texture1.UseTexture2D(shader1.Program, "ourTexture1");
      //glActiveTexture(GL_TEXTURE0);
      //glBindTexture(GL_TEXTURE_2D, texture1);
      //glUniform1i(glGetUniformLocation(shader1.Program, "ourTexture1"), 0);
      /*Texture2*/
      //texture2.UseTexture2D(shader1.Program, "ourTexture2");
      //glActiveTexture(GL_TEXTURE1);
      //glBindTexture(GL_TEXTURE_2D, texture2);
      //glUniform1i(glGetUniformLocation(shader1.Program, "ourTexture2"), 1);

      /************************/
      // ===================
      // Transformation
      // ===================
      //glm::vec3 trans;
      //trans = glm::translate(trans, glm::vec3((GLfloat)sin(glfwGetTime()) / 2, 0.25f, 0.0f));
      //trans = glm::rotate(trans, glm::radians((GLfloat)sin(glfwGetTime()) / 2 * 50.0f), glm::vec3(1.0f, 0.0f, 1.0f));
      //trans = glm::scale(trans, glm::vec3(0.2f, 0.2f, 0.2f));
      /************************/

      //run the shader1 program
      shader1.Use();
		
      glUniform1f(glGetUniformLocation(shader1.Program, "mixvalue"), mixValue);

      //GLfloat radius = 10.0f;
      //GLfloat camX = sin(glfwGetTime())*radius;
      //GLfloat camZ = cos(glfwGetTime())*radius;
			
      myView = myCamera.GetViewMatrix();
      //myView = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f,0.0f,0.0f), cameraUp);

      myProjection = myCamera.GetProjectionMatrix(HEIGHT, WIDTH);
      //myProjection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		
      //send view matrix 
      glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "view"), 1, GL_FALSE, glm::value_ptr(myView));
		
      //send projection matrix
      glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "projection"), 1, GL_FALSE, glm::value_ptr(myProjection));
		
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      //if shader program has benn well compiled, run it.
      //when we want to draw something, we should call VAO	
      glBindVertexArray(VAO1);

      /*for (int i = 0; i < v2.size(); i++)
	{
	GLfloat* ptr = v2[i].ptr<float>(0);
	GLfloat scale = 0.01f;
	GLfloat scale_1 = 0.1f;
	GLfloat inv_w = GLfloat(1.0f / ptr[3]);

	//scale *= inv_w;
	glm::vec3 v2 = glm::vec3((GLfloat)ptr[0] * scale_1 *inv_w, (GLfloat)ptr[1] * scale_1*inv_w, (GLfloat)ptr[2] * scale_1*inv_w);
	glm::vec3 v3 = glm::vec3((GLfloat)ptr[0], -(GLfloat)ptr[1], (GLfloat)ptr[2]);
	glm::vec3 v4 = glm::normalize(v2);

	glm::mat4 model;

	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(-1.0f, -1.0f, 0.0f));

	//1
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 1.0f, 0.0f));
	//model = glm::rotate(model, glm::radians((GLfloat)sin(glfwGetTime()) / 2 * 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, v2);
	//model = glm::translate(model, glm::vec3((GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime())));
	model = glm::scale(model, glm::vec3(scale, scale, scale));



	//GLfloat* ptr = v2[i].ptr<float>(0);
	//GLfloat scale = 0.01f;
	//GLfloat scale_1 = 0.1f;
	//GLfloat inv_w = GLfloat(1.0f / ptr[3]);

	////scale *= inv_w;
	//glm::vec3 v2 = glm::vec3(-(GLfloat)ptr[0] * scale_1 *inv_w, -(GLfloat)ptr[1] * scale_1*inv_w, (GLfloat)ptr[2] * scale_1*inv_w);
	//glm::vec3 v3 = glm::vec3((GLfloat)ptr[0], -(GLfloat)ptr[1], (GLfloat)ptr[2]);
	//glm::vec3 v4 = glm::normalize(v2);

	//glm::mat4 model;

	//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(-1.0f, -1.0f, 0.0f));
	//model = glm::translate(model, v2);
	//model = glm::translate(model, glm::vec3((GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime())));
	//model = glm::scale(model, glm::vec3(scale, scale, scale));

	//send model matrix
	glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//draw something...
	/******************************/
      //glDrawArrays(GL_TRIANGLES, 0, 36);
      //draw a triangle
      //param 1:the basic graph that we wanna draw
      //param 2:the index of the position of the vertex that we will start with in the vertex table
      //param 3:the number of the vertex that we want to draw
      /*****************************/

      //			}

      for (int i = 0; i < frameCount; i++)
	{
	  if (frameCount >= v1.size())
	    {
	      cout << "frameCount > v1.size." << endl;
	      exit(1);
	    }
	  GLfloat* ptr = v1[i].ptr<float>(0);
	  GLfloat scale = 0.01f;
	  GLfloat scale_1 = 0.1f;
	  GLfloat inv_w = GLfloat(1.0f/ptr[3]);

	  //scale *= inv_w;
	  glm::vec3 v2 = glm::vec3((GLfloat)ptr[0] * scale_1 *inv_w, (GLfloat)ptr[1] * scale_1*inv_w, (GLfloat)ptr[2] * scale_1*inv_w);
	  glm::vec3 v3 = glm::vec3((GLfloat)ptr[0], -(GLfloat)ptr[1], (GLfloat)ptr[2]);
	  glm::vec3 v4 = glm::normalize(v2);
				
	  glm::mat4 model;

	  //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(-1.0f, -1.0f, 0.0f));
					
	  //1
	  model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 1.0f, 0.0f));
	  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	  //model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	  model = glm::translate(model, v2);
	  //model = glm::translate(model, glm::vec3((GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime())));		
	  model = glm::scale(model, glm::vec3(scale, scale, scale));


	  //GLfloat* ptr = v1[i].ptr<float>(0);
	  //GLfloat scale = 0.01f;
	  //GLfloat scale_1 = 0.001f;
	  //GLfloat inv_w = GLfloat(1.0f / ptr[3]);

	  ////scale *= inv_w;
	  //glm::vec3 v2 = glm::vec3(-(GLfloat)ptr[0] * scale_1 *inv_w, -(GLfloat)ptr[1] * scale_1*inv_w, (GLfloat)ptr[2] * scale_1*inv_w);
	  //glm::vec3 v3 = glm::vec3((GLfloat)ptr[0], -(GLfloat)ptr[1], (GLfloat)ptr[2]);
	  //glm::vec3 v4 = glm::normalize(v2);

	  //glm::mat4 model;

	  //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(-1.0f, -1.0f, 0.0f));
	  //model = glm::translate(model, v2);
	  //model = glm::translate(model, glm::vec3((GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime()), (GLfloat)sin(glfwGetTime())));		
	  //model = glm::scale(model, glm::vec3(scale, scale, scale));
				
	  //send model matrix
	  glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	  //draw something...
	  /******************************/
	  glDrawArrays(GL_TRIANGLES, 0, 36);
	  //draw a triangle
	  //param 1:the basic graph that we wanna draw
	  //param 2:the index of the position of the vertex that we will start with in the vertex table
	  //param 3:the number of the vertex that we want to draw
	  /*****************************/

	}
      if ((frameCount < v1.size() - 1))
	frameCount++;
      n++;

	
      //if (frameCount >= v1.size())
      //{
      //	cout << "frameCount > v1.size." << endl;
      //	exit(1);
      //}
      //float* ptr = v1[frameCount].ptr<float>(0);
      //if (n % 5 == 0)
      //	frameCount++;
      //n++;
      ////std::cout << "n = " << n << endl;
      //GLfloat scale = 0.01f;
      //GLfloat scale_1 = 0.001f;
      //GLfloat inv_w = GLfloat(1.0f / ptr[3]);

      ////scale *= inv_w;
      //glm::vec3 v2 = glm::vec3((GLfloat)ptr[0] * scale_1 *inv_w, (GLfloat)ptr[1] * scale_1*inv_w, (GLfloat)ptr[2] * scale_1*inv_w);
      //glm::vec3 v3 = glm::vec3(-(GLfloat)ptr[0], (GLfloat)ptr[1], (GLfloat)ptr[2]);
      //glm::vec3 v4 = glm::normalize(v3);

      //glm::mat4 model;
      //model = glm::translate(model, v2);
      //model = glm::scale(model, glm::vec3(scale, scale, scale));

      ////send model matrix
      //glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
      //
      ////draw something...
      ///******************************/
      //glDrawArrays(GL_TRIANGLES, 0, 36);
      ////draw a triangle
      ////param 1:the basic graph that we wanna draw
      ////param 2:the index of the position of the vertex that we will start with in the vertex table
      ////param 3:the number of the vertex that we want to draw
      ///*****************************/

      glBindVertexArray(VAO2);

      glm::mat4 model;
      model = glm::rotate(model, glm::radians(180.0f), glm::vec3(-1.0f, -1.0f, 0.0f));

      glUniformMatrix4fv(glGetUniformLocation(shader2.Program, "view"), 1, GL_FALSE, glm::value_ptr(myView));
      glUniformMatrix4fv(glGetUniformLocation(shader2.Program, "projection"), 1, GL_FALSE, glm::value_ptr(myProjection));
      glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindVertexArray(0);


      //glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
      //for (GLuint i = 0; i < 10; i++)
      //{
      //	glm::mat4 model;
      //	model = glm::translate(model, cubePositions[i]);
      //	GLfloat angle = 20.0f*i;
      //	if (i % 3 == 0)
      //		angle = glfwGetTime()*25.0f;
      //	model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      //	glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
      //	glDrawArrays(GL_TRIANGLES, 0, 36);
      //}
      //glDrawArrays(GL_TRIANGLES, 0, 36);

      glBindVertexArray(0);

      //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
      //after drawing, unfasten the VAO again
      //this function should be put here, at the end of the loop.
      glfwSwapBuffers(window);//call this function to swap the buffer which is storing the data
    };

  glDeleteVertexArrays(1, &VAO1);
  glDeleteBuffers(1, &VBO1);

  glfwTerminate();//to release the memory
  return;
}

//key call-back function
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  GLfloat currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  if (action == GLFW_PRESS)
    {
      keys[key] = true;
    }
  else if (action == GLFW_RELEASE)
    {
      keys[key] = false;
    }
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
  if (firstMouse)
    {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }
  GLfloat xoffset = xpos - lastX;
  GLfloat yoffset = lastY - ypos; //here it's inverse
  lastX = xpos;
  lastY = ypos;
  myCamera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
  myCamera.ProcessMouseScroll(yoffset);
}

void do_movement()
{
  GLfloat currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  if (keys[GLFW_KEY_W])
    {
      myCamera.ProcessKeyboard(FORWARD, deltaTime);
    }
  if (keys[GLFW_KEY_S])
    {
      myCamera.ProcessKeyboard(BACKWARD, deltaTime);
    }
  if (keys[GLFW_KEY_A])
    {
      myCamera.ProcessKeyboard(LEFT, deltaTime);
    }
  if (keys[GLFW_KEY_D])
    {
      myCamera.ProcessKeyboard(RIGHT, deltaTime);
    }
}