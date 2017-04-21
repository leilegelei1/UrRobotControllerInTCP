//
// Created by 潘绪洋 on 17-4-5.
// Copyright (c) 2017 Wuhan Collaborative Robot Technology Co.,Ltd. All rights reserved.
//

#include <cobotsys.h>
#include <cobotsys_gui_logger_highlighter.h>
#include "BasicLoggerWidget.h"

BasicLoggerWidget::BasicLoggerWidget() {
    setupUi();
}

BasicLoggerWidget::~BasicLoggerWidget() {
    COBOT_LOG.clrFilter(this);
}

bool BasicLoggerWidget::setup(const QString& configFilePath) {
    return true;
}

void BasicLoggerWidget::updateTextToUI() {
    if (m_cachedMessage.size()) {
        m_textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        m_textCursor.insertText(m_cachedMessage);

        m_plainTextEdit->setTextCursor(m_textCursor);
        m_cachedMessage.clear();
    }
}

void BasicLoggerWidget::appendText(const std::string& message) {
    static QRegularExpression reg("^\\[\\s*([-A-Za-z_0-9]*)\\s*\\](.*)");

    QString final_message = QString::fromLocal8Bit(message.c_str());
    m_cachedMessage += final_message;
}

void BasicLoggerWidget::setupUi() {
    resize(1024, 600);
    setWindowTitle(tr("Logger"));
    m_plainTextEdit = new QPlainTextEdit(this);
    m_boxLayout = new QVBoxLayout;

    m_boxLayout->setContentsMargins(QMargins());
    m_boxLayout->addWidget(m_plainTextEdit);

    setLayout(m_boxLayout);

    QFont font = getMonospaceFont();
    m_plainTextEdit->setReadOnly(true);
    m_plainTextEdit->setFont(font);
    m_plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    m_textCursor = m_plainTextEdit->textCursor();
    cobotsys::gui::LoggerHighlighter::highlightEditorWithDefaultStyle(m_plainTextEdit->document());

    m_editUpdateTimer = new QTimer(this);
    m_editUpdateTimer->setInterval(1000 / 30); // 30Hz
    connect(m_editUpdateTimer, &QTimer::timeout, this, &BasicLoggerWidget::updateTextToUI);
    m_editUpdateTimer->start();

    COBOT_LOG.addFilter(this, [=](const std::string& m) { appendText(m); });
}