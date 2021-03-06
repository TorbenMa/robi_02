#include "Odometrie.h"
#include "Navigation.h"
#include "Motor.h"
#include <Arduino.h>


Navigation::Navigation(){
	Position = 0;
}


Odometrie& Navigation::getOdometrie(){
	return Odo;
}

Motor& Navigation::getMotor(){
	return Moto;
}

Kommunikation& Navigation::getJSON(){
	return JSON;
}

void Navigation::UpdateData() {
	Odo.updateOdometrie();
	Moto.updateVelocity();
	float xFromPosition, yFromPosition;

	if (/*JSON.getPosition(xFromPosition, yFromPosition)*/ 0 ) {
		x_aktuell = int(xFromPosition);
		y_aktuell = int(yFromPosition);
		Odo.setPosition(x_aktuell, y_aktuell);

		if ((x_PositionteamOld != 0) && (y_PositionteamOld != 0)){
			x_PositionteamNew = x_PositionteamOld;
			y_PositionteamNew = x_PositionteamOld;
		}
		x_PositionteamOld = x_aktuell;
		y_PositionteamOld = y_aktuell;
		if ((x_PositionteamNew != 0) && (y_PositionteamNew != 0)){
			int deltaX = x_PositionteamNew - x_PositionteamOld;
			int deltaY = y_PositionteamNew - y_PositionteamOld;
			float angle = atan2(deltaY, deltaX) * 180/PI;	// evtl ( 0 , 0 ) abfangen
			Odo.setAngle(angle);
		}
	} else {
		x_aktuell = Odo.getX_position();
		y_aktuell = Odo.getY_position();
		x_PositionteamOld = 0;
		y_PositionteamOld = 0;
		x_PositionteamNew = 0;
		y_PositionteamNew = 0;
	}
	Odo.testOdometrie();
}

float Navigation::getCalculateAngle(int x, int y) {

	double delta_x = x - x_aktuell;
	double delta_y = y - y_aktuell;
	double Winkel = atan2(delta_y, delta_x) * 180 / PI;

	return Winkel;
}
// TODO: Evtl nicht mehr genutzt
bool Navigation::PositionInLava(){
	bool Lava = false;
	int Lavabereich = 300;
	if (((x_aktuell >= 0) && (x_aktuell <= Lavabereich)) ){
		if ((y_aktuell >= 0) && (y_aktuell <= 2000)){
			Lava = true;
		}
	}
	if (((x_aktuell >= 3000-Lavabereich) && (x_aktuell <= 3000))){
		if ((y_aktuell >= 0) && (y_aktuell <= 2000)){
			Lava = true;
		}
	}
	if((x_aktuell >= Lavabereich) && (x_aktuell <= 3000-Lavabereich)){
		if ((y_aktuell >= 2000-Lavabereich) && (y_aktuell <= 2000)){
			Lava = true;
		}
	}
	if ((x_aktuell >= Lavabereich) && (x_aktuell <= 3000-Lavabereich)){
		if ((y_aktuell >= 0) && (y_aktuell <= Lavabereich)){
			Lava = true;
		}
	}

	return Lava;
}
// Berechnet die VektorlÃ¤nge zwischen dem aktuellem Standort und dem Zielpunkt.
// Wird evtl nicht mehr gebraucht
float Navigation::getLengthToPosition(int x, int y) {
	int delta_x = x - x_aktuell;
	int delta_y = y - y_aktuell;
	return sqrt(pow(delta_x, 2) + pow(delta_y, 2));
}

void Navigation::turnToTargetAngle() {
	e = targetAngle - Odo.getAngle();
	Moto.turn(signum(e) * speed);
	Serial.print("  targetAngle: ");
	Serial.print(targetAngle);
	Serial.print("  OdoAngle: ");
	Serial.print(Odo.getAngle());
	Serial.print(" speed: ");
	Serial.print(speed);
	Serial.print(" e: ");
	Serial.println(e);
	if (abs(e) < 50){
		//speed --;		//Originalversion
		speed = speed - 5;
		if (speed <= 0){
			Moto.stop();
		}
	}
}

void Navigation::driveToTargetPosition(){
	e = getLengthToPosition(X_Koordinaten[Position], Y_Koordinaten[Position]);
	// implementierung P-Regler
	Serial.println(e);
	controlDeviation = getTargetAngle();	// Soll Winkel
	actualDeviation = Odo.getAngle();		// Ist Winkel
	//Reglerdifferenz verstärken und übertragen
	differenceDeviation = (controlDeviation - actualDeviation) * amplifierKp;
	Moto.driveStraightRegulated(speed, differenceDeviation);
}

bool Navigation::DetectedEnemyInArea(){
	int xPositionObject = 0;
	int yPositionObject = 0;
	bool result = false;
	int radius = 270; 		// Radius des Fahrzeugs 150 + 100 Messentfernung + 20 Offset
	xPositionObject = cos(Odo.getAngle()* PI/180)*radius + x_aktuell;
	yPositionObject = sin(Odo.getAngle()* PI/180)*radius + y_aktuell;

	if (((xPositionObject > 0) && (xPositionObject < 3000)) && ((yPositionObject > 0) && (yPositionObject < 2000))){
		result = true;
	}
	Serial.print("x:  ");
	Serial.println(xPositionObject);
	Serial.print("y:  ");
	Serial.println(yPositionObject);
	Serial.print("Ergebnis: Objekt im Gebiet:  ");
	Serial.println(result);
	return result;
}

int Navigation::getTargetCoordinateX(){
	return X_Koordinaten[Position];
}

int Navigation::getTargetCoordinateY(){
	return Y_Koordinaten[Position];
}

float Navigation::getDeviation(){
	return e;
}

float Navigation::getSpeed(){
	return speed;
}
float Navigation::getTargetAngle(){
	float targetAngle = getCalculateAngle(X_Koordinaten[Position], Y_Koordinaten[Position]);
	return targetAngle;
}

int Navigation::getPosition(){
	return Position;
}

int Navigation::getY(){
	return y_aktuell;
}

int Navigation::getX(){
	return x_aktuell;
}

float Navigation::getSafetyRadius(){
	return safetyRadius;
}

int Navigation::getMaximalPosition(){
	return maxPosition;
}

void Navigation::setSpeed(int speed){
	this->speed = speed;
}


void Navigation::setTargetAngle(float angle) {
	if (angle >= 360.0){
		angle -= 360.0;
	}
	targetAngle = angle;
}

void Navigation::setNextPosition(){
	Position ++;
}

void Navigation::setPosition(int Position){
	this->Position = Position;
}
void Navigation::setStartParameters(int x, int y, float angle){
	Odo.setXposition(x);
	Odo.setYposition(y);
	Odo.setAngle(angle);
}

// Gibt Vorzeichen des übergebenen Wertes zurück
int Navigation::signum(float sign){
	int NumberSign = 0;
	if (sign > 0){
		NumberSign = 1;
	}
	if (sign < 0){
		NumberSign = -1;
	}
	return NumberSign;
}


