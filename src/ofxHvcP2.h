#pragma once
#include "ofMain.h"
#include "uart/uart.h"
#include "HVCApi/HVCApi.h"
#include "HVCApi/HVCDef.h"
#include "HVCApi/HVCExtraUartFunc.h"


#define LOGBUFFERSIZE   8192

#define UART_SETTING_TIMEOUT              1000            /* HVC setting command signal timeout period */
#define UART_EXECUTE_TIMEOUT              ((10+10+6+3+15+15+1+1+15+10)*1000)
/* HVC execute command signal timeout period */

#define SENSOR_ROLL_ANGLE_DEFAULT            0            /* Camera angle setting (0Åã) */

#define BODY_THRESHOLD_DEFAULT             500            /* Threshold for Human Body Detection */
#define FACE_THRESHOLD_DEFAULT             500            /* Threshold for Face Detection */
#define HAND_THRESHOLD_DEFAULT             500            /* Threshold for Hand Detection */
#define REC_THRESHOLD_DEFAULT              500            /* Threshold for Face Recognition */

#define BODY_SIZE_RANGE_MIN_DEFAULT         30            /* Human Body Detection minimum detection size */
#define BODY_SIZE_RANGE_MAX_DEFAULT       8192            /* Human Body Detection maximum detection size */
#define HAND_SIZE_RANGE_MIN_DEFAULT         40            /* Hand Detection minimum detection size */
#define HAND_SIZE_RANGE_MAX_DEFAULT       8192            /* Hand Detection maximum detection size */
#define FACE_SIZE_RANGE_MIN_DEFAULT         64            /* Face Detection minimum detection size */
#define FACE_SIZE_RANGE_MAX_DEFAULT       8192            /* Face Detection maximum detection size */

#define FACE_POSE_DEFAULT                    0            /* Face Detection facial pose (frontal face)*/
#define FACE_ANGLE_DEFAULT                   0            /* Face Detection roll angle (Å}15Åã)*/


class ofxHvcP2 : public ofThread {
public:
	ofxHvcP2();
	~ofxHvcP2();

	void setup(int comPortNum);
	void update(ofEventArgs &e);
	void close();

	enum Gender {
		UnknownGender,
		Male,
		Female
	};

	enum Expression {
		UnknownExpression,
		Neutoral,
		Happiness,
		Surprize,
		Anger,
		Sadness,
		ExpressionNum
	};
	static string getExpressionName(Expression e);

	enum ImageSize {
		NoImage,
		QVGA,
		QVGA_HALF
	};

	struct vec2i {
		int x, y;
		vec2i() : x(0), y(0) {}
		vec2i(int _x, int _y) : x(_x), y(_y){}
		vec2i operator +(vec2i right) {
			return vec2i(x + right.x, y + right.y);
		}
		vec2i operator -(vec2i right) {
			return vec2i(x - right.x, y - right.y);
		}
	};

	struct vec3i {
		int x, y, z;
		vec3i() : x(0), y(0), z(0) {}
		vec3i(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
		vec3i operator + (vec3i right) {
			return vec3i(x + right.x, y + right.y, z + right.z);
		}
		vec3i operator - (vec3i right) {
			return vec3i(x - right.x, y - right.y, z - right.z);
		}
	};

	struct Body {
		int confidence;
		vec2i position;
		int size;
	};

	struct Hand {
		int confidence;
		vec2i position;
		int size;
	};

	struct Face {
		int confidence;
		vec2i position;
		int size;
		vec3i direction;
		int directionConfidence;
		int age;
		int ageConfidence;
		Gender gender;
		int genderConfidence;
		vec2i gaze;
		int blinkL, blinkR;
		Expression expression;
		int expressionScore[ExpressionNum - 1];
		int expressionDegree;
	};

	typedef vector<Body> Bodies;
	typedef vector<Hand> Hands;
	typedef vector<Face> Faces;

	// set active detection
private:
	void setExecFlag(INT32, bool);
	bool getExecFlag(INT32);
public:
	void setActiveBody(bool enable);
	void setActiveHand(bool enable);
	void setActiveFace(bool enable);
	void setActiveDirection(bool enable);
	void setActiveAge(bool enable);
	void setActiveGender(bool enable);
	void setActiveGaze(bool enable);
	void setActiveBlink(bool enable);
	void setActiveExpression(bool enable);
	void setImageSize(ImageSize imageSize);

	bool getActiveBody();
	bool getActiveHand();
	bool getActiveFace();
	bool getActiveDirection();
	bool getActiveAge();
	bool getActiveGender();
	bool getActiveGaze();
	bool getActiveBlink();
	bool getActiveExpression();
	ImageSize getImageSize();
	void setActiveDebugPrint(bool enable);

	// getter
	void getBodies(Bodies &out);
	void getHands(Hands &out);
	void getFaces(Faces &out);
	ofImage &getImage();

	// if frame updated, return true
	bool isFrameNew();

	bool isInitialized();

private:
	UINT8 status;
	HVC_VERSION version;
	HVC_RESULT *pHVCResult = NULL;

	INT32 agleNo;
	HVC_THRESHOLD threshold;
	HVC_SIZERANGE sizeRange;
	INT32 pose;
	INT32 angle;
	INT32 timeOutTime;
	INT32 execFlag;
	INT32 imageNo;

	void threadedFunction();
	void loop();

	void makeCapturedImage();

	bool loopBreakFlag;

	int comPortNum;
	Bodies bodies;
	Hands hands;
	Faces faces;
	ofImage capture;
	ofPixels capturePixels;

	bool frameUpdated, frameNew;
	bool initialized;
	ofMutex mutex;
	bool debugPrint;
};

