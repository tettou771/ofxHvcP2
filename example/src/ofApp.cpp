#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	// HVC-P2 is connect with Serial
	// Please check device manager, and set "comPortNum"
	// Port name is maybe this style "USB Serial Device (COM18)" <- You need this number!
	int comPortNum = 18;
	hvc.setup(comPortNum);

	// Setup hvc parametors.
	hvc.setImageSize(ofxHvcP2::ImageSize::QVGA);
	hvc.setActiveFace(true);
	hvc.setActiveDirection(true);
	hvc.setActiveAge(true);
	hvc.setActiveGender(true);
	hvc.setActiveExpression(true);
	hvc.setActiveGaze(true);
	hvc.setActiveBlink(true);

	// Body and hand must disabled, if you need high frame rate
	hvc.setActiveBody(true); // too heavy
	hvc.setActiveHand(true); // too heavy

	// debug print in consle
	hvc.setActiveDebugPrint(true);
}

//--------------------------------------------------------------
void ofApp::update(){
	// hvc update is automatically.
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (hvc.isInitialized()) {
		// Camera image
		ofPushStyle();
		ofImage image = hvc.getImage();
		image.update();
		ofSetColor(255);
		image.draw(0, 0);
		ofPopStyle();

		// Set frame draw style
		ofPushMatrix();
		ofPushStyle();
		float hvcScale = 0.2; // Scale of position is 5x
		ofScale(hvcScale, hvcScale);
		ofNoFill();
		ofSetRectMode(ofRectMode::OF_RECTMODE_CENTER); // Position that is returned by hvc is center of object

		// Face frames (green)
		ofSetColor(0, 255, 0);
		vector<ofxHvcP2::Face> faces;
		hvc.getFaces(faces);
		for (auto f : faces) {
			ofDrawRectangle(f.position.x, f.position.y, f.size, f.size);
			stringstream info;
			info << "face confidence:" << f.confidence << '\n'
				<< "direction:(" << f.direction.x << ", " << f.direction.y << ", " << f.direction.z << ") confidence:" << f.directionConfidence << '\n'
				<< "age:" << f.age << " confidence:" << f.ageConfidence << '\n'
				<< "gender:" << f.gender << " confidence:" << f.genderConfidence << '\n'
				<< "gaze:(" << f.gaze.x << ", " << f.gaze.y << ")\n"
				<< "blink L:" << f.blinkL << " R:" << f.blinkR << '\n'
				<< ofxHvcP2::getExpressionName(f.expression) << " " << f.expressionDegree;
			ofDrawBitmapString(info.str(), f.position.x - f.size / 2, f.position.y + f.size / 2 + 12 / hvcScale);
		}

		// Body frames (blue)
		ofSetColor(0, 60, 255);
		vector<ofxHvcP2::Body> bodies;
		hvc.getBodies(bodies);
		for (auto b : bodies) {
			ofDrawRectangle(b.position.x, b.position.y, b.size, b.size);
			stringstream info;
			info << "body confidence:" << b.confidence << '\n';
			ofDrawBitmapString(info.str(), b.position.x - b.size / 2, b.position.y + b.size / 2 + 12 / hvcScale);
		}

		// Hand frames (yellow)
		ofSetColor(255, 210, 0);
		vector<ofxHvcP2::Hand> hands;
		hvc.getHands(hands);
		for (auto h : hands) {
			ofDrawRectangle(h.position.x, h.position.y, h.size, h.size);
			stringstream info;
			info << "hand confidence:" << h.confidence << '\n';
			ofDrawBitmapString(info.str(), h.position.x - h.size / 2, h.position.y + h.size / 2 + 12 / hvcScale);
		}

		ofPopStyle();
		ofPopMatrix();
	}

	// Draw error message if hvc didn't initialized
	else {
		ofDrawBitmapStringHighlight("HVC-P2 is not initialized.\nPlease check COM port number.", 20, 20);
	}
}