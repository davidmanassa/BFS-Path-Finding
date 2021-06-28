// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// GLM header file
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

// shaders header file
#include "common/shader.hpp"

#include "shape.hpp"
#include "BFS.hpp"

// Vertex array object (VAO)
GLuint VertexArrayID;

// GLSL program from the shaders
GLuint programID;

GLint WindowWidth = 800;
GLint WindowHeight = 600;

static vector<GLuint> vertexbuffer, colorbuffer;
static vector<Shape> shapes;

vector<vector<int>> map1;

float squareSize = 2;
float k = squareSize - 0.08f;
int mapSizeX = 5;
int mapSizeY = 5;
int offset = 40;

int loadShape( int index, Shape shape, glm::vec3 color ) {

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

	int length = shape.points.size() + (2 * (shape.points.size() - 3));
    length *= 3; // 3 valores por vetor (x, y, z)

	static GLfloat* g_vertex_buffer_data = (GLfloat*) malloc(length * sizeof(GLfloat)); 
	static GLfloat* g_color_buffer_data = (GLfloat*) malloc(length * sizeof(GLfloat));
	
    int i = 0;
    int numberOfEdgesInTheFace = shape.points.size() + (2 * (shape.points.size() - 3));
	int numberOfTriangles = numberOfEdgesInTheFace / 3;

    int nTriangle = 1, nPoint = 1;

    for (int p = 0; p < shape.points.size(); p++) {

        glm::vec2 point = shape.points[p];

        g_vertex_buffer_data[i] = point.x;
        g_vertex_buffer_data[i + 1] = point.y;
        g_vertex_buffer_data[i + 2] = 0;
        g_color_buffer_data[i] = color.x;
        g_color_buffer_data[i + 1] = color.y;
        g_color_buffer_data[i + 2] = color.z;
        i += 3;
        nPoint += 1;

        // std::cout << " N " << point.x << " " << point.y << std::endl;

        if ((3 * nTriangle) == nPoint && nTriangle != 1) { // 6, 9, 12, ... (ultimo ponto dos triangulos depois do primeiro)
            g_vertex_buffer_data[i] = shape.points[0].x;
            g_vertex_buffer_data[i + 1] = shape.points[0].y;
            g_vertex_buffer_data[i + 2] = 0;
            g_color_buffer_data[i] = color.x;
            g_color_buffer_data[i + 1] = color.y;
            g_color_buffer_data[i + 2] = color.z;
            i += 3;
            nPoint += 1;

            // std::cout << " F " << shape.points[0].x << " " << shape.points[0].y << std::endl;
        }
        if (nPoint == ((3 * nTriangle) + 1) && numberOfEdgesInTheFace != 3 && nTriangle < numberOfTriangles) { // 4, 7, 10, ... (primeiro de cada triangulo depois do primeiro)
            g_vertex_buffer_data[i] = point.x;
            g_vertex_buffer_data[i + 1] = point.y;
            g_vertex_buffer_data[i + 2] = 0;
            g_color_buffer_data[i] = color.x;
            g_color_buffer_data[i + 1] = color.y;
            g_color_buffer_data[i + 2] = color.z;
            i += 3;
            nPoint += 1;
            nTriangle += 1;

            // std::cout << " I " << point.x << " " << point.y << std::endl;
        }

    }

    vertexbuffer.push_back(NULL);
    colorbuffer.push_back(NULL);

    glGenBuffers(1, &vertexbuffer[index]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[index]);
	glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW); // float 4 bits

	glGenBuffers(1, &colorbuffer[index]);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer[index]);
	glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), g_color_buffer_data, GL_STATIC_DRAW);

	return length / 3;

}

void setMVP() {

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glm::mat4 mvp = glm::ortho(-80.0f, 80.0f, -60.0f, 60.0f);
    unsigned int matrix = glGetUniformLocation(programID, "mvp");
    glUniformMatrix4fv(matrix, 1, GL_FALSE, &mvp[0][0]);

}

