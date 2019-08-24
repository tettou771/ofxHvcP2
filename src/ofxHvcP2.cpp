#include "ofxHvcP2.h"


int UART_SendData(int inDataSize, UINT8 *inData) {
	/* UART send signal */
	int ret = com_send(inData, inDataSize);
	return ret;
}

int UART_ReceiveData(int inTimeOutTime, int inDataSize, UINT8 *outResult) {
	/* UART receive signal */
	int ret = com_recv(inTimeOutTime, outResult, inDataSize);
	return ret;
}

ofxHvcP2::ofxHvcP2() {
	execFlag = 0x0;
	imageNo = HVC_EXECUTE_IMAGE_NONE;
	initialized = false;
}


ofxHvcP2::~ofxHvcP2() {
	close();
}

void ofxHvcP2::setup(int _comPortNum) {
	int comPortNum;

	S_STAT serialStat;
	serialStat.com_num = _comPortNum;
	serialStat.BaudRate = 0;

	if (com_init(&serialStat) == 0) {
		ofLogError() << "Failed to open COM port.";
		initialized = false;
	}
	else {
		startThread();
		initialized = true;
		ofAddListener(ofEvents().update, this, &ofxHvcP2::update);
	}
}

void ofxHvcP2::update(ofEventArgs & e) {
	if (frameUpdated) {
		frameNew = true;
		frameUpdated = false;
	}
	else {
		frameNew = false;
	}

	if (!initialized) {
		startThread();
		initialized = true;
		ofLog() << "restart ofxHvcP2 thread.";
	}
}

void ofxHvcP2::close() {
	if (initialized) {
		ofRemoveListener(ofEvents().update, this, &ofxHvcP2::update);
		com_close();
	}
}

string ofxHvcP2::getExpressionName(Expression e) {
	switch (e) {
	case UnknownExpression: return "UnknownExpression"; break;
	case Neutoral: return "Neutoral"; break;
	case Happiness: return "Happiness"; break;
	case Surprize: return "Surprize"; break;
	case Anger: return "Anger"; break;
	case Sadness: return "Sadness"; break;
	default: return "";
	}
}


void ofxHvcP2::threadedFunction() {
	pHVCResult = new HVC_RESULT();

	loopBreakFlag = false;
	while (true) {
		loop();
		frameUpdated = true;

		if (loopBreakFlag) {
			initialized = false;
			break;
		}
	}
}

