//
// Created by 杨帆 on 17-5-2.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#ifndef COBOT_MOTOMAN_H
#define COBOT_MOTOMAN_H

#include <vector>
#include <stdint.h>
#include <cstdio>
#include <QByteArray>
#include <cobotsys.h>
#include <condition_variable>

#define FRAME_LENGTH 80
#define UDP_PORT 11001
#define TCP_PORT 11000
#define MAX_SOCKET_WAIT 1000
#define JOINT_NUM 6
#define DIGITAL_INPUT_NUM 8
#define MAX_ANGLE_INCREMENT 1.0
#define FLOAT_PRECISION 10000

class MotomanRobotState{
public:
    MotomanRobotState(std::condition_variable& msg_cond);
    void unpack(QByteArray &msg);
    void setVesion();
    std::string getVersion();
    std::vector<double> getQActual();
    int getDigitalOutputBits();
    int getDigitalInputBits();
private:
    std::vector<double> q_target_; //Target joint positions//度
    //std::vector<double> qd_target_; //Target joint velocities
    //std::vector<double> qdd_target_; //Target joint accelerations
    //std::vector<double> i_target_; //Target joint currents
    //std::vector<double> m_target_; //Target joint moments (torques)
    std::vector<double> q_actual_; //Actual joint positions//度
    //std::vector<double> qd_actual_; //Actual joint velocities
    //std::vector<double> i_actual_; //Actual joint currents
    //std::vector<double> i_control_; //Joint control currents
    //std::vector<double> tool_vector_actual_; //Actual Cartesian coordinates of the tool: (x,y,z,rx,ry,rz), where rx, ry and rz is a rotation vector representation of the tool orientation
    //std::vector<double> tcp_speed_actual_; //Actual speed of the tool given in Cartesian coordinates
    //std::vector<double> tcp_force_; //Generalised forces in the TC
    //std::vector<double> tool_vector_target_; //Target Cartesian coordinates of the tool: (x,y,z,rx,ry,rz), where rx, ry and rz is a rotation vector representation of the tool orientation
    //std::vector<double> tcp_speed_target_; //Target speed of the tool given in Cartesian coordinates
    std::vector<bool> digital_input_bits_; //Current state of the digital inputs. NOTE: these are bits encoded as int64_t, e.g. a value of 5 corresponds to bit 0 and bit 2 set high
    std::vector<double> pos_actual_;//Actual cartesian position.//mm,度
    int major_version_;
    int minor_version_;
    std::condition_variable* pMsg_cond_;
};
using namespace cobotsys;
QByteArray IntToArray(qint32 source); //Use qint32 to ensure that the number have 4 bytes

#endif //COBOT_MOTOMAN_H
