static GLfloat gl_trans[6][3] = {
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0},
    {1.0, 0.0, 0.0},
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, -1.0, 0.0}
};
static GLfloat gl_rots[6][4] = {
    {0.0, 0.0, 1.0, 0.0},
    {180.0, 0.0, 1.0, 0.0},
    {90.0, 0.0, 1.0, 0.0},
    {-90.0, 0.0, 1.0, 0.0},
    {-90.0, 1.0, 0.0, 0.0},
    {90.0, 1.0, 0.0, 0.0}
};
static GLfloat gl_cols[6][3] = {
    {1, 0, 0},
    {0, 1, 1},
    {0, 1, 0},
    {1, 0, 1},
    {0, 0, 1},
    {1, 1, 0}
};

unsigned int* readBmpFile(char* filename, int &width, int &height)
{
    FILE* f = fopen(filename, "rb");

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f);

    width = *(int*)&info[18];
    height = *(int*)&info[22];

    int rowLength = (width*3 + 3) & (~3);
    unsigned char* data = new unsigned char[rowLength];
    unsigned int* textureData = new unsigned int[rowLength*height/3];

    for(int i = 0; i < height; i++)
    {
    	fread(data, sizeof(unsigned char), rowLength, f);
    	for(int j = 0,k=0; j < width*3; j += 3,k+=1)
        {
    		int textureColor = 0;
    		MGL_SET_RED(textureColor, (MGLpixel) data[j+2]);
			MGL_SET_GREEN(textureColor, (MGLpixel) data[j+1]);
			MGL_SET_BLUE(textureColor, (MGLpixel) data[j]);
			MGL_SET_ALPHA(textureColor, (MGLpixel) 1);
			textureData[i*width+k] = textureColor;
        }
    }
    fclose(f);
    return textureData;
}

void glTests(const int test_number)
{
	int height = 0, width = 0;
	unsigned int* textureData;
    switch(test_number)
    {
        case 1:
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glColor3f(1,1,1);
            glBegin(GL_TRIANGLES);
            glVertex2f(0.25, 0.25);
            glVertex2f(0.75, 0.25);
            glVertex2f(0.75, 0.75);
            glEnd();
            break;
        case 2:
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glColor3f(1, 1, 1);
            glBegin(GL_QUADS);
            glVertex3f(-1.0, -1.0, -5.0);
            glVertex3f(1.0, -1.0, -2.0);
            glVertex3f(1.0, 1.0, -2.0);
            glVertex3f(-1.0, 1.0, -5.0);
            glEnd();
            break;
        case 3:
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glPushMatrix();

            glTranslatef(0.25, 0.5, 0.0);
            glRotatef(-45, 0.0, 1.0, 0.0);
            glScalef(0.25, 0.25, 1.0);

            glColor3f(0, 0, 1);
            glBegin(GL_QUADS);
            glVertex2f(-1.0, -1.0);
            glVertex2f(1.0, -1.0);
            glVertex2f(1.0, 1.0);
            glVertex2f(-1.0, 1.0);
            glEnd();

            glPopMatrix();

            glColor3f(1, 0, 0);
            glBegin(GL_TRIANGLES);
            glVertex3f(0.5, 0.25, 0.5);
            glVertex3f(0.75, 0.25, -0.5);
            glVertex3f(0.75, 0.75, -0.5);
            glEnd();
            break;
        case 4:
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glColor3f(0, 0, 1);
            glBegin(GL_QUADS);
            glVertex2f(0.4, 0.2);
            glVertex2f(2.0, 0.2);
            glVertex2f(2.0, 0.8);
            glVertex2f(0.4, 0.8);
            glEnd();

            glColor3f(1, 0, 0);
            glBegin(GL_TRIANGLES);
            glVertex3f(0.2, 0.2, -0.5);
            glVertex3f(0.8, 0.5, 0.5);
            glVertex3f(0.2, 0.8, -0.5);
            glEnd();
            break;
        case 5:
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glTranslatef(0.0, 0.0, -5.0);
            glRotatef(-60, -1.0, 1.0, -1.0);

            for (int i = 0; i < 6; ++i) {
                GLfloat* tran = gl_trans[i];
                GLfloat* rot = gl_rots[i];
                GLfloat* col = gl_cols[i];

                glPushMatrix();

                glTranslatef(tran[0], tran[1], tran[2]);
                glRotatef(rot[0], rot[1], rot[2], rot[3]);

                glColor3f(col[0], col[1], col[2]);

                glBegin(GL_QUADS);
                glVertex2f(-1.0, -1.0);
                glVertex2f(1.0, -1.0);
                glVertex2f(1.0, 1.0);
                glVertex2f(-1.0, 1.0);
                glEnd();

                glPopMatrix();
            }

            break;
        case 6:
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glTranslatef(0.1667, 0.5, 0.0);
            glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0);
            for( int i = 0; i < 3; i++ )
            {
                glVertex2f(0 + 0.33*i, 0.25);
                glVertex2f(-0.1667 + 0.33*i, -0.25);
                if( i == 0 ) glColor3f(0, 1, 0);
                else if( i == 1) glColor3f(0, 0, 1);
                else if( i == 2) glColor3f(1, 0, 0);
                glVertex2f(0.1667 + 0.33*i, -0.25);
            }
            glEnd();
            break;
        case 7:
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glColor3f(1,1,1);

			GLuint id;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			height=0,width=0;
			textureData = readBmpFile("texture.bmp", width, height);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,GL_UNSIGNED_INT_8_8_8_8, textureData);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glEnable(GL_TEXTURE_2D);

			glBegin(GL_TRIANGLES);
			glTexCoord2f(0,0);
			glVertex2f(0.25, 0.25);
			glTexCoord2f(1,0);
			glVertex2f(0.75, 0.25);
			glTexCoord2f(1,1);
			glVertex2f(0.75, 0.75);
			glEnd();

			glDisable(GL_TEXTURE_2D);

			break;
        default:
            std::cerr<<"Invalid test number"<<std::endl;
            break;
    }
}

