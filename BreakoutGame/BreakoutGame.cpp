#include <windows.h>
#include <gl/GL.h>
#include <gl/glu.h>
#include "glut.h"
#include <time.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string>

/* timer to indicate how long the ball has been shot */
clock_t shootTime;
/* score to indicate how much object broke */
int score = 0;
/* whether to fill the polygons or just draw outline */
bool isFilled = true;
/* menu entries */
enum{
	MENU_POLYGON = 1,
	MENU_FILLED,
	MENU_WIRE,
	MENU_RESET,
	MENU_QUIT

};
/* simple sleeep function */
void sleep(unsigned int mseconds){
	clock_t upper_1 = mseconds + clock();
	while (upper_1 > clock());
}
/* point class */
template <class myType>
class Point{
public:
	Point(){x = y = 0;}
	Point(myType _x, myType _y):x(_x), y(_y){}
	void setPoint(myType _x, myType _y){ x = _x; y = _y;}
	
	Point& operator=(const Point& rhs){
		if(this != &rhs){
			x = rhs.x;
			y = rhs.y;
		}
		return *this;
	}

	myType x, y;
	
};

typedef Point<int> intPoint;
typedef Point<float> floatPoint;
typedef Point<double> doublePoint;

/* angle class */
class Angle{
public:
	Angle(){angle = 0.0;}
	Angle(double _angle):angle(_angle){}

	double DEG2RAD(){
		return(angle*M_PI/180.0);
	}
	void setAngle(double _angle){
		angle = _angle;
	}
	double getAngle(){return angle;}

	void moveAngle(double _angle){
		angle += _angle;
	}
	Angle& operator=(const Angle& rhs){
		if(this != &rhs){
			angle = rhs.angle;
		}
		return *this;
	}
private:
	double angle;
};

/* class that maintains the viewports information */
class Viewport{
public:
	Viewport(){
		setViewport(0,0,600, 600);
	}
	Viewport(float x, float y, float _width, float _height):width(_width), height(_height){
		corner.setPoint(x, y);
		x_min = corner.x;
		x_max = corner.x + width;
		y_min = corner.y;
		y_max = corner.y + height;
	}
	void setViewport(float x, float y, float _width, float _height){
		width = _width; height = _height;
		corner.setPoint(x, y);
		x_min = corner.x;
		x_max = corner.x + width;
		y_min = corner.y;
		y_max = corner.y + height;
	}
	floatPoint corner;
	float x_min, x_max, y_min, y_max;
	float width, height;
};
/* class to maintain ortho space information */
class Space{
public:
	Space(){
		init(1024, 680);
	}
	Space(int _w, int _h){
		init(_w, _h);
	}
	
	void resize(int w, int h){
		window_width = w; window_height = h;
		width = window_width * dx; height = window_height * dy;
		x_min = -width/2.0; x_max = width/2.0; y_min = -height/2.0; y_max = height/2.0;
		scoreView.setViewport(0, 0, window_width, 0.1*window_height);
		mainView.setViewport(0, 0.1*window_height, window_width, 0.9*window_height);
		resetViewports();
	}
	/* height and width of the actual window */
	int window_width, window_height;
	/* hiehgt and width of the ortho space */
	float width, height;
	/* as said */
	float x_min, x_max, y_min, y_max;
	/* the ratio of the ortho space width and height over the actual window width and height */
	float dx, dy;
	float d;
	floatPoint center;
	Viewport mainView;
	Viewport scoreView;
private:
	void init(int _w, int _h){
		window_width = _w; window_height = _h;
		d = dx = dy = 1;
		width = window_width * dx; height = window_height * dy;
		center.setPoint(0.0, 0.0);
		x_min = -width/2.0; x_max = width/2.0; y_min = -height/2.0; y_max = height/2.0;
		scoreView.setViewport(0, 0, window_width, 0.1*window_height);
		mainView.setViewport(0, 0.1*window_height, window_width, 0.9*window_height);
		resetViewports();
	}
	void resetViewports(){
		mainView.x_min = x_min;
		mainView.y_min = y_min + scoreView.height * dy;
		mainView.x_max = x_min + mainView.width * dx;
		mainView.y_max = y_max;

		scoreView.x_min = x_min;
		scoreView.y_min = y_min;
		scoreView.x_max = x_min + scoreView.width * dx;
		scoreView.y_max = y_min + scoreView.height * dy;
	}
};

Space space;

/* square class */
class Square{
public:
	Square(){bL.x = bL.y = bR.x = bR.y = tR.x = tR.y = tL.x = tL.y = 0.0; proportion.setPoint(0,0); isHit = false;}
	Square(float blx, float bly, float bRx, float bRy, float tRx, float tRy, float tLx, float tLy, Space s){
		bL.x = blx; bL.y = bly; bR.x = bRx; bR.y = bRy; tR.x = tRx; tR.y = tRy; tL.x = tLx; tL.y = tLy;
		center = calculateCenter();
		setProportion(s);
		isHit = false;
	}
	/* constructor with center point */
	Square(float x, float y, float _width, float _height, Space s){
		width = _width; height = _height;
		center.setPoint(x, y);
		calculatePoints(center);
		setProportion(s);
		isHit = false;
	}

	void setProportion(Space s){
		proportion.setPoint(center.x/s.x_max, center.y/s.y_max);
	}
	
