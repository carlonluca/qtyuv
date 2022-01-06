/**
 * GPLv3 license
 *
 * Copyright (c) 2022 Luca Carlon
 *
 * This file is part of QtYuv
 *
 * QtYuv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QtYuv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtYuv.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef SIMPLEMATERIALITEM_H
#define SIMPLEMATERIALITEM_H

#include <QQuickItem>
#include <QImage>
#include <QSGTexture>
#include <QSGGeometryNode>
#include <QSGMaterialShader>
#include <QSGMaterial>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QThread>
#include <QString>
#include <QSize>

#include <lqtutils_bqueue.h>
#include <lqtutils_prop.h>
#include <lqtutils_freq.h>

struct VideoFrame
{
    qint64 pts;
    QByteArray frameData;
    QSize frameSize;
};

struct StreamReader : public QThread
{
    StreamReader(const QString& filePath, const QSize& frameSize, int fps, LQTBlockingQueue<VideoFrame>* buffer);
    void run() override;

private:
    QString m_filePath;
    QSize m_videoSize;
    int m_frameSize;
    int m_fps;
    LQTBlockingQueue<VideoFrame>* m_buffer;
};

class YuvShader : public QSGMaterialShader
{
public:
    explicit YuvShader() : QSGMaterialShader() { setShaderSourceFile(QOpenGLShader::Fragment, ":/yuv420p.fsh"); }
    ~YuvShader();

    const char* vertexShader() const override;
    char const *const *attributeNames() const override;
    void updateState(const RenderState& state, QSGMaterial* newMaterial, QSGMaterial *) override;
    void initialize() override;

private:
    unsigned int m_textures[3];
    int id_y;
    int id_u;
    int id_v;
    int id_matrix;
    int id_opacity;
};

class YuvMaterial : public QSGMaterial
{
public:
    QSGMaterialType* type() const { static QSGMaterialType type; return &type; }
    QSGMaterialShader* createShader() const { return new YuvShader; }

public:
    std::optional<VideoFrame> frame;
};

class YuvDisplay : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    L_RW_PROP_AS(QString, videoPath)
    L_RW_PROP_AS(QSize, videoSize)
    L_RW_PROP_AS(int, videoFps)
    L_RO_PROP_AS(LQTFreqMeter*, freqMeter, new LQTFreqMeter)
public:
    explicit YuvDisplay();
    ~YuvDisplay();
    QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData*) override;

public slots:
    void play();

signals:
    void frameSwapped();

private:
    LQTBlockingQueue<VideoFrame> m_queue;
    StreamReader* m_reader;
    std::optional<VideoFrame> m_currentFrame;
    QElapsedTimer m_elapsed;
};

#endif // SIMPLEMATERIALITEM_H
