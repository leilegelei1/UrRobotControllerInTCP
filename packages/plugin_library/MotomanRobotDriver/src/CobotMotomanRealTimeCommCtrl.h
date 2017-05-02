//
// Created by 杨帆 on 17-5-2.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#ifndef COBOT_MOTOMAN_REALTIME_COMM_CTRL_H
#define COBOT_MOTOMAN_REALTIME_COMM_CTRL_H

#include <QObject>
#include <QThread>
#include <cobotsys_logger.h>
#include "CobotMotomanRealTimeComm.h"
//TODO DONE
class CobotMotomanRealTimeCommCtrl : public QObject {
Q_OBJECT
protected:
    QThread workerThread;

public:
    CobotMotomanRealTimeComm* motoman;
    std::condition_variable& cond_msg;
public:
    CobotMotomanRealTimeCommCtrl(std::condition_variable& msg, const QString& hostIp, QObject* parent = nullptr)
            : QObject(parent), cond_msg(msg){
        motoman = new CobotMotomanRealTimeComm(msg, hostIp);
        motoman->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, motoman, &QObject::deleteLater);
        connect(this, &CobotMotomanRealTimeCommCtrl::start, motoman, &CobotMotomanRealTimeComm::start);
        connect(this, &CobotMotomanRealTimeCommCtrl::commandReady, motoman, &CobotMotomanRealTimeComm::writeLine);
        connect(motoman, &CobotMotomanRealTimeComm::connected, this, &CobotMotomanRealTimeCommCtrl::onRealTimeConnected);
        connect(this, &CobotMotomanRealTimeCommCtrl::stopServoj, motoman, &CobotMotomanRealTimeComm::stopProg);
        workerThread.start();
    }

    ~CobotMotomanRealTimeCommCtrl(){
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

#endif //COBOT_MOTOMAN_REALTIME_COMM_CTRL_H