	void setSquare(float x, float y, float _width, float _height, Space s){
		width = _width; height = _height;
		center.setPoint(x, y);
		calculatePoints(center);
		setProportion(s);
	}
	void setSquare(float blx, float bly, float bRx, float bRy, float tRx, float tRy, float tLx, float tLy){
		bL.x = blx; bL.y = bly; bR.x = bRx; bR.y = bRy; tR.x = tRx; tR.y = tRy; tL.x = tLx; tL.y = tLy;
		center = calculateCenter();
	}
	floatPoint calculateCenter(){
		floatPoint c;
		floatPoint bottomCenter( (bL.x + bR.x)/2.0, (bL.y + bR.y)/2.0 );
		floatPoint topCenter( (tL.x + tR.x)/2.0, (tL.y + tR.y)/2.0 );
		c.x = (bottomCenter.x + topCenter.x)/2.0;
		c.y = (bottomCenter.y + topCenter.y)/2.0;
		return c;
	}
	/* calculate points of square based on the center point*/
	void calculatePoints(floatPoint center){
		bL.x = center.x - width/2.0;
		bL.y = center.y - height/2.0;
		bR.x = center.x + width/2.0;
		bR.y = center.y - height/2.0;
		tR.x = center.x + width/2.0;
		tR.y = center.y + height/2.0;
		tL.x = center.x - width/2.0;
		tL.y = center.y + height/2.0; 
	}
	void reshape(Space s){
		center.setPoint( proportion.x * s.x_max, proportion.y * s.y_max);
		calculatePoints(center);
	}
	Square& operator= (const Square& rhs){
		if(this != &rhs){
			bL = rhs.bL;
			bR = rhs.bR;
			tR = rhs.tR;
			tL = rhs.tL;
			center = rhs.center;
			proportion = rhs.proportion;
		}
		return *this;
	}
	float width, height;
	floatPoint bL, bR, tR, tL;
	floatPoint center;
	floatPoint proportion;
	bool isHit;
};
/* bounding box class */
class BBox : public Square{
public:
	BBox() : Square(){
	}
};

/* paddle class */
class Paddle : public Square{
public:
	Paddle() : Square(){}
	Paddle(float blx, float bly, float bRx, float bRy, float tRx, float tRy, float tLx, float tLy, Space s) :
	Square(blx, bly, bRx, bRy, tRx, tRy, tLx, tLy, s){angle.setAngle(0);}
	Paddle(float x, float y, float width, float height, Space s) : Square(x, y, width, height, s){
		angle.setAngle(0);
	}
	void setPaddle(float blx, float bly, float bRx, float bRy, float tRx, float tRy, float tLx, float tLy){
		setSquare(blx, bly, bRx, bRy, tRx, tRy, tLx, tLy);
	}
	void movePaddle(float x, float y, Space s){
		if(center.x <= s.x_min + 50*s.dx && x <0)
			return;
		else if(center.x >= s.x_max-50*s.dx && x > 0)
			return;
		bL.x += x; bL.y += y;
		bR.x += x; bR.y += y;
		tL.x += x; tL.y += y;
		tR.x += x; tR.y += y;
		center = calculateCenter();
		setProportion(s);
	}
	void translatePaddle(float x, float y){
		bL.x += x; bL.y += y;
		bR.x += x; bR.y += y;
		tL.x += x; tL.y += y;
		tR.x += x; tR.y += y;
	}

	/* hard coded rotation proceduare for paddle */
	void rotatePaddle(float _theta){
		if (angle.getAngle() >= 80 && _theta > 0)
			return;
		else if(angle.getAngle() <= -80 && _theta < 0)
			return;
		angle.moveAngle(_theta);
		double theta = _theta * M_PI / 180.0;
		float x, y;
		x = bR.x* cos(theta) - bR.y * sin(theta);
		y = bR.x*sin(theta) + bR.y * cos(theta);
		bR.x = x; bR.y = y;
		
		x = bL.x* cos(theta) - bL.y * sin(theta);
		y = bL.x*sin(theta) + bL.y * cos(theta);
		bL.x = x; bL.y = y;

		x = tL.x* cos(theta) - tL.y * sin(theta);
		y = tL.x*sin(theta) + tL.y * cos(theta);
		tL.x = x; tL.y = y;

		x = tR.x* cos(theta) - tR.y * sin(theta);
		y = tR.x*sin(theta) + tR.y * cos(theta);
		tR.x = x; tR.y = y;
	}
	
	/* reset the angle of paddle to 0 after the ball is shot */
	void resetPaddleAngle(){
		angle.setAngle(0);
		calculatePoints(center);
	}
	/* adjust paddle when screen resizes */
	void reshape(Space s){
		center.setPoint( proportion.x * s.x_max, proportion.y * s.y_max);
		calculatePoints(center);
		translatePaddle(-center.x, -center.y);
		float x, y;
		x = bR.x* cos(angle.DEG2RAD()) - bR.y * sin(angle.DEG2RAD());
		y = bR.x*sin(angle.DEG2RAD()) + bR.y * cos(angle.DEG2RAD());
		bR.x = x; bR.y = y;
		
		x = bL.x* cos(angle.DEG2RAD()) - bL.y * sin(angle.DEG2RAD());
		y = bL.x*sin(angle.DEG2RAD()) + bL.y * cos(angle.DEG2RAD());
		bL.x = x; bL.y = y;

		x = tL.x* cos(angle.DEG2RAD()) - tL.y * sin(angle.DEG2RAD());
		y = tL.x*sin(angle.DEG2RAD()) + tL.y * cos(angle.DEG2RAD());
		tL.x = x; tL.y = y;

		x = tR.x* cos(angle.DEG2RAD()) - tR.y * sin(angle.DEG2RAD());
		y = tR.x*sin(angle.DEG2RAD()) + tR.y * cos(angle.DEG2RAD());
		tR.x = x; tR.y = y;
		translatePaddle(center.x, center.y);
	}
	Angle angle;
};