void ofxHvcP2::loop() {
	/*********************************/
	/* Execute Detection             */
	/*********************************/
	timeOutTime = 1000; // msec // UART_EXECUTE_TIMEOUT;
	int ret = HVC_ExecuteEx(timeOutTime, execFlag, imageNo, pHVCResult, &status);
	if (ret != 0) {
		ofLogError() << "HVCApi(HVC_ExecuteEx) Error : " + ofToString(ret);
		loopBreakFlag = true;
		return;
	}
	if (status != 0) {
		ofLogError() << "HVC_ExecuteEx Response Error : " + ofToString(ofToHex(status));
		loopBreakFlag = true;
		return;
	}

	mutex.lock();

	if (imageNo != HVC_EXECUTE_IMAGE_NONE) {
		makeCapturedImage();
	}
	if (pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION) {
		/* Body Detection result string */
		bodies.clear();
		int numBodies = pHVCResult->bdResult.num;

		for (int i = 0; i < numBodies; i++) {
			bodies.push_back(Body());
			Body &newBody = bodies.back();

			newBody.position.x = pHVCResult->bdResult.bdResult[i].posX;
			newBody.position.y = pHVCResult->bdResult.bdResult[i].posY;
			newBody.size = pHVCResult->bdResult.bdResult[i].size;
			newBody.confidence = pHVCResult->bdResult.bdResult[i].confidence;
		}

		// body information debug print
		if (debugPrint) {
			if (debugPrint) cout << "body count : " + ofToString(numBodies) << endl;
			for (int bodyIndex = 0; bodyIndex < bodies.size(); ++bodyIndex) {
				auto &b = bodies[bodyIndex];
				cout << "index:" << bodyIndex;
				cout << "\tpos:(" << b.position.x << ", " << b.position.y << ")\tsize:" << b.size << "\tconfidence:" << b.confidence << endl;
				cout << endl;
			}
			cout << endl;
		}
	}

	if (pHVCResult->executedFunc & HVC_ACTIV_HAND_DETECTION) {
		/* Hand Detection result string */
		hands.clear();
		int numHands = pHVCResult->hdResult.num;

		for (int i = 0; i < numHands; ++i) {
			hands.push_back(Hand());
			auto &newHand = hands.back();

			newHand.position.x = pHVCResult->hdResult.hdResult[i].posX;
			newHand.position.y = pHVCResult->hdResult.hdResult[i].posY;
			newHand.size = pHVCResult->hdResult.hdResult[i].size;
			newHand.confidence = pHVCResult->hdResult.hdResult[i].confidence;
		}

		// hand information debug print
		if (debugPrint) {
			if (debugPrint) cout << "hand count : " + ofToString(numHands) << endl;
			for (int handIndex = 0; handIndex < hands.size(); ++handIndex) {
				auto &h = hands[handIndex];
				cout << "index:" << handIndex;
				cout << "\tpos:(" << h.position.x << ", " << h.position.y << ")\tsize:" << h.size << "\tconfidence:" << h.confidence << endl;
				cout << endl;
			}
			cout << endl;
		}
	}

	/* Face Detection result string */
	if (pHVCResult->executedFunc &
		(HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
			HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION |
			HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
			HVC_ACTIV_EXPRESSION_ESTIMATION | HVC_ACTIV_FACE_RECOGNITION)) {



		int numFaces = pHVCResult->fdResult.num;
		faces.clear();

		for (int i = 0; i < pHVCResult->fdResult.num; i++) {
			faces.push_back(Face());
			auto &newFace = faces.back();

			if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION) {
				/* Detection */
				newFace.position.x = pHVCResult->fdResult.fcResult[i].dtResult.posX;
				newFace.position.y = pHVCResult->fdResult.fcResult[i].dtResult.posY;
				newFace.size = pHVCResult->fdResult.fcResult[i].dtResult.size;
				newFace.confidence = pHVCResult->fdResult.fcResult[i].dtResult.confidence;
			}
			if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION) {
				/* Face Direction */
				newFace.direction.x = pHVCResult->fdResult.fcResult[i].dirResult.pitch;
				newFace.direction.y = pHVCResult->fdResult.fcResult[i].dirResult.roll;
				newFace.direction.z = pHVCResult->fdResult.fcResult[i].dirResult.yaw;
				newFace.directionConfidence = pHVCResult->fdResult.fcResult[i].dirResult.confidence;
			}
			if (pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION) {
				/* Age */
				if (-128 == pHVCResult->fdResult.fcResult[i].ageResult.age) {
					newFace.age = 0;
					newFace.ageConfidence = 0;
				}
				else {
					newFace.age = pHVCResult->fdResult.fcResult[i].ageResult.age;
					newFace.ageConfidence = pHVCResult->fdResult.fcResult[i].ageResult.confidence;
				}

			}
			if (pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION) {
				/* Gender */
				if (-128 == pHVCResult->fdResult.fcResult[i].genderResult.gender) {
					newFace.gender = UnknownGender;
					newFace.genderConfidence = 0;
				}
				else {
					if (1 == pHVCResult->fdResult.fcResult[i].genderResult.gender) {
						newFace.gender = Male;
					}
					else {
						newFace.gender = Female;
					}
					newFace.genderConfidence = pHVCResult->fdResult.fcResult[i].genderResult.confidence;
				}
			}
			if (pHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION) {
				/* Gaze */
				if ((-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR) ||
					(-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD)) {
					newFace.gaze = vec2i();
				}
				else {
					newFace.gaze.x = pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR;
					newFace.gaze.y = pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD;
				}
			}
			if (pHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION) {
				/* Blink */
				if ((-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioL) ||
					(-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioR)) {
					newFace.blinkL = 0;
					newFace.blinkR = 0;
				}
				else {
					newFace.blinkL = pHVCResult->fdResult.fcResult[i].blinkResult.ratioL;
					newFace.blinkR = pHVCResult->fdResult.fcResult[i].blinkResult.ratioR;
				}
			}
			if (pHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION) {
				/* Expression */
				newFace.expressionDegree = 0;

				if (-128 == pHVCResult->fdResult.fcResult[i].expressionResult.score[0]) {
					newFace.expression = UnknownExpression;
				}
				else {
					if (pHVCResult->fdResult.fcResult[i].expressionResult.topExpression > EX_SADNESS) {
						pHVCResult->fdResult.fcResult[i].expressionResult.topExpression = 0;
					}

					newFace.expression = Expression(pHVCResult->fdResult.fcResult[i].expressionResult.topExpression);
					for (int exIndex = 0; exIndex < ExpressionNum - 1; ++exIndex) {
						newFace.expressionScore[exIndex] = pHVCResult->fdResult.fcResult[i].expressionResult.score[exIndex];
						if (newFace.expressionScore[exIndex] > newFace.expressionDegree) {
							newFace.expressionDegree = newFace.expressionScore[exIndex];
						}
					}

					//newFace.expressionDegree = pHVCResult->fdResult.fcResult[i].expressionResult.degree;
				}
			}
		}

		// face information debug print
		if (debugPrint) {
			cout << "face count " << numFaces << endl;
			for (int faceIndex = 0; faceIndex < faces.size(); ++faceIndex) {
				auto &f = faces[faceIndex];
				cout << "index:" << faceIndex;
				cout << "\tpos:(" << f.position.x << ", " << f.position.y << ")\tsize:" << f.size << "\tconfidence:" << f.confidence << endl;
				cout << "\tdirection:(" << f.direction.x << ", " << f.direction.y << ", " << f.direction.z << ")\tconfidence:" << f.directionConfidence << endl;
				cout << "\tage:" << f.age << "\tconfidence:" << f.ageConfidence << endl;
				cout << "\tgender:" << f.gender << "\tconfidence:" << f.genderConfidence << endl;
				cout << "\tgaze:(" << f.gaze.x << ", " << f.gaze.y << ")" << endl;
				cout << "\tblink_L:" << f.blinkL << "\tblink_R:" << f.blinkR << endl;
				cout << "\texpression:" << f.expression << endl;
				cout << "\tscore ";
				for (int i = 0; i < ExpressionNum - 1; ++i) {
					cout << ' ' << i << ':' << f.expressionScore[i];
				}
				cout << endl << "\texpressionDegree:" << f.expressionDegree << endl;
				cout << endl;
			}
			cout << endl;
		}
	}

	mutex.unlock();
}

