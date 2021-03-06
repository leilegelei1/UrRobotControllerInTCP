//
// Created by 杨帆 on 17-5-4.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#include <extra2.h>
#include <cobotsys_global_object_factory.h>
#include "MotomanManipulator.h"
#include <QtWidgets/QFileDialog>
#include <cobotsys_file_finder.h>

class AutoCtx {
protected:
    bool& m_sign;
public:
    AutoCtx(bool& b) : m_sign(b) { m_sign = true; }

    ~AutoCtx() { m_sign = false; }
};
#define Q_SLOT_REDUCE(flag) if (flag) {return;} AutoCtx autoctx(flag);


MotomanManipulator::MotomanManipulator() {
    m_initUIData = 0;
    ui.setupUi(this);

    m_noHandleChange = false;
    m_targetToGo.resize(6);

    m_sliders.push_back(ui.slider_1);
    m_sliders.push_back(ui.slider_2);
    m_sliders.push_back(ui.slider_3);
    m_sliders.push_back(ui.slider_4);
    m_sliders.push_back(ui.slider_5);
    m_sliders.push_back(ui.slider_6);

    m_target.push_back(ui.dsb_target_1);
    m_target.push_back(ui.dsb_target_2);
    m_target.push_back(ui.dsb_target_3);
    m_target.push_back(ui.dsb_target_4);
    m_target.push_back(ui.dsb_target_5);
    m_target.push_back(ui.dsb_target_6);

    m_actual.push_back(ui.dsb_real_1);
    m_actual.push_back(ui.dsb_real_2);
    m_actual.push_back(ui.dsb_real_3);
    m_actual.push_back(ui.dsb_real_4);
    m_actual.push_back(ui.dsb_real_5);
    m_actual.push_back(ui.dsb_real_6);

    for (auto& slider : m_sliders) {
        connect(slider, &QSlider::valueChanged, [=](int) { this->handleSliderChange(); });
    }

    for (auto& iter : m_target) {
        connect(iter, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                [=](double) { handleTargetChange(); });
    }

    connect(ui.btnCreate, &QPushButton::released, this, &MotomanManipulator::createRobot);
    connect(ui.btnStart, &QPushButton::released, this, &MotomanManipulator::startRobot);
    connect(ui.btnStop, &QPushButton::released, this, &MotomanManipulator::stopRobot);
    connect(this, &MotomanManipulator::updateActualQ, this, &MotomanManipulator::onActualQUpdate);
    connect(this, &MotomanManipulator::robotConnectStateChanged, this, &MotomanManipulator::handleRobotState);

    connect(ui.btnRecTarget, &QPushButton::released, this, &MotomanManipulator::onRecTarget);
    connect(ui.btnGoTarget, &QPushButton::released, this, &MotomanManipulator::onGoTarget);
    connect(ui.btnPosiB, &QPushButton::released, this, &MotomanManipulator::onPosiB);
    connect(ui.btnGoPosiB, &QPushButton::released, this, &MotomanManipulator::onGoPosiB);
    connect(ui.btnLoopAB, &QPushButton::released, this, &MotomanManipulator::onLoopAB);

    connect(ui.btnIoRevert, &QPushButton::released, this, &MotomanManipulator::revertIoPorts);

    ui.btnStart->setEnabled(false);
    ui.btnStop->setEnabled(false);

    m_groupBoxDefTitle = ui.groupBox->title();
    m_loopAB = false;
}

MotomanManipulator::~MotomanManipulator() {
    INFO_DESTRUCTOR(this);
}

bool MotomanManipulator::setup(const QString& configFilePath) {
    QJsonObject json;
    if (loadJson(json, configFilePath)) {
        m_joint_num = json["joint_num"].toDouble(6);
        m_defaultRobotInfo.clear();
        m_defaultRobotInfo << json["robot_factory"].toString();
        m_defaultRobotInfo << json["robot_type"].toString();

        int jmin = json["joint_min"].toInt(-180);
        int jmax = json["joint_max"].toInt(180);
        for (int i = 0; i < (int) m_sliders.size(); i++) {
            m_sliders[i]->setRange(jmin, jmax);
        }

        setupCreationList();
    } else {
        m_joint_num = 6;
        m_defaultRobotInfo.clear();
        for (int i = 0; i < (int) m_sliders.size(); i++) {
            m_sliders[i]->setRange(-180, 180);
        }
        setupCreationList();
    }
    return true;
}