/* circle class */
class Circle{
public:
	Circle(){radius = 0.0; center.x = 0; center.y = 0; proportion.setPoint(0,0);}
	Circle(float _x, float _y, float _radius, Space s){
		center.setPoint(_x, _y);
		radius = _radius;
		setProportion(s);
		bBox.setSquare(_x, _y, _radius*2.0, _radius*2.0, s);
	}
	
	void setCircle(float _x, float _y, float _radius, Space s){
		center.setPoint(_x, _y);
		radius = _radius;
		setProportion(s);
		bBox.setSquare(_x, _y, _radius*2.0, _radius*2.0, s);
	}

	void setProportion(Space s){
		proportion.setPoint(center.x/s.x_max, center.y/s.y_max);
	}

	void reshape(Space s){
		center.setPoint(proportion.x * s.x_max, proportion.y * s.y_max);
		bBox.reshape(s);
	}
	float radius;
	floatPoint center;
	floatPoint proportion;
	BBox bBox;
};

/* the moving ball object */
class Ball : public Circle{
public:
	Ball() : Circle(){isShot = false; speed = 5*space.d;}
	Ball(float _x, float _y, float _radius, Space s) : Circle(_x, _y, _radius, s){
		isShot = false; 
		speed = 5*space.d;
	}
	
	/* initialize the ball position based on the paddle position */
	void initialize(Paddle paddle, Space s){
		ang = paddle.angle;
		if(ang.getAngle() < 0){
			ang.setAngle(360.0 + ang.getAngle());
		}
		floatPoint tangentPoint( (paddle.tL.x + paddle.tR.x)/2.0, (paddle.tL.y + paddle.tR.y)/2.0 );
		center.setPoint( (tangentPoint.x - radius * sin(ang.DEG2RAD())), (tangentPoint.y + radius * cos(ang.DEG2RAD())) );
		/*recalculated the angle of the ball based on the paddle angle*/
		if(paddle.angle.getAngle() != 0){
			ang.setAngle(90.0 + paddle.angle.getAngle());
		}
		else{
			ang.setAngle(90.0);
		}
		calculateDxDy();
		setProportion(s);
	}

	/* calculate velocity based on angle and speed */
	void calculateDxDy(){
		dx = speed * cos(ang.DEG2RAD());
		dy = speed * sin(ang.DEG2RAD());
	}

	void moveBall(Space s){
		center.x += dx;
		center.y += dy;
		setProportion(s);
	}