void ofxHvcP2::makeCapturedImage() {
	if (pHVCResult == NULL) return;

	int width = pHVCResult->image.width;
	int height = pHVCResult->image.height;
	if (width == 0 || height == 0) return;

	// if image size is not match, reallocate
	if (capturePixels.getWidth() != width || capturePixels.getHeight() != height) {
		capturePixels.clear();
		capturePixels.allocate(width, height, ofImageType::OF_IMAGE_GRAYSCALE);
	}

	// make image
	for (int pixelIndex = 0; pixelIndex < width * height; ++pixelIndex) {
		capturePixels[pixelIndex] = pHVCResult->image.image[pixelIndex];
	}
	capture.setFromPixels(capturePixels);
}

void ofxHvcP2::setExecFlag(INT32 flag, bool enable) {
	if (enable) execFlag = execFlag | flag;
	else execFlag = execFlag & (~flag);
}

bool ofxHvcP2::getExecFlag(INT32 flag) {
	return (execFlag & flag) == flag;
}

void ofxHvcP2::setActiveBody(bool enable) { setExecFlag(HVC_ACTIV_BODY_DETECTION, enable); }
void ofxHvcP2::setActiveHand(bool enable) { setExecFlag(HVC_ACTIV_HAND_DETECTION, enable); }
void ofxHvcP2::setActiveFace(bool enable) { setExecFlag(HVC_ACTIV_FACE_DETECTION, enable); }
void ofxHvcP2::setActiveDirection(bool enable) {	setExecFlag(HVC_ACTIV_FACE_DIRECTION, enable);}
void ofxHvcP2::setActiveAge(bool enable) {	setExecFlag(HVC_ACTIV_AGE_ESTIMATION, enable);}
void ofxHvcP2::setActiveGender(bool enable) {	setExecFlag(HVC_ACTIV_GENDER_ESTIMATION, enable);}
void ofxHvcP2::setActiveGaze(bool enable) {	setExecFlag(HVC_ACTIV_GAZE_ESTIMATION, enable);}
void ofxHvcP2::setActiveBlink(bool enable) {	setExecFlag(HVC_ACTIV_BLINK_ESTIMATION, enable);}
void ofxHvcP2::setActiveExpression(bool enable) {	setExecFlag(HVC_ACTIV_EXPRESSION_ESTIMATION, enable);}
void ofxHvcP2::setImageSize(ImageSize imageSize) {
	switch (imageSize) {
	case NoImage: imageNo = HVC_EXECUTE_IMAGE_NONE; break;
	case QVGA: imageNo = HVC_EXECUTE_IMAGE_QVGA; break;
	case QVGA_HALF: imageNo = HVC_EXECUTE_IMAGE_QVGA_HALF; break;
	}
}

