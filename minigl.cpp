/**
 * minigl.cpp
 * -------------------------------
 * Implement miniGL here.
 * Do not use any additional files
 */

#include <cstdio>
#include <cstdlib>
#include "minigl.h"

#include <iostream>
#include <stack>
#include <tuple>
#include <vector>
#include <limits>
#include <algorithm>
#include <math.h>

using namespace std;

#define FLT_MAX numeric_limits<float>::max()
#define FLT_MIN numeric_limits<float>::min()

class Vertex;
class Matrix4X4;
class MinimumBoundingBox;

stack <Matrix4X4> modelViewMatrixStack;
stack <Matrix4X4> projetionMatrixStack;

MGLmatrix_mode currentMatrixMode;
MGLpoly_mode currentGeometryMode;
MGLcapability_mode currentTextureMode;

MGLpixel color[3];
unsigned int* textureData;
int textureWidth = 0;
int textureHeigth = 0;

vector<Vertex> vertexList;
vector<vector<Vertex>> geometryList;

vector<vector<MGLfloat>> zBuffer;
vector<vector<MGLpixel>> frameBuffer;

bool textureEnabled = false;

/**
 * Standard macro to report errors
 */
inline void MGL_ERROR(const char* description) {
    printf("%s\n", description);
    exit(1);
}

class Vertex
{
public:
	MGLfloat x, y, z, w;
	MGLpixel vertexColor[3];
	bool textureEnabled = false;
	MGLfloat u, v;

	Vertex() :x(0), y(0), z(0), w(1), u(0), v(0)
	{
		applyVertexColor();
	}

	Vertex(MGLfloat x, MGLfloat y, MGLfloat z, MGLfloat w) :x(x), y(y), z(z), w(w)
	{
		applyVertexColor();
	}

	Vertex& operator= (const Vertex& vertex)
	{
		if (this != &vertex)
		{
			x = vertex.x;
			y = vertex.y;
			z = vertex.z;
			w = vertex.w;
			vertexColor[0] = vertex.vertexColor[0];
			vertexColor[1] = vertex.vertexColor[1];
			vertexColor[2] = vertex.vertexColor[2];
		}
		return *this;
	}

private:
	void applyVertexColor()
	{
		vertexColor[0] = color[0];
		vertexColor[1] = color[1];
		vertexColor[2] = color[2];
	}
};

Vertex currentTextureCoordinates;

class Matrix4X4
{
public:
	MGLfloat matrix[4][4];

	Matrix4X4()
	{
		init(1);
	}

	Matrix4X4(MGLfloat x)
	{
		init(x);
	}

	void populateViewportMatrix(MGLfloat width, MGLfloat height){
		init(1);
		matrix[0][0] = width/2.0;
		matrix[1][1] = height/2.0;
		matrix[0][3] = (width-1)/2.0;
		matrix[1][3] = (height-1)/2.0;
	}

	Matrix4X4 operator* (const Matrix4X4& input)
	{
		Matrix4X4 output(0);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++){
					output.matrix[i][j] += this->matrix[i][k] * input.matrix[k][j];
				}
			}
		}
		return output;
	}


	Vertex operator* (const Vertex& vertex)
	{
		Vertex output = vertex;
		MGLfloat inputVertex[4] = {vertex.x,vertex.y,vertex.z,vertex.w};
		MGLfloat outputVertex[4];
		MGLfloat sum;
		for (int i = 0; i < 4; i++)
		{
			sum = 0;
			for (int j = 0; j < 4; j++)
			{
				sum += matrix[i][j] * inputVertex[j];
			}
			outputVertex[i] = sum;
		}
		output.x = outputVertex[0];
		output.y = outputVertex[1];
		output.z = outputVertex[2];
		output.w = outputVertex[3];
		return output;
	}

	Matrix4X4& operator= (const Matrix4X4& matrix)
	{
		if (this != &matrix)
		{
			for (int row = 0; row < 4; ++row){
				for (int col = 0; col < 4; ++col){
					this->matrix[row][col] = matrix.matrix[row][col];
				}
			}
		}
		return *this;
	}

private:
	void init (MGLfloat x)
	{
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				if(i==j){
					matrix[i][j] = x;
				} else{
					matrix[i][j] = 0;
				}
			}
		}
	}
};

class MinimumBoundingBox
{
public:
	MGLfloat x_min, x_max, y_min, y_max;

