#pragma once

#include "ofMain.h"
#include "ofxHvcP2.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

	ofxHvcP2 hvc;
};