bool ofxHvcP2::getActiveBody() { return getExecFlag(HVC_ACTIV_BODY_DETECTION); }
bool ofxHvcP2::getActiveHand() { return getExecFlag(HVC_ACTIV_HAND_DETECTION); }
bool ofxHvcP2::getActiveFace() { return getExecFlag(HVC_ACTIV_FACE_DETECTION); }
bool ofxHvcP2::getActiveDirection() { return getExecFlag(HVC_ACTIV_FACE_DIRECTION); }
bool ofxHvcP2::getActiveAge() { return getExecFlag(HVC_ACTIV_AGE_ESTIMATION); }
bool ofxHvcP2::getActiveGender() { return getExecFlag(HVC_ACTIV_GENDER_ESTIMATION); }
bool ofxHvcP2::getActiveGaze() { return getExecFlag(HVC_ACTIV_GAZE_ESTIMATION); }
bool ofxHvcP2::getActiveBlink() { return getExecFlag(HVC_ACTIV_BLINK_ESTIMATION); }
bool ofxHvcP2::getActiveExpression() { return getExecFlag(HVC_ACTIV_EXPRESSION_ESTIMATION); }
ofxHvcP2::ImageSize ofxHvcP2::getImageSize() {
	return (ImageSize)imageNo;
}

void ofxHvcP2::setActiveDebugPrint(bool enable) {
	debugPrint = enable;
}

void ofxHvcP2::getBodies(Bodies &out) {
	mutex.lock();
	out = bodies;
	mutex.unlock();
}

void ofxHvcP2::getHands(Hands &out) {
	mutex.lock();
	out = hands;
	mutex.unlock();
}

void ofxHvcP2::getFaces(Faces &out) {
	mutex.lock();
	out = faces;
	mutex.unlock();
}

ofImage & ofxHvcP2::getImage() {
	return capture;
}

bool ofxHvcP2::isFrameNew() {
	return frameNew;
}

bool ofxHvcP2::isInitialized() {
	return initialized;
}

