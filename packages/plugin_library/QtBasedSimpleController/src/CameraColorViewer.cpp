//
// Created by 潘绪洋 on 17-3-10.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#include <highgui.h>
#include <QtCore/QCoreApplication>
#include <cobotsys_global_object_factory.h>
#include <extra2.h>
#include "CameraColorViewer.h"
#include "opencv2/opencv.hpp"

CameraColorViewer::CameraColorViewer()
        : QObject(nullptr){
    m_captureTimer = new QTimer(this);
    m_captureTimer->setInterval(1);
    connect(m_captureTimer, &QTimer::timeout, this, &CameraColorViewer::updateCamera);
}

CameraColorViewer::~CameraColorViewer(){
    INFO_DESTRUCTOR(this);
    stop();
}

bool CameraColorViewer::start(){
    if (m_camera && m_captureTimer) {
        if (m_camera->open()) {
            m_captureTimer->start();
            return true;
        }
    }
    return false;
}

void CameraColorViewer::pause(){
    if (m_captureTimer)
        m_captureTimer->stop();
}

void CameraColorViewer::stop(){
    pause();
    if (m_camera) {
        m_camera->close();
    }
}

void CameraColorViewer::onCameraStreamUpdate(const cobotsys::CameraFrame& frames, cobotsys::AbstractCamera* camera){
    for (const auto& frame: frames.frames) {
        if (frame.type == cobotsys::ImageType::Color) {
            cv::Mat m;
            cv::pyrDown(frame.data, m);
            cv::imshow(cobotsys::toString(frame.type), m);

            char key = (char) cv::waitKey(5);
            if (key == 27) {
                m_camera->close();
                QCoreApplication::quit();
            }
        }
    }
}

void CameraColorViewer::updateCamera(){
    if (m_camera) {
        m_camera->capture();
    }
}

bool CameraColorViewer::setup(const QString& configFilePath){
    auto pFactory = cobotsys::GlobalObjectFactory::instance();
    if (pFactory) {
        auto pObject = pFactory->createObject("Kinect2CameraFactory, Ver 1.0", "Kinect2");
        if (pObject) {
            m_camera = std::dynamic_pointer_cast<cobotsys::AbstractCamera>(pObject);
            if (m_camera) {
                auto shObj = shared_from_this();
                auto shObs = std::dynamic_pointer_cast<cobotsys::CameraStreamObserver>(shObj);
                m_camera->attach(shObs);
                return true;
            }
        }
    }
    return false;
}