void MotomanManipulator::handleSliderChange() {
    Q_SLOT_REDUCE(m_noHandleChange);

    for (size_t i = 0; i < m_sliders.size(); i++) {
        auto slider = m_sliders[i];
        auto dsbTgt = m_target[i];
        auto dsbAct = m_actual[i];
        if (slider->value() != (int) dsbTgt->value()) {
            dsbTgt->setValue(slider->value());
        }
    }
    updateTargetQ();
}

void MotomanManipulator::handleTargetChange() {
    Q_SLOT_REDUCE(m_noHandleChange);

    for (size_t i = 0; i < m_target.size(); i++) {
        auto slider = m_sliders[i];
        auto dsbTgt = m_target[i];
        auto dsbAct = m_actual[i];

        if ((int) dsbTgt->value() != slider->value()) {
            slider->setValue((int) dsbTgt->value());
            dsbTgt->setValue(slider->value());
            return;
        }
    }
    updateTargetQ();
}

void MotomanManipulator::createRobot() {
    if (!GlobalObjectFactory::instance()) return;
    if (ui.cboRobotType->count() == 0)
        return;

//    QString robotConfig = QFileDialog::getOpenFileName(this,
//                                                       tr("Get Robot Config JSON file ..."),
//                                                       QString(FileFinder::getPreDefPath().c_str()),
//                                                       tr("JSON files (*.JSON *.json)"));

#ifdef WIN32
    QString robotConfig = "D:/Projects/cobotsys/data/CONFIG/Motoman_MH5F_config.json";
#else
    QString robotConfig = "/home/sail/CLionProjects/cobotsys/data/CONFIG/Motoman_MH5F_config.json";
#endif
    if (robotConfig.isEmpty())
        return;

    QStringList obj_info = ui.cboRobotType->currentData().toStringList();
    QString factory = obj_info.front();
    QString typen = obj_info.back();

    auto obj = GlobalObjectFactory::instance()->createObject(factory, typen);
    m_ptrRobot = std::dynamic_pointer_cast<AbstractArmRobotRealTimeDriver>(obj);
    if (m_ptrRobot) {
        auto ob = std::dynamic_pointer_cast<ArmRobotRealTimeStatusObserver>(shared_from_this());
        m_ptrRobot->attach(ob);
        if (m_ptrRobot->setup(robotConfig)) {
            COBOT_LOG.notice() << "Create and setup success";


            ui.groupBox->setTitle(m_groupBoxDefTitle + " - " + m_ptrRobot->getRobotUrl());
        } else {
            m_ptrRobot.reset();
        }
    }

    ui.btnStart->setEnabled(true);
}

void MotomanManipulator::setupCreationList() {
    if (!GlobalObjectFactory::instance()) return;

    bool foundDefault = false;
    QString defIdxText;

    ui.cboRobotType->clear();
    auto factory_names = GlobalObjectFactory::instance()->getFactoryNames();
    for (auto& name : factory_names) {
        auto types = GlobalObjectFactory::instance()->getFactorySupportedNames(name);

        for (auto& type : types) {
            auto obj = GlobalObjectFactory::instance()->createObject(name, type);
            auto robot = std::dynamic_pointer_cast<AbstractArmRobotRealTimeDriver>(obj);
            if (robot) {
                QStringList data;
                QString text;
                text = QString("%1 - %2").arg(name.c_str()).arg(type.c_str());
                data << name.c_str();
                data << type.c_str();
                ui.cboRobotType->addItem(text, data);
                if (data == m_defaultRobotInfo) {
                    foundDefault = true;
                    defIdxText = text;
                }
            }
        }
    }

    if (foundDefault) {
        auto defaultIndex = ui.cboRobotType->findText(defIdxText);
        ui.cboRobotType->setCurrentIndex(defaultIndex);
        COBOT_LOG.info() << "Default ROBOT: " << defaultIndex;
    }
}

void MotomanManipulator::startRobot() {
    if (m_ptrRobot) {
        ui.btnStart->setEnabled(false);

        m_initUIData = 5;
        if (m_ptrRobot->start()) {
            COBOT_LOG.info() << "Robot Start Success";
        }
    }
}

void MotomanManipulator::stopRobot() {
    if (m_ptrRobot) {
        ui.btnStop->setEnabled(false);

        m_ptrRobot->stop();
        m_loopAB = false;
    }
}

void MotomanManipulator::onArmRobotConnect() {
    Q_EMIT robotConnectStateChanged(true);
}

void MotomanManipulator::onArmRobotDisconnect() {
    Q_EMIT robotConnectStateChanged(false);
}