	MinimumBoundingBox()
		: x_min(0), x_max(0), y_min(0), y_max(0)
	{}

	MinimumBoundingBox(const vector<Vertex> geometry){
		vector<MGLfloat> x_coordinates;
		vector<MGLfloat> y_coordinates;
		for(unsigned int i=0;i<geometry.size();i++){
			x_coordinates.push_back(geometry[i].x);
			y_coordinates.push_back(geometry[i].y);
		}
		x_min = getMin(x_coordinates);
		x_max = getMax(x_coordinates);
		y_min = getMin(y_coordinates);
		y_max = getMax(y_coordinates);
	}

private:
	MGLfloat getMin(const vector<MGLfloat> vector){
		MGLfloat min = FLT_MAX;
		for(unsigned int i=0;i<vector.size();i++){
			if(vector[i]<min){
				min = vector[i];
			}
		}
		return min;
	}

	MGLfloat getMax(const vector<MGLfloat> vector){
		MGLfloat max = FLT_MIN;
		for(unsigned int i=0;i<vector.size();i++){
			if(vector[i]>max){
				max = vector[i];
			}
		}
		return max;
	}
};

MGLfloat distanceOfPointFromTheLine(MGLfloat x, MGLfloat y, MGLfloat endpoint1_x, MGLfloat endpoint1_y, MGLfloat endpoint2_x, MGLfloat endpoint2_y)
{
	return (endpoint1_y - endpoint2_y)*x + (endpoint2_x - endpoint1_x)*y + (endpoint1_x * endpoint2_y) - (endpoint2_x * endpoint1_y);
}

void rasterizeTriangle(vector<Vertex>& triangle){

	// Initiating Minimum Bounding Box for the triangle.
	MinimumBoundingBox minimumBoundingBox(triangle);

	// Rounding off x and y coordinates of the triangle vertices to nearest integers.
	triangle[0].x = round(triangle[0].x);
	triangle[0].y = round(triangle[0].y);
	triangle[1].x = round(triangle[1].x);
	triangle[1].y = round(triangle[1].y);
	triangle[2].x = round(triangle[2].x);
	triangle[2].y = round(triangle[2].y);

	// Boundaries of the bounding boxes are rounded off to the integers.
	// The lower bounds are rounded off to the floor (largest integer less than the value) and
	// the upper bounds are rounded off to the ceiling (smallest integer greater than the value).
	minimumBoundingBox.x_min = floor(minimumBoundingBox.x_min);
	minimumBoundingBox.x_max = ceil(minimumBoundingBox.x_max);
	minimumBoundingBox.y_min = floor(minimumBoundingBox.y_min);
	minimumBoundingBox.y_max = ceil(minimumBoundingBox.y_max);

	for(int x=minimumBoundingBox.x_min;x<minimumBoundingBox.x_max;x++){
		for(int y=minimumBoundingBox.y_min;y<minimumBoundingBox.y_max;y++){

			MGLfloat alpha = distanceOfPointFromTheLine(x,y,triangle[1].x,triangle[1].y,triangle[2].x,triangle[2].y)/distanceOfPointFromTheLine(triangle[0].x,triangle[0].y,triangle[1].x,triangle[1].y,triangle[2].x,triangle[2].y);
			MGLfloat beta = distanceOfPointFromTheLine(x,y,triangle[0].x,triangle[0].y,triangle[2].x,triangle[2].y)/distanceOfPointFromTheLine(triangle[1].x,triangle[1].y,triangle[0].x,triangle[0].y,triangle[2].x,triangle[2].y);
			MGLfloat gamma = distanceOfPointFromTheLine(x,y,triangle[0].x,triangle[0].y,triangle[1].x,triangle[1].y)/distanceOfPointFromTheLine(triangle[2].x,triangle[2].y,triangle[0].x,triangle[0].y,triangle[1].x,triangle[1].y);

			if (alpha >= 0 && beta >= 0 && gamma >= 0)
			{
				MGLpixel interpolatedColor = 0;
				if(triangle[0].textureEnabled && triangle[1].textureEnabled && triangle[2].textureEnabled){

					MGLfloat d = triangle[1].w*triangle[2].w + triangle[2].w*beta*(triangle[0].w-triangle[1].w) + triangle[1].w*gamma*(triangle[0].w-triangle[2].w);
					MGLfloat beta_w = triangle[0].w*triangle[2].w*beta/d;
					MGLfloat gamma_w = triangle[0].w*triangle[1].w*gamma/d;
					MGLfloat alpha_w = 1-beta_w-gamma_w;

					int interpolatedTextureU = (int)((alpha_w*triangle[0].u + beta_w*triangle[1].u + gamma_w*triangle[2].u)*256);
					int interpolatedTextureV = (int)((alpha_w*triangle[0].v + beta_w*triangle[1].v + gamma_w*triangle[2].v)*256);
					interpolatedColor = textureData[interpolatedTextureV*textureWidth + interpolatedTextureU];
				} else{
					MGLfloat interpolatedRedComponet = alpha*triangle[0].vertexColor[0] + beta*triangle[1].vertexColor[0]+ gamma*triangle[2].vertexColor[0];
					MGLfloat interpolatedGreenComponet = alpha*triangle[0].vertexColor[1] + beta*triangle[1].vertexColor[1]+ gamma*triangle[2].vertexColor[1];
					MGLfloat interpolatedBlueComponet = alpha*triangle[0].vertexColor[2] + beta*triangle[1].vertexColor[2]+ gamma*triangle[2].vertexColor[2];

					MGL_SET_RED(interpolatedColor, (MGLpixel) interpolatedRedComponet);
					MGL_SET_GREEN(interpolatedColor, (MGLpixel) interpolatedGreenComponet);
					MGL_SET_BLUE(interpolatedColor, (MGLpixel) interpolatedBlueComponet);
				}

				MGLfloat interpolatedZValue = alpha*triangle[0].z + beta*triangle[1].z + gamma*triangle[2].z;

				if(interpolatedZValue < zBuffer[y][x]){
					zBuffer[y][x] = interpolatedZValue;
					frameBuffer[y][x] = interpolatedColor;
				}
			}
		}
	}
}

