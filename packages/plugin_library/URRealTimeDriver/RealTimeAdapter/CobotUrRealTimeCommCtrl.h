//
// Created by 潘绪洋 on 17-3-28.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#ifndef PROJECT_COBOTURREALTIMECOMMCTRL_H
#define PROJECT_COBOTURREALTIMECOMMCTRL_H

#include <QObject>
#include <QThread>
#include <cobotsys_logger.h>
#include "CobotUrRealTimeComm.h"

class CobotUrRealTimeCommCtrl : public QObject {
Q_OBJECT
protected:
    QThread workerThread;

public:
    CobotUrRealTimeComm* ur;
    std::condition_variable& cond_msg;
public:
    CobotUrRealTimeCommCtrl(std::condition_variable& msg, const QString& hostIp, QObject* parent = nullptr)
            : QObject(parent), cond_msg(msg){
        ur = new CobotUrRealTimeComm(msg, hostIp);
        ur->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, ur, &QObject::deleteLater);
        connect(this, &CobotUrRealTimeCommCtrl::start, ur, &CobotUrRealTimeComm::start);
        connect(this, &CobotUrRealTimeCommCtrl::commandReady, ur, &CobotUrRealTimeComm::writeLine);
        connect(ur, &CobotUrRealTimeComm::connected, this, &CobotUrRealTimeCommCtrl::onRealTimeConnected);
        connect(this, &CobotUrRealTimeCommCtrl::stopServoj, ur, &CobotUrRealTimeComm::stopProg);
        workerThread.start();
    }

    ~CobotUrRealTimeCommCtrl(){
        workerThread.quit();
        workerThread.wait();
        COBOT_LOG.info() << "CobotUrRealTimeCommCtrl freed";
    }

    void startComm(){
        Q_EMIT start();
    }

    void addCommandToQueue(const QByteArray& ba){
        Q_EMIT commandReady(ba);
    }

    void requireStopServoj(){
        Q_EMIT stopServoj();
    }

Q_SIGNALS:
    void start();
    void commandReady(const QByteArray& ba);
    void stopServoj();

protected:
    void onRealTimeConnected();

private:
    const int MULT_JOINTSTATE_ = 1000000;
    const int MULT_TIME_ = 1000000;
};

#endif //PROJECT_COBOTURREALTIMECOMMCTRL_H
