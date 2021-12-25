#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;


	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}


	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;

//����������� ����� ������ ��������
void initRender(OpenGL* ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);


	//������ ����������� ���������  (R G B)
	RGBTRIPLE* texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH);


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





void Render(OpenGL* ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//��������������
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  

	//������ ��������� ������

	static bool flag = 0;
	/*if (flag)
		goto flag2;*/
	flag++;
	double niz[8][3] = { {-4,6,0}, {-1,2,0}, {6,5,0}, {5,0,0}, {1,-1,0}, {0,-6,0}, {-6,-4,0}, {-2,1,0} };
	double verh[8][3] = { {-4,6,2}, {-1,2,2}, {6,5,2}, {5,0,2}, {1,-1,2}, {0,-6,2}, {-6,-4,2}, {-2,1,2} };
	double nizOkr[15][3];
	double verhOkr[15][3];
	double nizOkrCenter[] = { -3,-5,0 };
	double verhOkrCenter[] = { -3,-5,2 };
	double x = -6;
	// ���������� ���������� ����� �����������
	double D1 = 100 - 4 * -(10 - (x + 3) * (x + 3) - 25);
	double y1 = (-10 + sqrt(D1)) / 2;
	nizOkr[0][0] = x;
	nizOkr[0][1] = y1;
	nizOkr[0][2] = 0;
	verhOkr[0][0] = x;
	verhOkr[0][1] = y1;
	verhOkr[0][2] = 2;

	double x1 = -3 - sqrt(10);
	D1 = 0; // 100 - 4 * -(10 - (x1 + 3) * (x1 + 3) - 25);
	y1 = (-10 + sqrt(D1)) / 2;
	nizOkr[1][0] = x1;
	nizOkr[1][1] = y1;
	nizOkr[1][2] = 0;
	verhOkr[1][0] = x1;
	verhOkr[1][1] = y1;
	verhOkr[1][2] = 2;
	for (int i = 2; i < 15; i++) {
		double D = 100 - 4 * -(10 - (x + 3) * (x + 3) - 25);
		double y = (-10 - sqrt(D)) / 2;
		nizOkr[i][0] = x;
		nizOkr[i][1] = y;
		nizOkr[i][2] = 0;
		verhOkr[i][0] = x;
		verhOkr[i][1] = y;
		verhOkr[i][2] = 2;
		x += 0.5;
	}

	double vectorA[3];
	double vectorB[3];
	double vectorN[3];
	double vectorModule;

	vectorA[0] = verh[1][0] - verh[0][0];
	vectorA[1] = verh[1][1] - verh[0][1];
	vectorA[2] = verh[1][2] - verh[0][2];
	vectorB[0] = verh[7][0] - verh[0][0];
	vectorB[1] = verh[7][1] - verh[0][1];
	vectorB[2] = verh[7][2] - verh[0][2];
	vectorN[0] = vectorA[1] * vectorB[2] - vectorB[1] * vectorA[2];
	vectorN[1] = -vectorA[0] * vectorB[2] + vectorB[0] * vectorA[2];
	vectorN[2] = vectorA[0] * vectorB[1] - vectorB[0] * vectorA[1];
	vectorModule = sqrt(vectorN[0] * vectorN[0] + vectorN[1] * vectorN[1] + vectorN[2] * vectorN[2]);
	vectorN[0] /= vectorModule;
	vectorN[1] /= vectorModule;
	vectorN[2] /= vectorModule;

	// ����������� ���
	glColor3d(1, 0.1, 0.1);
	glNormal3d(vectorN[0], vectorN[1], vectorN[2]);
	glBegin(GL_TRIANGLES);
	glVertex3dv(&niz[0][0]);
	glVertex3dv(&niz[1][0]);
	glVertex3dv(&niz[7][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&niz[1][0]);
	glVertex3dv(&niz[2][0]);
	glVertex3dv(&niz[4][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&niz[2][0]);
	glVertex3dv(&niz[3][0]);
	glVertex3dv(&niz[4][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&niz[4][0]);
	glVertex3dv(&niz[1][0]);
	glVertex3dv(&niz[7][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&niz[4][0]);
	glVertex3dv(&niz[5][0]);
	glVertex3dv(&niz[7][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&niz[5][0]);
	glVertex3dv(&niz[6][0]);
	glVertex3dv(&niz[7][0]);
	glEnd();

	double vectorN2[3] = { vectorN[0], vectorN[1], vectorN[2] };;
	double vectorModule2 = vectorModule;
	// ����������� ��������
	glColor3d(0, 1, 1);
	for (int i = 0; i < 8; i++) {
		if (i == 7) {
			vectorA[0] = verh[i][0] - verh[0][0];
			vectorA[1] = verh[i][1] - verh[0][1];
			vectorA[2] = verh[i][2] - verh[0][2];
			vectorB[0] = niz[0][0] - verh[0][0];
			vectorB[1] = niz[0][1] - verh[0][1];
			vectorB[2] = niz[0][2] - verh[0][2];
			vectorN[0] = vectorA[1] * vectorB[2] - vectorB[1] * vectorA[2];
			vectorN[1] = -vectorA[0] * vectorB[2] + vectorB[0] * vectorA[2];
			vectorN[2] = vectorA[0] * vectorB[1] - vectorB[0] * vectorA[1];
			vectorModule = sqrt(vectorN[0] * vectorN[0] + vectorN[1] * vectorN[1] + vectorN[2] * vectorN[2]);
			vectorN[0] /= vectorModule;
			vectorN[1] /= vectorModule;
			vectorN[2] /= vectorModule;
			glNormal3d(-vectorN[0], -vectorN[1], -vectorN[2]);
			glBegin(GL_QUADS);
			glVertex3dv(&verh[i][0]);
			glVertex3dv(&verh[0][0]);
			glVertex3dv(&niz[0][0]);
			glVertex3dv(&niz[i][0]);
			glEnd();
			break;
		}
		if (i == 5) {
			continue;
		}
		vectorA[0] = verh[i][0] - verh[i + 1][0];
		vectorA[1] = verh[i][1] - verh[i + 1][1];
		vectorA[2] = verh[i][2] - verh[i + 1][2];
		vectorB[0] = niz[i + 1][0] - verh[i + 1][0];
		vectorB[1] = niz[i + 1][1] - verh[i + 1][1];
		vectorB[2] = niz[i + 1][2] - verh[i + 1][2];
		vectorN[0] = vectorA[1] * vectorB[2] - vectorB[1] * vectorA[2];
		vectorN[1] = -vectorA[0] * vectorB[2] + vectorB[0] * vectorA[2];
		vectorN[2] = vectorA[0] * vectorB[1] - vectorB[0] * vectorA[1];
		vectorModule = sqrt(vectorN[0] * vectorN[0] + vectorN[1] * vectorN[1] + vectorN[2] * vectorN[2]);
		vectorN[0] /= vectorModule;
		vectorN[1] /= vectorModule;
		vectorN[2] /= vectorModule;
		glNormal3d(-vectorN[0], -vectorN[1], -vectorN[2]);
		glBegin(GL_QUADS);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexCoord2d(0, 0);
		glVertex3dv(&verh[i][0]);
		glTexCoord2d(0, 1);
		glVertex3dv(&verh[i + 1][0]);
		glTexCoord2d(1, 1);
		glVertex3dv(&niz[i + 1][0]);
		glTexCoord2d(1, 0);
		glVertex3dv(&niz[i][0]);
		glEnd();
	}
	for (int i = 0; i < 15; i++) {
		if (i == 14) {
			break;
		}
		vectorA[0] = verhOkr[i][0] - verhOkr[i + 1][0];
		vectorA[1] = verhOkr[i][1] - verhOkr[i + 1][1];
		vectorA[2] = verhOkr[i][2] - verhOkr[i + 1][2];
		vectorB[0] = nizOkr[i + 1][0] - verhOkr[i + 1][0];
		vectorB[1] = nizOkr[i + 1][1] - verhOkr[i + 1][1];
		vectorB[2] = nizOkr[i + 1][2] - verhOkr[i + 1][2];
		vectorN[0] = vectorA[1] * vectorB[2] - vectorB[1] * vectorA[2];
		vectorN[1] = -vectorA[0] * vectorB[2] + vectorB[0] * vectorA[2];
		vectorN[2] = vectorA[0] * vectorB[1] - vectorB[0] * vectorA[1];
		vectorModule = sqrt(vectorN[0] * vectorN[0] + vectorN[1] * vectorN[1] + vectorN[2] * vectorN[2]);
		vectorN[0] /= vectorModule;
		vectorN[1] /= vectorModule;
		vectorN[2] /= vectorModule;
		glNormal3d(vectorN[0], vectorN[1], vectorN[2]);
		glBegin(GL_QUADS);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexCoord2d(0, 0);
		glVertex3dv(&verhOkr[i][0]);
		glTexCoord2d(0, 1);
		glVertex3dv(&verhOkr[i + 1][0]);
		glTexCoord2d(1, 1);
		glVertex3dv(&nizOkr[i + 1][0]);
		glTexCoord2d(1, 0);
		glVertex3dv(&nizOkr[i][0]);
		glEnd();
	}
	// ����������� ����
	memcpy(vectorN, vectorN2, sizeof vectorN);
	vectorModule = vectorModule2;

	glColor4d(1, 1, 0.1, 0);
	glNormal3d(-vectorN[0], -vectorN[1], -vectorN[2]);
	glBegin(GL_TRIANGLES);
	glVertex3dv(&verh[0][0]);
	glVertex3dv(&verh[1][0]);
	glVertex3dv(&verh[7][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&verh[1][0]);
	glVertex3dv(&verh[2][0]);
	glVertex3dv(&verh[4][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&verh[2][0]);
	glVertex3dv(&verh[3][0]);
	glVertex3dv(&verh[4][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&verh[4][0]);
	glVertex3dv(&verh[1][0]);
	glVertex3dv(&verh[7][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&verh[4][0]);
	glVertex3dv(&verh[5][0]);
	glVertex3dv(&verh[7][0]);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(&verh[5][0]);
	glVertex3dv(&verh[6][0]);
	glVertex3dv(&verh[7][0]);
	glEnd();

	glColor3d(0.5, 1, 0.5);
	glNormal3d(vectorN[0], vectorN[1], vectorN[2]);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3dv(nizOkrCenter);
	for (int i = 0; i < 15; i++) {
		glVertex3dv(&nizOkr[i][0]);
	}
	glEnd();
	glColor4d(0.5, 1, 0.5, 0.3);
	glNormal3d(-vectorN[0], -vectorN[1], -vectorN[2]);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3dv(verhOkrCenter);
	for (int i = 0; i < 15; i++) {
		glVertex3dv(&verhOkr[i][0]);
	}
	glEnd();
	//����� ��������� ���������� ��������


   //��������� ������ ������


	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
									//(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}