void applyProjectionTransformation(Vertex& vertex){
	vertex = modelViewMatrixStack.top() * vertex;
	vertex = projetionMatrixStack.top() * vertex;
}

void applyViewportTransformation(Matrix4X4 viewport, Vertex& vertex, MGLsize width, MGLsize height){

	vertex = viewport * vertex;
	vertex.x = vertex.x/vertex.w;
	vertex.y = vertex.y/vertex.w;
	vertex.z = vertex.z/vertex.w;
	vertex.w = 1;
	if (vertex.x > width){
		vertex.x = width;
	}
	if (vertex.y > height){
		vertex.y = height;
	}
}

/**
 * Read pixel data starting with the pixel at coordinates
 * (0, 0), up to (width,  height), into the array
 * pointed to by data.  The boundaries are lower-inclusive,
 * that is, a call with width = height = 1 would just read
 * the pixel at (0, 0).
 *
 * Rasterization and z-buffering should be performed when
 * this function is called, so that the data array is filled
 * with the actual pixel values that should be displayed on
 * the two-dimensional screen.
 */
void mglReadPixels(MGLsize width,
                   MGLsize height,
                   MGLpixel *data)
{
	if(geometryList.size()>0){
		zBuffer.resize(height);
		frameBuffer.resize(height);
		for(unsigned int i=0;i<height;i++){
			frameBuffer[i].assign(width,0);
			zBuffer[i].assign(width,FLT_MAX);
		}
	}
	// Viewport matrix is computed at this stage as we now know the window size.
	// The matrix has been passed as parameter to avoid recomputing it for each vertex.
	Matrix4X4 viewport(1);
	viewport.populateViewportMatrix(width,height);

	for(unsigned int i=0;i<geometryList.size();i++){
		for(unsigned int j=0;j<geometryList[i].size();j++){
			applyViewportTransformation(viewport,geometryList[i][j],width,height);
		}
		if(geometryList[i].size()==3){
			rasterizeTriangle(geometryList[i]);
		}
		if(geometryList[i].size()==4){
			vector<Vertex> triangle;
			triangle.push_back(geometryList[i][0]);
			triangle.push_back(geometryList[i][1]);
			triangle.push_back(geometryList[i][2]);
			rasterizeTriangle(triangle);
			triangle.clear();
			triangle.push_back(geometryList[i][0]);
			triangle.push_back(geometryList[i][3]);
			triangle.push_back(geometryList[i][2]);
			rasterizeTriangle(triangle);
		}
	}
	for(unsigned int i=0;i<zBuffer.size();i++){
		for(unsigned int j=0;j<zBuffer[i].size();j++){
			data[i*width + j] = frameBuffer[i][j];
		}
	}
	if(geometryList.size()>0){
		geometryList.clear();
	}
}