static MGLfloat mgl_trans[6][3] = {
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0},
    {1.0, 0.0, 0.0},
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, -1.0, 0.0}
};
static MGLfloat mgl_rots[6][4] = {
    {0.0, 0.0, 1.0, 0.0},
    {180.0, 0.0, 1.0, 0.0},
    {90.0, 0.0, 1.0, 0.0},
    {-90.0, 0.0, 1.0, 0.0},
    {-90.0, 1.0, 0.0, 0.0},
    {90.0, 1.0, 0.0, 0.0}
};
static MGLbyte mgl_cols[6][3] = {
    {255, 0, 0},
    {0, 255, 255},
    {0, 255, 0},
    {255, 0, 255},
    {0, 0, 255},
    {255, 255, 0}
};

void mglTests(const int test_number)
{
	int height = 0, width = 0;
	unsigned int* textureData;
    switch(test_number)
    {
        case 1:
            mglMatrixMode(MGL_PROJECTION);
            mglLoadIdentity();
            mglOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            mglMatrixMode(MGL_MODELVIEW);
            mglLoadIdentity();

            mglColor(255, 255, 255);
            mglBegin(MGL_TRIANGLES);
            mglVertex2(0.25, 0.25);
            mglVertex2(0.75, 0.25);
            mglVertex2(0.75, 0.75);
            mglEnd();
            break;
        case 2:
            mglMatrixMode(MGL_PROJECTION);
            mglLoadIdentity();
            mglFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
            mglMatrixMode(MGL_MODELVIEW);
            mglLoadIdentity();

            mglColor(255, 255, 255);
            mglBegin(MGL_QUADS);
            mglVertex3(-1.0, -1.0, -5.0);
            mglVertex3(1.0, -1.0, -2.0);
            mglVertex3(1.0, 1.0, -2.0);
            mglVertex3(-1.0, 1.0, -5.0);
            mglEnd();
            break;
        case 3:
            mglMatrixMode(MGL_PROJECTION);
            mglLoadIdentity();
            mglOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            mglMatrixMode(MGL_MODELVIEW);
            mglLoadIdentity();

            mglPushMatrix();

            mglTranslate(0.25, 0.5, 0.0);
            mglRotate(-45, 0.0, 1.0, 0.0);
            mglScale(0.25, 0.25, 1.0);

            mglColor(0, 0, 255);
            mglBegin(MGL_QUADS);
            mglVertex2(-1.0, -1.0);
            mglVertex2(1.0, -1.0);
            mglVertex2(1.0, 1.0);
            mglVertex2(-1.0, 1.0);
            mglEnd();

            mglPopMatrix();

            mglColor(255, 0, 0);
            mglBegin(MGL_TRIANGLES);
            mglVertex3(0.5, 0.25, 0.5);
            mglVertex3(0.75, 0.25, -0.5);
            mglVertex3(0.75, 0.75, -0.5);
            mglEnd();
            break;
        case 4:
            mglMatrixMode(MGL_PROJECTION);
            mglLoadIdentity();
            mglOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            mglMatrixMode(MGL_MODELVIEW);
            mglLoadIdentity();

            mglColor(0, 0, 255);
            mglBegin(MGL_QUADS);
            mglVertex2(0.4, 0.2);
            mglVertex2(2.0, 0.2);
            mglVertex2(2.0, 0.8);
            mglVertex2(0.4, 0.8);
            mglEnd();

            mglColor(255, 0, 0);
            mglBegin(MGL_TRIANGLES);
            mglVertex3(0.2, 0.2, -0.5);
            mglVertex3(0.8, 0.5, 0.5);
            mglVertex3(0.2, 0.8, -0.5);
            mglEnd();
            break;
        case 5:
            mglMatrixMode(MGL_PROJECTION);
            mglLoadIdentity();
            mglFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
            mglMatrixMode(MGL_MODELVIEW);
            mglLoadIdentity();

            mglTranslate(0.0, 0.0, -5.0);
            mglRotate(-60, -1.0, 1.0, -1.0);

            for (int i = 0; i < 6; ++i) {
                MGLfloat* tran = mgl_trans[i];
                MGLfloat* rot = mgl_rots[i];
                MGLbyte* col = mgl_cols[i];

                mglPushMatrix();

                mglTranslate(tran[0], tran[1], tran[2]);
                mglRotate(rot[0], rot[1], rot[2], rot[3]);

                mglColor(col[0], col[1], col[2]);
                mglBegin(MGL_QUADS);
                mglVertex2(-1.0, -1.0);
                mglVertex2(1.0, -1.0);
                mglVertex2(1.0, 1.0);
                mglVertex2(-1.0, 1.0);
                mglEnd();

                mglPopMatrix();
            }
            break;
        case 6:
            mglMatrixMode(MGL_PROJECTION);
            mglLoadIdentity();
            mglOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
            mglMatrixMode(MGL_MODELVIEW);
            mglLoadIdentity();

            mglTranslate(0.1667, 0.5, 0.0);
            mglBegin(MGL_TRIANGLES);
            mglColor(255, 0, 0);
            for( int i = 0; i < 3; i++ )
            {
                mglVertex2(0 + 0.33*i, 0.25);
                mglVertex2(-0.1667 + 0.33*i, -0.25);
                if( i == 0 ) mglColor(0, 255, 0);
                else if( i == 1) mglColor(0, 0, 255);
                else if( i == 2) mglColor(255, 0, 0);
                mglVertex2(0.1667 + 0.33*i, -0.25);
            }
            mglEnd();
            break;
        case 7:
			mglMatrixMode(MGL_PROJECTION);
			mglLoadIdentity();
			mglOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
			mglMatrixMode(MGL_MODELVIEW);
			mglLoadIdentity();

			mglColor(255, 255, 255);

			/**
			 * Did not get time to implement commented functions.
			 * The following two calls are made to specify the number of textures and
			 * bind a named texture to a texturing target.
			 */
//			GLuint id;
//			glGenTextures(1, &id);
//			glBindTexture(GL_TEXTURE_2D, id);
			textureData=readBmpFile("texture.bmp", width, height);
			mglTexImage2D(MGL_TEXTURE_2D, width, height, textureData);
			/**
			 * The texture minifying/magnification functions are used
			 * whenever the pixel being textured maps to an area greater/less than one texture element.
			 */
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			mglEnable(MGL_TEXTURE_2D);

			mglColor(255, 255, 255);
			mglBegin(MGL_TRIANGLES);
			mglTexCoord2f(0,0);
			mglVertex2(0.25, 0.25);
			mglTexCoord2f(1,0);
			mglVertex2(0.75, 0.25);
			mglTexCoord2f(1,1);
			mglVertex2(0.75, 0.75);
			mglEnd();

			mglDisable(MGL_TEXTURE_2D);

			break;
        default:
            std::cerr<<"Invalid test number"<<std::endl;
            break;
    }
}
