/*
 * VideoSource.cpp
 *
 *  Created on: 20/02/2011
 *      Author: cesar
 */

#include <Kernel/VideoSource.h>

VideoSource::VideoSource(const std::string& name, const int& nDevice, int width,
		int height, int apiPreference) {
	cv::VideoCapture* c = new cv::VideoCapture(nDevice, apiPreference);
	_cam = c;

	if(!_cam->isOpened()){
		_cam->release();
		delete _cam;
		_cam = NULL;
		throw "Camera not found exception: "+name;
	}

	_name = name;
	_lastFrame = new cv::Mat();
	_width = width;
	_height = height;
	_cam->set(cv::CAP_PROP_FRAME_WIDTH,_width);
	_cam->set(cv::CAP_PROP_FRAME_HEIGHT,_height);
	//_width = _cam->get(CV_CAP_PROP_FRAME_WIDTH);
	//_height = _cam->get(CV_CAP_PROP_FRAME_HEIGHT);
	//_fps = _cam.get(CV_CAP_PROP_FPS);
	_fps = -1;
}

cv::Mat* VideoSource::grabFrame(){
	*_cam >> *_lastFrame;

	return _lastFrame;
}

cv::Mat* VideoSource::getLastFrame(){
	if(_lastFrame!=NULL)
		return _lastFrame;
	throw "No frame found exception!";
}

std::string& VideoSource::getName(){return _name;}

int VideoSource::getWidth(){
	return _width;
}

int VideoSource::getHeight(){
	return _height;
}

int VideoSource::getFps(){
	return _fps;
}

VideoSource::~VideoSource() {
	if (_cam != NULL) {
		_cam->release();
		delete _cam;
		_cam = NULL;
	}

	if(_lastFrame!=NULL)
		delete _lastFrame;
}