/**
 * Start specifying the vertices for a group of primitives,
 * whose type is specified by the given mode.
 */
void mglBegin(MGLpoly_mode mode)
{
	// https://www.opengl.org/sdk/docs/man2/xhtml/glBegin.xml
	switch (mode) {
	case MGL_TRIANGLES:
		currentGeometryMode = mode;
		break;
	case MGL_QUADS:
		currentGeometryMode = mode;
		break;
	default:
		MGL_ERROR("This polygon mode has not been implemented.");
		break;
	}
}

/**
 * Stop specifying the vertices for a group of primitives.
 */
void mglEnd()
{
	// https://www.opengl.org/sdk/docs/man2/xhtml/glBegin.xml
	switch (currentGeometryMode) {
	case MGL_TRIANGLES:
		if(vertexList.size()==3){
			geometryList.push_back(vertexList);
			vertexList.clear();
		}
		break;
	case MGL_QUADS:
		if(vertexList.size()==4){
			geometryList.push_back(vertexList);
			vertexList.clear();
		}
		break;
	default:
		MGL_ERROR("This polygon mode has not been implemented.");
		break;
	}
}

void mglTexCoord2f(MGLfloat x,
        		   MGLfloat y){
	currentTextureCoordinates.x = x;
	currentTextureCoordinates.y = y;
}

/**
 * Specify a two-dimensional vertex; the x- and y-coordinates
 * are explicitly specified, while the z-coordinate is assumed
 * to be zero.  Must appear between calls to mglBegin() and
 * mglEnd().
 */
void mglVertex2(MGLfloat x,
                MGLfloat y)
{
	switch (currentGeometryMode) {
	case MGL_TRIANGLES:
		if(vertexList.size()==3){
			geometryList.push_back(vertexList);
			vertexList.clear();
		}
		break;
	case MGL_QUADS:
		if(vertexList.size()==4){
			geometryList.push_back(vertexList);
			vertexList.clear();
		}
		break;
	default:
		MGL_ERROR("This polygon mode has not been implemented.");
		break;
	}
	Vertex vertex(x,y,0,1);
	if(textureEnabled){
		vertex.u = currentTextureCoordinates.x;
		vertex.v = currentTextureCoordinates.y;
		vertex.textureEnabled = true;
	}
	applyProjectionTransformation(vertex);
	vertexList.push_back(vertex);
}

/**
 * Specify a three-dimensional vertex.  Must appear between
 * calls to mglBegin() and mglEnd().
 */
void mglVertex3(MGLfloat x,
                MGLfloat y,
                MGLfloat z)
{
	switch (currentGeometryMode) {
	case MGL_TRIANGLES:
		if(vertexList.size()==3){
			geometryList.push_back(vertexList);
			vertexList.clear();
		}
		break;
	case MGL_QUADS:
		if(vertexList.size()==4){
			geometryList.push_back(vertexList);
			vertexList.clear();
		}
		break;
	default:
		MGL_ERROR("This polygon mode has not been implemented.");
		break;
	}
	Vertex vertex(x,y,z,1);
	if(textureEnabled){
		vertex.u = currentTextureCoordinates.x;
		vertex.v = currentTextureCoordinates.y;
		vertex.textureEnabled = true;
	}
	applyProjectionTransformation(vertex);
	vertexList.push_back(vertex);
}

/**
 * Set the current matrix mode (modelview or projection).
 */
void mglMatrixMode(MGLmatrix_mode mode)
{
	switch (mode) {
	case MGL_MODELVIEW:
		currentMatrixMode = mode;
		break;
	case MGL_PROJECTION:
		currentMatrixMode = mode;
		break;
	default:
		MGL_ERROR("This matrix mode has not been implemented.");
		break;
	}
}

/**
 * Push a copy of the current matrix onto the stack for the
 * current matrix mode.
 */
void mglPushMatrix()
{
	switch (currentMatrixMode) {
	case MGL_MODELVIEW:
		if (!modelViewMatrixStack.empty()) {
			modelViewMatrixStack.push(modelViewMatrixStack.top());
		}
		break;
	case MGL_PROJECTION:
		if (!projetionMatrixStack.empty()) {
			projetionMatrixStack.push(projetionMatrixStack.top());
		}
		break;
	default:
		break;
	}
}