	/* detect the collision with the all objects */
	void detectCollision(Square& p, Space s, bool isPaddle){
		if(p.isHit)
			return;
		/* left side collision */
		if(center.x + radius >= p.tL.x && center.x < p.tL.x && center.y <= p.tL.y && center.y >= p.bL.y){
			ang.setAngle(180.0 - ang.getAngle());
			if(ang.getAngle() < 0){
				ang.setAngle(360.0 + ang.getAngle());
			}
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* bottom side collision */
		else if(center.y + radius >= p.bL.y && center.y < p.bL.y && center.x >= p.bL.x && center.x <= p.bR.x){
			ang.setAngle(360.0 - ang.getAngle());
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* right side collision */
		else if(center.x - radius <= p.tR.x && center.x > p.tR.x && center.y <= p.tR.y && center.y >= p.bR.y){
			ang.setAngle(180.0 - ang.getAngle());
			if(ang.getAngle() < 0){
				ang.setAngle(360.0 + ang.getAngle());
			}
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* top side collision */
		else if(center.y - radius <= p.tL.y && center.y > p.tL.y && center.x >= p.tL.x && center.x <= p.tR.x){
			ang.setAngle(360.0 - ang.getAngle());
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* bottom left corner */
		else if(center.y + radius >= p.bL.y && center.y < p.bL.y && center.x + radius >= p.bL.x && center.x < p.bL.x){
			ang.setAngle(180.0 + ang.getAngle());
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* bottom right corner */
		else if(center.y + radius >= p.bR.y && center.y < p.bR.y && center.x - radius <= p.bR.x && center.x > p.bR.x){
			ang.setAngle(180.0 + ang.getAngle());
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* top right corner */
		else if(center.y - radius <= p.tR.y && center.y > p.tR.y && center.x - radius <= p.tR.x && center.x > p.tR.x){
			ang.setAngle(ang.getAngle() - 180.0);
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}
		/* top left corner */
		else if(center.y - radius <= p.tL.y && center.y > p.tL.y && center.x + radius >= p.tL.x && center.x < p.tL.x){
			ang.setAngle(ang.getAngle() - 180.0);
			if(!isPaddle){
				p.isHit = true;
				++score;
			}
		}

		calculateDxDy();
	}

	/* detect the collision with the boundaries */
	void detectWallCollision(Space space){
		/* left wall collision */
		if(center.x - radius <= space.mainView.x_min && center.y + radius < space.mainView.y_max && center.y - radius > space.mainView.y_min){
			ang.setAngle(180.0 - ang.getAngle());
			if(ang.getAngle() < 0){
				ang.setAngle(360.0 + ang.getAngle());
			}
		}
		/* bottom wall collision */
		else if(center.y - radius <= space.mainView.y_min && center.x + radius < space.mainView.x_max && center.x - radius > space.mainView.x_min){
			ang.setAngle(360.0 - ang.getAngle());
		}
		/* right wall collision */
		else if(center.x + radius >= space.mainView.x_max && center.y + radius < space.mainView.y_max && center.y - radius > space.mainView.y_min){
			ang.setAngle(180.0 - ang.getAngle());
			if(ang.getAngle() < 0){
				ang.setAngle(360.0 + ang.getAngle());
			}
		}
		/* top wall collision */
		else if(center.y + radius >= space.mainView.y_max && center.x + radius < space.mainView.x_max && center.x - radius > space.mainView.x_min){
			ang.setAngle(360.0 - ang.getAngle());
		}
		/*top left corner*/
		else if(center.x - radius <= space.mainView.x_min && center.y + radius >= space.mainView.y_max){
			ang.setAngle(180.0 + ang.getAngle());
		}
		/* bottom left corner */
		else if(center.x - radius <= space.mainView.x_max && center.y - radius <= space.mainView.y_min){
			ang.setAngle(ang.getAngle() - 180.0);
		}
		/* bottom right corner */
		else if(center.x + radius >= space.mainView.x_max && center.y - radius <= space.mainView.y_min){
			ang.setAngle(ang.getAngle() - 180.0);
		}
		/* top right corner */
		else if(center.x + radius > space.mainView.x_max && center.y + radius >= space.mainView.y_max){
			ang.setAngle(ang.getAngle() + 180.0);
		}
		calculateDxDy();
	}

	/* vector components of velocity*/
	float dx, dy;
	/* speed of the ball */
	float speed;
	Angle ang;
	bool isShot;
};

/* class to maintain information for triangles */
class Triangle{
public:
	Triangle(){}
	Triangle(Space s){
		setTriangle(0, 0, 50, s);
	}

	Triangle(float x, float y, float len, Space s){
		center.setPoint(x, y);
		length = len;
		calculatePoints();
		setProportion(s);
		bBox.setSquare(x, y, len, len, s);
	}

	void setTriangle(float x, float y, float len, Space s){
		center.setPoint(x, y);
		length = len;
		calculatePoints();
		setProportion(s);
		bBox.setSquare(x, y, len, len, s);
	}
	void setProportion(Space s){
		proportion.setPoint(center.x/s.x_max, center.y/s.y_max);
	}

	void reshape(Space s){
		center.setPoint( proportion.x * s.x_max, proportion.y * s.y_max);
		calculatePoints();
	}
	void calculatePoints(){
		pT.setPoint(center.x, center.y + length/2.0);
		pL.setPoint(center.x - length/2.0, center.y - length/2.0);
		pR.setPoint(center.x + length/2.0, center.y - length/2.0);
	}

	floatPoint center;
	/* left point, top point, right point*/
	floatPoint pL, pT, pR;
	float length;
	BBox bBox;
	floatPoint proportion;
};
/* class to draw a star */
class Star{
public:
	Star(){}
	Star(float x, float y, float baseLength, float _sideLength, Space s){
		sideLength = _sideLength;
		setStar(x, y, baseLength, sideLength, s);
	}
	void setStar(float x, float y, float baseLength, float _sideLength, Space s){
		sideLength = _sideLength;
		centerSquare.setSquare(x, y, baseLength, baseLength, s);
		float l = baseLength/2.0;
		pL.setPoint(x-l-sideLength, y);
		pB.setPoint(x, y-l-sideLength);
		pR.setPoint(x+l+sideLength, y);
		pT.setPoint(x, y+l+sideLength);
		bBox.setSquare(centerSquare.center.x, centerSquare.center.y, abs(pR.x-pL.x), abs(pT.y - pB.y), s);
	}
	
	void reshape(Space s){
		centerSquare.reshape(s);
		float l = centerSquare.width/2.0;
		pL.setPoint(centerSquare.center.x-l-sideLength, centerSquare.center.y);
		pB.setPoint(centerSquare.center.x, centerSquare.center.y-l-sideLength);
		pR.setPoint(centerSquare.center.x+l+sideLength, centerSquare.center.y);
		pT.setPoint(centerSquare.center.x, centerSquare.center.y+l+sideLength);
		bBox.setSquare(centerSquare.center.x, centerSquare.center.y, abs(pR.x-pL.x), abs(pT.y - pB.y), s);
	}
	Square centerSquare;
	float sideLength;
	floatPoint pL, pB, pR, pT;
	BBox bBox;
};
/******************************
 * Declare the global objects *
 ******************************/
Square s1(0, space.y_max * 1.2/4.0, 30, 30, space);
Triangle t1(space.x_max * 1.5/4.0, space.y_max*1.2/4.0, 50, space);
Triangle t2(space);
Paddle paddle(0, space.y_min*2.5/4.0, 90*space.dx, 9*space.dy, space);
Ball ball(0.0, 0.0,9*space.d, space);
Circle circle[12];
Triangle trig[18];
const float circle_rotation_speed = -2.0;
const float triangle_rotation_speed = 2.0;
Angle circle_rotation(0.0);
Angle triangle_rotation(0.0);

Square s2(space.x_max * -2.65/4.0, space.y_max * -1.0/4.0, 30, 30, space); 
Circle miniCircle[2];
Triangle miniTrig[2];
const float miniCircle_rotation_speed = 1.0;
Angle miniCircle_rotation(0.0);
const float miniTriangle_rotation_speed = 4.0;
Angle miniTriangle_rotation(0.0);

/*six-gon*/
Circle sixgon(2.5/4.0*space.x_max, 2.5/4.0*space.y_max, 50, space);
/* star */
Star star(2.5/4.0*space.x_max, -1.5/4.0*space.y_max, 30, 50, space);
//draw the boundary 
void drawBoundary(Space s){
	glBegin(GL_LINES);
		glVertex2f(s.mainView.x_min, s.mainView.y_min*(1-0.001)); glVertex2f(s.mainView.x_max, s.mainView.y_min*(1-0.001));
	glEnd();
}
/* draw square */
void drawSquare(Square s, bool fill){
	if(s.isHit)
		return;
	if(fill){
		glBegin(GL_POLYGON);
			glVertex2f(s.bL.x, s.bL.y);
			glVertex2f(s.bR.x, s.bR.y);
			glVertex2f(s.tR.x, s.tR.y);
			glVertex2f(s.tL.x, s.tL.y);
		glEnd();
	}
	else{
		glBegin(GL_LINES);
			glVertex2f(s.bL.x, s.bL.y);	glVertex2f(s.bR.x, s.bR.y);
			glVertex2f(s.bR.x, s.bR.y); glVertex2f(s.tR.x, s.tR.y);
			glVertex2f(s.tR.x, s.tR.y); glVertex2f(s.tL.x, s.tL.y);
			glVertex2f(s.tL.x, s.tL.y); glVertex2f(s.bL.x, s.bL.y);
		glEnd();
	}
}

/* draw paddle */
void drawPaddle(Paddle p){
	glBegin(GL_POLYGON);
	glVertex2f(p.bL.x, p.bL.y);
	glVertex2f(p.bR.x, p.bR.y);
	glVertex2f(p.tR.x, p.tR.y);
	glVertex2f(p.tL.x, p.tL.y);
	glEnd();
}

/* draw triangle */
void drawTriangle(Triangle t, bool fill){
	if(t.bBox.isHit)
		return;
	if(fill){
		glBegin(GL_POLYGON);
			glVertex2f(t.pL.x, t.pL.y);
			glVertex2f(t.pT.x, t.pT.y);
			glVertex2f(t.pR.x, t.pR.y);
		glEnd();
	}
	else{
		glBegin(GL_LINES);
			glVertex2f(t.pL.x, t.pL.y); glVertex2f(t.pT.x, t.pT.y);
			glVertex2f(t.pT.x, t.pT.y); glVertex2f(t.pR.x, t.pR.y);
			glVertex2f(t.pR.x, t.pR.y); glVertex2f(t.pL.x, t.pL.y);
		glEnd();
	}
}
/* draw circle */
void drawCircle(Circle c, bool filled){
	if(c.bBox.isHit)
		return;
	if(filled){
		glBegin(GL_TRIANGLE_FAN);
			glVertex2f(c.center.x, c.center.y);
			for(int i=0; i<=360; i+=20){
				glVertex2f(c.center.x + sin(i*M_PI/180.0) * c.radius, c.center.y + cos(i*M_PI/180.0) * c.radius);
			}
		glEnd();
	}
	else{
		glBegin(GL_LINES);
			for(int i=0; i<360; i+=20){
				glVertex2f(c.center.x + sin(i*M_PI/180.0) * c.radius, c.center.y + cos(i*M_PI/180.0) * c.radius);
				glVertex2f(c.center.x + sin((i+20)*M_PI/180.0) * c.radius, c.center.y + cos((i+20)*M_PI/180.0) * c.radius);
			}
		glEnd();
	}
}

/* draw a six-gon */
void drawSixgon(Circle c, bool filled){
	if(c.bBox.isHit)
		return;
	if(filled){
		glBegin(GL_TRIANGLE_FAN);
			glVertex2f(c.center.x, c.center.y);
			for(int i=0; i<=360; i+=60){
				glVertex2f(c.center.x + sin(i*M_PI/180.0) * c.radius, c.center.y + cos(i*M_PI/180.0) * c.radius);
			}
		glEnd();
	}
	else{
		glBegin(GL_LINES);
			for(int i=0; i<360; i+=60){
				glVertex2f(c.center.x + sin(i*M_PI/180.0) * c.radius, c.center.y + cos(i*M_PI/180.0) * c.radius);
				glVertex2f(c.center.x + sin((i+60)*M_PI/180.0) * c.radius, c.center.y + cos((i+60)*M_PI/180.0) * c.radius);
			}
		glEnd();
	}
}

/* drawStar */
void drawStar(Star s, bool filled){
	if(s.bBox.isHit)
		return;
	glBegin(GL_LINES);
		glVertex2f(s.pL.x, s.pL.y); glVertex2f(s.centerSquare.bL.x, s.centerSquare.bL.y);
		glVertex2f(s.centerSquare.bL.x, s.centerSquare.bL.y); glVertex2f(s.pB.x, s.pB.y);
		glVertex2f(s.pB.x, s.pB.y); glVertex2f(s.centerSquare.bR.x, s.centerSquare.bR.y);
		glVertex2f(s.centerSquare.bR.x, s.centerSquare.bR.y); glVertex2f(s.pR.x, s.pR.y);
		glVertex2f(s.pR.x, s.pR.y); glVertex2f(s.centerSquare.tR.x, s.centerSquare.tR.y);
		glVertex2f(s.centerSquare.tR.x, s.centerSquare.tR.y); glVertex2f(s.pT.x, s.pT.y);
		glVertex2f(s.pT.x, s.pT.y); glVertex2f(s.centerSquare.tL.x, s.centerSquare.tL.y);
		glVertex2f(s.centerSquare.tL.x, s.centerSquare.tL.y); glVertex2f(s.pL.x, s.pL.y);
	glEnd();
}
/* special key funtion that controls paddle motion */
void paddleMotion(int key, int x, int y){
	switch(key){
		case GLUT_KEY_LEFT:
			paddle.movePaddle(-10*space.dx, 0, space);
			ball.detectCollision(paddle, space, true);
			break;
		case GLUT_KEY_RIGHT:
			paddle.movePaddle(10*space.dx, 0, space);
			ball.detectCollision(paddle, space, true);
			break;
		case GLUT_KEY_DOWN:
			if(!ball.isShot){
				paddle.translatePaddle(-paddle.center.x, -paddle.center.y);
				paddle.rotatePaddle(-10);
				paddle.translatePaddle(paddle.center.x, paddle.center.y);
			}
			break;
		case GLUT_KEY_UP:
			if(!ball.isShot){
				paddle.translatePaddle(-paddle.center.x, -paddle.center.y);
				paddle.rotatePaddle(10);
				paddle.translatePaddle(paddle.center.x, paddle.center.y);
			}
			break;
		
		default:
			break;
	}
	glutPostRedisplay();
}
/* normal key function*/
void keyboard(unsigned char key, int x, int y){
	switch(key){
		case ' ':
			if(!ball.isShot){
				ball.isShot = true;
				paddle.resetPaddleAngle();
				shootTime = clock();
			}
			break;
		default:
			break;
	}
}
/* controls the animation of objects */
void animation(){
	if(ball.isShot){
		ball.detectWallCollision(space);
		if(clock() - shootTime > 500)
			ball.detectCollision(paddle, space, true);
		ball.detectCollision(s1, space, false);
		ball.detectCollision(s2, space, false);
		for(int i = 0; i<12; i++){
			ball.detectCollision(circle[i].bBox, space, false);
		}
		for(int i = 0; i <18; i++){
			ball.detectCollision(trig[i].bBox, space, false);
		}
		for(int i=0; i<2; i++){
			ball.detectCollision(miniCircle[i].bBox, space, false);
			ball.detectCollision(miniTrig[i].bBox, space, false);
		}
		ball.detectCollision(sixgon.bBox, space, false);
		ball.detectCollision(star.bBox, space, false);
		ball.moveBall(space);
	}
	/* rotate the wall of circles */
	circle_rotation.moveAngle(circle_rotation_speed);
	if (circle_rotation.getAngle() < 0){
		circle_rotation.setAngle(circle_rotation.getAngle() + 360.0);
	}
	else if(circle_rotation.getAngle() >= 360){
		circle_rotation.setAngle(circle_rotation.getAngle() - 360.0);
	}

	/* rotate the wall of triangles */
	triangle_rotation.moveAngle(triangle_rotation_speed);
	if (triangle_rotation.getAngle() < 0){
		triangle_rotation.setAngle(triangle_rotation.getAngle() + 360.0);
	}
	else if(circle_rotation.getAngle() >= 360){
		triangle_rotation.setAngle(triangle_rotation.getAngle() - 360.0);
	}

	/* rotate the circles in the mini right figure */
	miniCircle_rotation.moveAngle(miniCircle_rotation_speed);
	if (miniCircle_rotation.getAngle() < 0){
		miniCircle_rotation.setAngle(miniCircle_rotation.getAngle() + 360.0);
	}
	else if(miniCircle_rotation.getAngle() >= 360){
		miniCircle_rotation.setAngle(miniCircle_rotation.getAngle() - 360.0);
	}

	/* rotate the triangles in the mini right figure */
	miniTriangle_rotation.moveAngle(miniTriangle_rotation_speed);
	if (miniTriangle_rotation.getAngle() < 0){
		miniTriangle_rotation.setAngle(miniTriangle_rotation.getAngle() + 360.0);
	}
	else if(miniTriangle_rotation.getAngle() >= 360){
		miniTriangle_rotation.setAngle(miniTriangle_rotation.getAngle() - 360.0);
	}
	glutPostRedisplay();
	sleep(20);
}

/* obtain the point from the updated matrix */
floatPoint getPointFromMatrix(floatPoint p, GLfloat matrix[]){
	floatPoint c;
	GLfloat x = p.x * matrix[0] + p.y * matrix[4] + matrix[8] + matrix[12];
	GLfloat y = p.x * matrix[1] + p.y * matrix[5] + matrix[9] + matrix[13];
	c.setPoint(x, y);
	return c;
}
/* display function */
void display(){
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(space.mainView.x_min, space.mainView.x_max,space.mainView.y_min, space.mainView.y_max);
	glViewport(space.mainView.corner.x, space.mainView.corner.y, space.mainView.width, space.mainView.height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);
	/* draw the boundary of the main viewport */
	drawBoundary(space);
	
	/* set up the initial circle */
	Circle initialCircle(space.x_max*0.9/4.0, space.y_max * 1.2/4.0, 30/2.0, space);
	/* this is the initial distance between the square and the circles */
	float circle_distance = abs(s1.center.x - initialCircle.center.x); 
	
	/* set up the initial triangle */
	Triangle initialTriangle(space.x_max*1.3/4.0, space.y_max * 1.2/4.0, 30, space);
	float triangle_distance = abs(s1.center.x - trig[0].center.x);

	glColor3f(0.0f, 0.0f, 1.0f);
	/* draw the center square */
	drawSquare(s1, isFilled);

	/* matrix to store the transformed cordinates */
	GLfloat matrix[16];


	/* draw a wall of circles */
	for(int i = 0; i < 12; i++){
		glPushMatrix();
		
		glTranslatef(-initialCircle.center.x, -initialCircle.center.y, 0.0);
		glRotatef(-(30*i + circle_rotation.getAngle()), 0, 0, 1);
		glTranslatef(initialCircle.center.x, 0, 0);
		glRotatef(30*i + circle_rotation.getAngle() , 0, 0, 1);
		glTranslatef(0, initialCircle.center.y, 0);
		/* this is to get the cordinates of the transformed circle in identity matrix */
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		circle[i].center = getPointFromMatrix(initialCircle.center, matrix);
		circle[i].setCircle(circle[i].center.x, circle[i].center.y, initialCircle.radius, space);
		/* draw */
		if(!circle[i].bBox.isHit)
			drawCircle(initialCircle, isFilled);
		glPopMatrix();
	}
	/* draw a wall of triangles */
	glColor3f(1.0, 0.2, 0.3);
	for(int i = 0; i < 18; i++){
		glPushMatrix();
		
		glTranslatef(-initialTriangle.center.x, -initialTriangle.center.y, 0.0);
		glRotatef(-(20*i + triangle_rotation.getAngle()), 0, 0, 1);
		glTranslatef(initialTriangle.center.x, 0, 0);
		glRotatef(20*i + triangle_rotation.getAngle() , 0, 0, 1);
		glTranslatef(0, initialTriangle.center.y, 0);
		/* this is to get the cordinates of the transformed triangle in identity matrix */
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		trig[i].center = getPointFromMatrix(initialTriangle.center, matrix);
		trig[i].setTriangle(trig[i].center.x, trig[i].center.y, initialTriangle.length, space);
		/* draw */
		if(!trig[i].bBox.isHit)
			drawTriangle(initialTriangle, isFilled);
		glPopMatrix();
	}
	/*for(int i = 0; i<18; i++){
		drawTriangle(trig[i]);
	}*/
	
	/* draw mini wall */

	//Circle initialMiniCircle(s2.center.x + 0.55/4.0 * space.x_max, s2.center.y, 30/2.0, space);
	Square initialMiniSquare(space.center.x, space.center.y, 30, 30, space);
	Circle initialMiniCircle(space.center.x, space.center.y, 30/2.0, space);
	Triangle initialMiniTriangle(space.center.x, space.center.y, 30, space);

	GLfloat miniCircleDistance = /*abs(s2.center.x - initialMiniCircle.center.x);*/ 0.75/4.0 * space.x_max;
	GLfloat miniTriangleDistance = 0.4/4.0 * space.x_max;/*abs(initialMiniCircle.center.x - initialMiniTriangle.center.x);*/
	
	glColor3f(0.2, 0.1, 0.4);

	/* draw */
	glPushMatrix();
		/* square */
		glTranslatef(s2.center.x, s2.center.y, 0);
		if(!s2.isHit)
			drawSquare(initialMiniSquare, isFilled);
	
		glPushMatrix();
			/* first circle */
			glRotatef(45 + miniCircle_rotation.getAngle(), 0, 0, 1);
			glTranslatef(miniCircleDistance, 0, 0);
			glRotatef(-(45 + miniCircle_rotation.getAngle()), 0, 0, 1);
				glPushMatrix();
					/* first triangle */
					glRotatef(miniTriangle_rotation.getAngle(), 0, 0, 1);
					glTranslatef(miniTriangleDistance, 0, 0);
					glRotatef(-miniTriangle_rotation.getAngle(), 0, 0, 1);
					
					glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
					miniTrig[0].center = getPointFromMatrix(initialMiniTriangle.center, matrix);
					miniTrig[0].setTriangle(miniTrig[0].center.x, miniTrig[0].center.y, initialMiniTriangle.length, space);
					/* draw triangle */
					glColor3f(0.02, 0.4, 0.01);
					if(!miniTrig[0].bBox.isHit)
						drawTriangle(initialMiniTriangle, isFilled);
				glPopMatrix();

			glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
			miniCircle[0].center = getPointFromMatrix(initialMiniCircle.center, matrix);
			miniCircle[0].setCircle(miniCircle[0].center.x, miniCircle[0].center.y, initialMiniCircle.radius, space);
			/* draw circle */
			glColor3f(0.3, 0.4, 0.6);
			if(!miniCircle[0].bBox.isHit)
				drawCircle(initialMiniCircle, isFilled);
		glPopMatrix();

		glPushMatrix();
			/* second circle */
			glRotatef(225 + miniCircle_rotation.getAngle(), 0, 0, 1);
			glTranslatef(miniCircleDistance, 0, 0);
			glRotatef(-(225 + miniCircle_rotation.getAngle()), 0, 0, 1);
				glPushMatrix();
					/* second triangle */
					glRotatef(-miniTriangle_rotation.getAngle(), 0, 0, 1);
					glTranslatef(miniTriangleDistance, 0, 0);
					glRotatef(miniTriangle_rotation.getAngle(), 0, 0, 1);

					glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
					miniTrig[1].center = getPointFromMatrix(initialMiniTriangle.center, matrix);
					miniTrig[1].setTriangle(miniTrig[1].center.x, miniTrig[1].center.y, initialMiniTriangle.length, space);
					/* draw triangle */
					glColor3f(0.0, 0.3, 0.32);
					if(!miniTrig[1].bBox.isHit)
						drawTriangle(initialMiniTriangle, isFilled);
				glPopMatrix();

			glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
			miniCircle[1].center = getPointFromMatrix(initialMiniCircle.center, matrix);
			miniCircle[1].setCircle(miniCircle[1].center.x, miniCircle[1].center.y, initialMiniCircle.radius, space);
			/* draw circle */
			glColor3f(1.0, 0.7, 0.02);
			if(!miniCircle[1].bBox.isHit)
				drawCircle(initialMiniCircle, isFilled);
		glPopMatrix();
	glPopMatrix();
	
	glColor3f(0.1, 0.6, 0.3);
	drawSixgon(sixgon, false);

	drawStar(star, false);

	glColor3f(1.0, 0.0, 0.0);
	drawPaddle(paddle);
	if( !ball.isShot )
		ball.initialize(paddle, space);
	glColor3f(0.0, 1.0, 0.0);
	drawCircle(ball, true);

	/* the score view */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(space.scoreView.x_min, space.scoreView.x_max,space.scoreView.y_min, space.scoreView.y_max);
	glViewport(space.scoreView.corner.x, space.scoreView.corner.y, space.scoreView.width, space.scoreView.height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glColor3f(0.1, 0.5, 0.6);
	glRasterPos2f(0.9*space.x_min,0.95*space.y_min);
	
	char buff[50];
	sprintf(buff, "Score: %d\n", score);
	for(int i=0; i<strlen(buff); i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buff[i]);
	}
	glRasterPos2f(space.center.x, 0.95*space.y_min);
	if(ball.isShot)
		sprintf(buff, "Timer: %d\n", clock()-shootTime);
	else
		sprintf(buff, "Timer: 0\n");
	for(int i=0; i<strlen(buff); i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buff[i]);
	}
	//glFlush();
	glutSwapBuffers();
}

/* initialize the display */
void init(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(space.mainView.x_min, space.mainView.x_max,space.mainView.y_min, space.mainView.y_max);
	glViewport(space.scoreView.corner.x, space.scoreView.corner.y, space.scoreView.width, space.scoreView.height);
	glViewport(space.mainView.corner.x, space.mainView.corner.y, space.mainView.width, space.mainView.height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
/* Menu */
void menu(int entry){
	switch(entry){
		case MENU_FILLED:
			isFilled = true;
			break;
		case MENU_WIRE:
			isFilled = false;
			break;
		case MENU_RESET:
			ball.isShot = false;
			paddle.resetPaddleAngle();
			shootTime = 0;
			score = 0;
			for(int i =0; i<12; i++){
				circle[i].bBox.isHit = false;
			}
			for(int i =0; i<18; i++){
				trig[i].bBox.isHit = false;
			}
			s1.isHit = false;
			s2.isHit = false;
			for(int i=0; i<2; i++){
				miniCircle[i].bBox.isHit = false;
				miniTrig[i].bBox.isHit = false;
			}
			sixgon.bBox.isHit = false;
			star.bBox.isHit = false;
			break;
		case MENU_QUIT:
			exit(0);
			break;
		default:

			break;
	}
	glutPostRedisplay();
}
/* create menu */
void create_menu(){
	int polygonId = glutCreateMenu(menu);
	glutAddMenuEntry("Filled", MENU_FILLED);
	glutAddMenuEntry("Wire", MENU_WIRE);
	int menu_id = glutCreateMenu(menu);
	glutAddSubMenu("Polygon", polygonId);
	glutAddMenuEntry("Reset", MENU_RESET);
	glutAddMenuEntry("Quit", MENU_QUIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
/* reshape the window */
void reshape(int w, int h){
	space.resize(w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(space.x_min, space.x_max, space.y_min, space.y_max);
	gluOrtho2D(space.mainView.x_min, space.mainView.x_max,space.mainView.y_min, space.mainView.y_max);
	glViewport(space.scoreView.corner.x, space.scoreView.corner.y, space.scoreView.width, space.scoreView.height);
	glViewport(space.mainView.corner.x, space.mainView.corner.y, space.mainView.width, space.mainView.height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	s1.reshape(space);
	paddle.reshape(space);
	ball.reshape(space);
	s2.reshape(space);
	sixgon.reshape(space);
	star.reshape(space);
}

int main(int argc, char**argv){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(space.window_width, space.window_height);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Break Out");
	create_menu();
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(paddleMotion);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(animation);
	
	glutMainLoop();
	
	return 0;

return 0;
}