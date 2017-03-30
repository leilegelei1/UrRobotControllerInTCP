//
// Created by 潘绪洋 on 17-3-27.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#include <QtCore/QJsonObject>
#include <extra2.h>
#include "URRealTimeDriver.h"


URRealTimeDriver::URRealTimeDriver() : QObject(nullptr){
    m_isWatcherRunning = false;
    m_isStarted = false;

    m_ctrl = nullptr;
    m_rt_ctrl = nullptr;
    m_urDriver = nullptr;
    m_curReqQ.clear();
}

URRealTimeDriver::~URRealTimeDriver(){
    if (m_isWatcherRunning) {
        m_isWatcherRunning = false;
        m_rt_msg_cond.notify_all();
        m_thread.join();
    }
}

void URRealTimeDriver::move(const std::vector<double>& q){
    std::lock_guard<std::mutex> lock_guard(m_mutex);
    if (m_isStarted) {
        m_curReqQ = q;
    }
}

std::shared_ptr<AbstractDigitIoDriver> URRealTimeDriver::getDigitIoDriver(int deviceId){
    return nullptr;
}

void URRealTimeDriver::attach(std::shared_ptr<ArmRobotRealTimeStatusObserver> observer){
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    for (auto& iter : m_observers) {
        if (iter.get() == observer.get()) {
            return; // Already have attached
        }
    }

    if (observer) {
        m_observers.push_back(observer);
    }
}

bool URRealTimeDriver::start(){
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    if (m_urDriver) {
        COBOT_LOG.info() << "Already start, if want restart, stop first";
        return false;
    }
    m_urDriver = new CobotUrDriver(m_rt_msg_cond, m_msg_cond, m_attr_robot_ip.c_str());
    connect(m_urDriver, &CobotUrDriver::driverStartSuccess, this, &URRealTimeDriver::driverReady);
    m_urDriver->setServojTime(m_attr_servoj_time);
    m_urDriver->setServojLookahead(m_attr_servoj_lookahead);
    m_urDriver->setServojGain(m_attr_servoj_gain);
    m_urDriver->startDriver();
    return true;
}

void URRealTimeDriver::stop(){
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    if (m_urDriver) {
        m_isStarted = false;
        m_urDriver->stopDriver();
        m_urDriver->deleteLater();
        m_urDriver = nullptr;
        m_curReqQ.clear();
    }
}

bool URRealTimeDriver::setup(const QString& configFilePath){
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    auto success = _setup(configFilePath);

    if (!success) {
        m_observers.clear(); // detach all observer
    }
    return success;
}

void URRealTimeDriver::robotStatusWatcher(){
    std::mutex m;
    std::unique_lock<std::mutex> lck(m);

    auto time_cur = std::chrono::high_resolution_clock::now();

    auto pStatus = std::make_shared<ArmRobotStatus>();
    std::vector<std::shared_ptr<ArmRobotRealTimeStatusObserver> > observer_tmp;
    std::vector<double> q_next;


    while (m_isWatcherRunning) {
        m_rt_msg_cond.wait(lck);

        auto time_rdy = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_diff = time_rdy - time_cur;
        time_cur = time_rdy;

        if (m_mutex.try_lock()) {
            if (m_urDriver) {
                auto pState = m_urDriver->m_urRealTimeCommCtrl->ur->getRobotState();
                q_next = pState->getQActual();
            }
            m_mutex.unlock();
        }

        pStatus->q_actual = q_next;

//        COBOT_LOG.info() << "Status Updated: " << time_diff.count();

        // Notify all attached observer
        if (m_mutex.try_lock()) {
            observer_tmp = m_observers;
            m_mutex.unlock();
        }
        for (auto& ob : observer_tmp) {
            ob->onArmRobotStatusUpdate(pStatus);
        }

        if (m_mutex.try_lock()) {
            q_next = m_curReqQ;
            m_mutex.unlock();
        }
        if (m_isStarted && q_next.size() >= 6) {
            m_urDriver->servoj(q_next);
        }
    }
}

bool URRealTimeDriver::_setup(const QString& configFilePath){
    QJsonObject json;
    if (loadJson(json, configFilePath)) {
        m_attr_robot_ip = json["robot_ip"].toString("localhost").toStdString();
        m_attr_servoj_time = json["servoj_time"].toDouble(0.08);
        m_attr_servoj_lookahead = json["servoj_lookahead"].toDouble(0.05);
        m_attr_servoj_gain = json["servoj_gain"].toDouble(300);

        m_isWatcherRunning = true;
        m_thread = std::thread(&URRealTimeDriver::robotStatusWatcher, this);
        return true;
    }
    return false;
}

void URRealTimeDriver::driverReady(){
    m_isStarted = true;
}

QString URRealTimeDriver::getRobotUrl(){
    return m_attr_robot_ip.c_str();
}