/**
 * Pop the top matrix from the stack for the current matrix
 * mode.
 */
void mglPopMatrix()
{
	switch (currentMatrixMode) {
	case MGL_MODELVIEW:
		if (!modelViewMatrixStack.empty()) {
			modelViewMatrixStack.pop();
		}
		break;
	case MGL_PROJECTION:
		if (!projetionMatrixStack.empty()) {
			projetionMatrixStack.pop();
		}
		break;
	default:
		break;
	}
}

/**
 * Replace the current matrix with the identity.
 * https://www.opengl.org/sdk/docs/man2/xhtml/glLoadIdentity.xml
 */
void mglLoadIdentity()
{
	Matrix4X4 identityMatrix;
	switch (currentMatrixMode) {
	case MGL_MODELVIEW:
		if(!modelViewMatrixStack.empty()){
			modelViewMatrixStack.pop();
		}
		modelViewMatrixStack.push(identityMatrix);
		break;
	case MGL_PROJECTION:
		if(!projetionMatrixStack.empty()){
			projetionMatrixStack.pop();
		}
		projetionMatrixStack.push(identityMatrix);
		break;
	default:
		break;
	}
}

/**
 * Replace the current matrix with an arbitrary 4x4 matrix,
 * specified in column-major order.  That is, the matrix
 * is stored as:
 *
 *   ( a0  a4  a8  a12 )
 *   ( a1  a5  a9  a13 )
 *   ( a2  a6  a10 a14 )
 *   ( a3  a7  a11 a15 )
 *
 * where ai is the i'th entry of the array.
 */
void mglLoadMatrix(const MGLfloat *matrix)
{
	/**
	 * This function is never used.
	 */
}

/**
 * Multiply the current matrix by an arbitrary 4x4 matrix,
 * specified in column-major order.  That is, the matrix
 * is stored as:
 *
 *   ( a0  a4  a8  a12 )
 *   ( a1  a5  a9  a13 )
 *   ( a2  a6  a10 a14 )
 *   ( a3  a7  a11 a15 )
 *
 * where ai is the i'th entry of the array.
 */
void mglMultMatrix(const MGLfloat *matrix)
{
	/**
	 * This function is never used.
	 */
}

/**
 * The following method right multiplies the given matrix with the top element of the stack.
 */
void updateStackTopElement(const Matrix4X4& matrix) {
	Matrix4X4 currentMatrix;
	Matrix4X4 newMatrix;
	switch (currentMatrixMode) {
	case MGL_MODELVIEW:
		currentMatrix = modelViewMatrixStack.top();
		modelViewMatrixStack.pop();
		newMatrix = currentMatrix * matrix;
		modelViewMatrixStack.push(newMatrix);
		break;
	case MGL_PROJECTION:
		currentMatrix = projetionMatrixStack.top();
		projetionMatrixStack.pop();
		newMatrix = currentMatrix * matrix;
		projetionMatrixStack.push(newMatrix);
		break;
	default:
		break;
	}
}

/**
 * Multiply the current matrix by the translation matrix
 * for the translation vector given by (x, y, z).
 */
void mglTranslate(MGLfloat x,
                  MGLfloat y,
                  MGLfloat z)
{
	Matrix4X4 translation(1);
	translation.matrix[0][3] = x;
	translation.matrix[1][3] = y;
	translation.matrix[2][3] = z;
	updateStackTopElement(translation);
}

void normalize(MGLfloat &x, MGLfloat &y, MGLfloat &z)
{
	MGLfloat magnitude = sqrt(pow(x,2) + pow(y,2) + pow(z,2));
	if (magnitude > 0){
		x = x/magnitude;
		y = y/magnitude;
		z = z/magnitude;
	}
}

/**
 * Multiply the current matrix by the rotation matrix
 * for a rotation of (angle) degrees about the vector
 * from the origin to the point (x, y, z).
 */