void MotomanManipulator::onArmRobotStatusUpdate(const ArmRobotStatusPtr& ptrRobotStatus) {
    auto q_actual_size = ptrRobotStatus->q_actual.size();
    if (q_actual_size > 6)
        q_actual_size = 6;

    m_mutex.lock();
    m_actualValue.resize(q_actual_size);
    for (size_t i = 0; i < q_actual_size; i++) {
        m_actualValue[i] = ptrRobotStatus->q_actual[i];
    }
    m_mutex.unlock();

    Q_EMIT updateActualQ();
}

void MotomanManipulator::closeEvent(QCloseEvent* event) {
    if (m_ptrRobot) {
        m_ptrRobot->stop();
    }
    QWidget::closeEvent(event);
}

void MotomanManipulator::onActualQUpdate() {
    std::vector<double> tmpq;
    m_mutex.lock();
    tmpq = m_actualValue;
    m_mutex.unlock();
    m_qActual = tmpq;

    for (size_t i = 0; i < tmpq.size(); i++) {
        m_actual[i]->setValue(tmpq[i] / CV_PI * 180);
    }

    if (m_initUIData > 0) {
        auto log = COBOT_LOG.info();
        log << "Actual Q: ";
        for (size_t i = 0; i < tmpq.size(); i++) {
            m_target[i]->setValue(tmpq[i] / CV_PI * 180);
            log << tmpq[i] / CV_PI * 180 << ". ";
        }
        m_targetToGo = m_actualValue;
        m_initUIData--;
    }

    if (m_loopAB) {
        loopAbProg();
    }
}

void MotomanManipulator::updateTargetQ() {
    if (m_initUIData > 0) {
        return;
    }

    std::vector<double> target_q = getUiTarget();

    goTarget(target_q);
}

void MotomanManipulator::onRecTarget() {
    m_PosiA = getUiTarget();
}

void MotomanManipulator::onGoTarget() {
    goTarget(m_PosiA);
}

void MotomanManipulator::onPosiB() {
    m_PosiB = getUiTarget();
}

void MotomanManipulator::onGoPosiB() {
    goTarget(m_PosiB);
}

void MotomanManipulator::onLoopAB() {
    m_loopAB = !m_loopAB;
    if (m_loopAB) {
        m_loopQ = m_PosiA;
        m_loopQQueueIndex = 0;
        m_loopQQueue.clear();
        m_loopQQueue.push_back(m_PosiA);
        m_loopQQueue.push_back(m_PosiB);
        goTarget(m_loopQ);
    }
}

std::vector<double> MotomanManipulator::getUiTarget() {
    std::vector<double> target;
    target.resize(m_target.size());
    for (int i = 0; i < (int) target.size(); i++) {
        target[i] = m_target[i]->value() / 180.0 * CV_PI;
    }
    return target;
}

void MotomanManipulator::goTarget(const std::vector<double>& targetq) {
    m_targetToGo = targetq;

    updateTargetQToUi();

    if (m_ptrRobot) {
        COBOT_LOG.message("Target") << putfixedfloats(6, 1, m_targetToGo, 180 / M_PI);
        m_ptrRobot->move(m_targetToGo);
    }
}

void MotomanManipulator::loopAbProg() {
    if (m_loopQ.size() != m_qActual.size())
        return;

    double err = 0;
    for (size_t i = 0; i < m_loopQ.size(); i++) {
        auto d = m_loopQ[i] - m_qActual[i];
        err += d * d;
    }
    err = sqrt(err);

    if (err <= 0.1) {
        m_loopQQueueIndex++;
        if (m_loopQQueueIndex >= m_loopQQueue.size())
            m_loopQQueueIndex = 0;
        m_loopQ = m_loopQQueue[m_loopQQueueIndex];
        goTarget(m_loopQ);
    }
}

void MotomanManipulator::updateTargetQToUi() {
    for (int i = 0; i < (int) m_targetToGo.size(); i++) {
        m_target[i]->setValue(m_targetToGo[i] * 180 / CV_PI);
    }
}

void MotomanManipulator::handleRobotState(bool isConnected) {
    if (isConnected) {
        ui.btnStart->setEnabled(false);
        ui.btnStop->setEnabled(true);
    } else {
        ui.btnStart->setEnabled(true);
        ui.btnStop->setEnabled(false);
    }
}

void MotomanManipulator::revertIoPorts() {
    static bool io_stat = true;
    if (m_ptrRobot) {
        auto ptrIo = m_ptrRobot->getDigitIoDriver();

        if (io_stat) {
            ptrIo->setIo(DigitIoPort::Port_1, DigitIoStatus::Set);
        } else {
            ptrIo->setIo(DigitIoPort::Port_1, DigitIoStatus::Reset);
        }
        io_stat = !io_stat;
    }
}