void drawShape(int index, int nPoints, glm::vec2 transform) {

    glm::mat4 trans = glm::translate(glm::mat4(1.0), glm::vec3(transform.x, transform.y, 0.0f));

    unsigned int m = glGetUniformLocation(programID, "trans");
    glUniformMatrix4fv(m, 1, GL_FALSE, &trans[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[index]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer[index]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, nPoints * 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

}

void cleanupDataFromGPU()
{
    glDeleteBuffers(1, &vertexbuffer[0]);
    glDeleteBuffers(1, &colorbuffer[0]);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
}

GLfloat rotation = 0.0f;

int readMap ( char* fileName ) {

    std::ifstream infile(fileName);

    std::string line;

    int lineNumber = 0;
    mapSizeX = 0;
    mapSizeY = 0;

    while (std::getline(infile, line)) {

        if (line.at(0) != '.' && line.at(0) != 'T' && line.at(0) != '@')
            continue;

        map1.push_back(vector<int>());

        mapSizeY = 0;

        for (char& c : line) {

            if (c == '.') // chão
                map1[lineNumber].push_back(0);
            else if (c == 'T') // arvore
                map1[lineNumber].push_back(1);
            else // @ muro
                map1[lineNumber].push_back(2);

            mapSizeY += 1;

        }

        lineNumber += 1;
        mapSizeX += 1;

    }


    return 0;

}

int matToLst(int x, int y) {
    return (x * mapSizeY) + y;
}

glm::vec2 lstToMat(int i) {
    int x = int(i / mapSizeY);
    return glm::vec2(x, i - (x * mapSizeY));
}


int main( void ) {

    // Initialise GLFW
    glewExperimental = true; // Needed for core profile
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window

    window = glfwCreateWindow( WindowWidth, WindowHeight, "BFS", NULL, NULL);
    if( window == NULL ) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }

    // Create window context
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "SimpleVertexShader.vert", "SimpleFragmentShader.frag" );

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glUseProgram(programID);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


    readMap("den009d.map");

    // Ponto inicial e final
    map1[10][10] = 3;
    map1[10][40] = 4;

    int lstSize = int(mapSizeX * mapSizeY);

    Graph g;

    vector<int> adj[lstSize];

    g.createEdges(map1, adj);

    vector<int> sp = g.shortestDistance(adj, matToLst(10, 10), matToLst(10,40), lstSize);


    /**
     *  i,j ---- i,j+k
     *   |         |
     *   |         |
     *  i+k,j ---- i+k,j+k
     **/

    Shape s;
    s.points.push_back(glm::vec2(0.02f, 0.02f));
    s.points.push_back(glm::vec2(0.02f + k, 0.02f));
    s.points.push_back(glm::vec2(0.02f + k, 0.02f + k));
    s.points.push_back(glm::vec2(0.02f, 0.02f + k));

    /**
     * 0 -> chão . -> branco
     * 1 -> arvore T -> verde
     * 2 -> muro @ -> cinzento
     **/

    int nPoints = loadShape(0, s, glm::vec3(1,1,1));
    loadShape(1, s, glm::vec3(0, 0.7, 0));
    loadShape(2, s, glm::vec3(0.3,0.3,0.3));
    // inicial e final
    loadShape(3, s, glm::vec3(0,0,0.9));
    loadShape(4, s, glm::vec3(0.9,0,0));
    // pesquisado e proximoAPesquisar
    loadShape(5, s, glm::vec3(1,0.6,0));
    loadShape(6, s, glm::vec3(1,1,0)); //proximo
    // path
    loadShape(7, s, glm::vec3(0,0,0.4));


    float x = 0;
    float y = 0;

    do {

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        
        setMVP();

        for (int i = 0; i < mapSizeX; i++) {
            for (int j = 0; j < mapSizeY; j++) {
                s.translate(glm::vec2(squareSize, 0));
                x += squareSize;
                if (j == 0) {
                    s.translate(glm::vec2(-squareSize * mapSizeY, squareSize));
                    x += (-squareSize * mapSizeY);
                    y += squareSize;
                }
                if (std::find(sp.begin(), sp.end(), matToLst(i,j)) != sp.end() && map1[i][j] != 3 && map1[i][j] != 4)
                    drawShape(5, nPoints, glm::vec2(x +offset, y -offset));
                else
                    drawShape(map1[i][j], nPoints, glm::vec2(x +offset, y -offset)); 
            }   
        }


        glfwSwapBuffers(window);
        glfwPollEvents();

        x = 0; y = 0;
        
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    
    // Cleanup VAO, VBOs, and shaders from GPU
    cleanupDataFromGPU();
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
    return 0;
}