void mglRotate(MGLfloat angle,
               MGLfloat x,
               MGLfloat y,
               MGLfloat z)
{
	// https://www.opengl.org/sdk/docs/man2/xhtml/glRotate.xml
	MGLfloat sine = sin(angle * M_PI / 180);
	MGLfloat cosine = cos(angle * M_PI / 180);

	normalize(x, y, z);
	Matrix4X4 rotation(1);

	rotation.matrix[0][0] = x*x * (1 - cosine) + cosine;
	rotation.matrix[0][1] = x*y * (1 - cosine) - z*sine;
	rotation.matrix[0][2] = x*z * (1 - cosine) + y*sine;

	rotation.matrix[1][0] = y*x * (1 - cosine) + z*sine;
	rotation.matrix[1][1] = y*y * (1 - cosine) + cosine;
	rotation.matrix[1][2] = y*z * (1 - cosine) - x*sine;

	rotation.matrix[2][0] = x*z * (1 - cosine) - y*sine;
	rotation.matrix[2][1] = y*z * (1 - cosine) + x*sine;
	rotation.matrix[2][2] = z*z * (1 - cosine) + cosine;

	updateStackTopElement(rotation);
}

/**
 * Multiply the current matrix by the scale matrix
 * for the given scale factors.
 */
void mglScale(MGLfloat x,
              MGLfloat y,
              MGLfloat z)
{
	Matrix4X4 scale(1);
	scale.matrix[0][0] = x;
	scale.matrix[1][1] = y;
	scale.matrix[2][2] = z;

	updateStackTopElement(scale);
}

/**
 * Multiply the current matrix by the perspective matrix
 * with the given clipping plane coordinates.
 */
void mglFrustum(MGLfloat left,
                MGLfloat right,
                MGLfloat bottom,
                MGLfloat top,
                MGLfloat near,
                MGLfloat far)
{
	// https://www.opengl.org/sdk/docs/man2/xhtml/glFrustum.xml
	MGLfloat x, y, A, B, C, D;

	x = (2 * near)/(right - left);
	y = (2 * near)/(top - bottom);
	A = (right + left)/(right - left);
	B = (top + bottom)/(top - bottom);
	C = -(far + near)/(far - near);
	D = -(2 * far * near)/(far - near);

	Matrix4X4 perspective(0);

	perspective.matrix[0][0] = x;
	perspective.matrix[1][1] = y;

	perspective.matrix[0][2] = A;
	perspective.matrix[1][2] = B;
	perspective.matrix[2][2] = C;
	perspective.matrix[2][3] = D;
	perspective.matrix[3][2] = -1;

	updateStackTopElement(perspective);
}

/**
 * Multiply the current matrix by the orthographic matrix
 * with the given clipping plane coordinates.
 */
void mglOrtho(MGLfloat left,
              MGLfloat right,
              MGLfloat bottom,
              MGLfloat top,
              MGLfloat near,
              MGLfloat far)
{
	// https://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml
	Matrix4X4 ortho;

	MGLfloat x_ortho = 2.0/(right - left);
	MGLfloat y_ortho = 2.0/(top - bottom);
	MGLfloat z_ortho = 2.0/(near - far);

	ortho.matrix[0][0] = x_ortho;
	ortho.matrix[1][1] = y_ortho;
	ortho.matrix[2][2] = z_ortho;

	MGLfloat translation_x = -(right + left)/(right - left);
	MGLfloat translation_y = -(top + bottom)/(top - bottom);
	MGLfloat translation_z = -(near + far)/(far - near);

	ortho.matrix[0][3] = translation_x;
	ortho.matrix[1][3] = translation_y;
	ortho.matrix[2][3] = translation_z;

	updateStackTopElement(ortho);
}

/**
 * Set the current color for drawn shapes.
 */
void mglColor(MGLbyte red,
              MGLbyte green,
              MGLbyte blue)
{
	color[0] = red;
	color[1] = green;
	color[2] = blue;
}

/**
 * Enable the specified server-side capability.
 */
void mglEnable(MGLcapability_mode mode){
	switch (mode) {
	case MGL_TEXTURE_2D:
		currentTextureMode = mode;
		textureEnabled = true;
		break;
	default:
		MGL_ERROR("This feature has not been implemented.");
		break;
	}
}

/**
 * Disable the specified server-side capability.
 */
void mglDisable(MGLcapability_mode mode){
	switch (mode) {
	case MGL_TEXTURE_2D:
		textureEnabled = false;
		break;
	default:
		MGL_ERROR("This feature has not been implemented.");
		break;
	}
}

/**
 * Store the texture image details.
 */
void mglTexImage2D(MGLcapability_mode mode, int width, int height, unsigned int* data){
	textureWidth = width;
	textureHeigth = height;
	textureData = data;
}
