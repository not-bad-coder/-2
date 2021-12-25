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

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}


	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE* texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

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


	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	//Начало рисования фигуры

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
	// Высчитываю координаты точек окружностей
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

	// Отрисовываю низ
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
	// Отрисовываю боковину
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
	// Отрисовываю верх
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
	//конец рисования квадратика станкина


   //Сообщение вверху экрана


	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
									//(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}