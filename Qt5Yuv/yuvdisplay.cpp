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

#include "yuvdisplay.h"

#include <QFile>
#include <QQuickWindow>

StreamReader::StreamReader(const QString &filePath, const QSize &frameSize, int fps, LQTBlockingQueue<VideoFrame> *buffer) :
    QThread(),
    m_filePath(filePath),
    m_videoSize(frameSize),
    m_fps(fps),
    m_buffer(buffer)
{
    m_frameSize = 12*frameSize.width()*frameSize.height()/8;
}

void StreamReader::run()
{
    QFile f(m_filePath);
    qDebug() << m_filePath;
    assert(f.open(QIODevice::ReadOnly));

    int i = 0;
    while (!isInterruptionRequested()) {
        QByteArray frameData = f.read(m_frameSize);
        if (frameData.size() < m_frameSize)
            break;
        VideoFrame frame {
            .pts = static_cast<int>(qRound((i++)*(1.0/m_fps)*1000.0)),
            .frameData = frameData,
            .frameSize = m_videoSize
        };
        m_buffer->enqueue(frame);
    }

    qInfo() << "Stream reader thread disposed";
}

YuvShader::~YuvShader()
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glDeleteTextures(3, m_textures);
    qInfo() << "OGL textures freed";
}

const char* YuvShader::vertexShader() const
{
    return
            "attribute highp vec4 aVertex;           \n"
            "attribute highp vec2 aTexCoord;         \n"
            "uniform highp mat4 qt_Matrix;           \n"
            "varying highp vec2 vTexCoord;           \n"
            "void main() {                           \n"
            "    gl_Position = qt_Matrix * aVertex;  \n"
            "    vTexCoord = aTexCoord;              \n"
            "}";
}

const char* const *YuvShader::attributeNames() const
{
    static char const *const names[] = { "aVertex", "aTexCoord", 0 };
    return names;
}

void YuvShader::updateState(const RenderState& state, QSGMaterial* newMaterial, QSGMaterial*)
{
    if (state.isMatrixDirty())
        program()->setUniformValue(id_matrix, state.combinedMatrix());
    if (state.isOpacityDirty())
        program()->setUniformValue(id_opacity, state.opacity());

    std::optional<VideoFrame> frame = static_cast<const YuvMaterial*>(newMaterial)->frame;
    if (!frame || frame->frameData.isEmpty())
        return;

    uint fw = frame->frameSize.width();
    uint fh = frame->frameSize.height();
    uint pixels = fw*fh;
    uint uvHeight = fh/2;
    uint uvWidth = fw/2;

    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    GLubyte* planes[3] = {
        (GLubyte*)frame->frameData.data(),
        (GLubyte*)(frame->frameData.data() + pixels),
        (GLubyte*)(frame->frameData.data() + pixels + pixels/4)
    };
    uint widths[3] = {
        fw,
        uvWidth,
        uvWidth
    };
    uint heights[3] = {
        fh,
        uvHeight,
        uvHeight
    };
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < 3; i++) {
        f->glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_textures[i]);
        f->glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, widths[i], heights[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, planes[i]);
    }
}

void YuvShader::initialize()
{
    QSGMaterialShader::initialize();
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glGenTextures(3, m_textures);
    for (int i = 0; i < 3; i++) {
        f->glActiveTexture(GL_TEXTURE0 + i);
        f->glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_textures[i]);
        f->glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    id_y = program()->uniformLocation("s_texture_y");
    id_u = program()->uniformLocation("s_texture_u");
    id_v = program()->uniformLocation("s_texture_v");
    id_matrix = program()->uniformLocation("qt_Matrix");
    id_opacity = program()->uniformLocation("qt_Opacity");

    assert(id_y != -1);
    assert(id_u != -1);
    assert(id_v != -1);
    assert(id_matrix != -1);
    assert(id_opacity != -1);

    program()->setUniformValue(id_y, 0);
    program()->setUniformValue(id_u, 1);
    program()->setUniformValue(id_v, 2);
}

void YuvDisplay::play()
{
    if (!m_reader) {
        m_reader = new StreamReader(m_videoPath, m_videoSize, m_videoFps, &m_queue);
        m_reader->start();
        m_elapsed.start();
    }
}

YuvDisplay::YuvDisplay() : QQuickItem(), m_queue(3), m_reader(nullptr)
{
    setFlag(ItemHasContents, true);
    connect(this, &YuvDisplay::frameSwapped,
            m_freqMeter, &LQTFreqMeter::registerSample);
    connect(m_freqMeter, &LQTFreqMeter::freqChanged, this, [&] {
        qDebug() << "FPS:" << m_freqMeter->freq();
    });
}

YuvDisplay::~YuvDisplay()
{
    if (m_reader) {
        m_reader->requestInterruption();
        m_queue.requestDispose();
        m_reader->wait();
        delete m_reader;
        qInfo() << "YuvDisplay cleanup completed";
    }
}

QSGNode* YuvDisplay::updatePaintNode(QSGNode* node, UpdatePaintNodeData*)
{
    update();

    QSGGeometryNode* n = static_cast<QSGGeometryNode*>(node);
    if (!n) {
        n = new QSGGeometryNode;
        YuvMaterial* m = new YuvMaterial;
        QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
        QSGGeometry::updateTexturedRectGeometry(g, QRect(), QRect());

        n->setMaterial(m);
        n->setGeometry(g);
        n->setFlag(QSGGeometryNode::OwnsMaterial, true);
        n->setFlag(QSGGeometryNode::OwnsGeometry, true);
    }

    QSGGeometry::updateTexturedRectGeometry(n->geometry(), boundingRect(), QRectF(0, 0, 1920, 1080));

    std::optional<VideoFrame> nextFrame = m_queue.peek();
    bool swapped = false;
    if (nextFrame) {
        if (m_elapsed.elapsed() >= nextFrame->pts) {
            static_cast<YuvMaterial*>(n->material())->frame = m_queue.dequeue();
            emit frameSwapped();
            swapped = true;
        }
    }

    QSGNode::DirtyState state = QSGNode::DirtyGeometry;
    if (swapped)
        state |= QSGNode::DirtyMaterial;
    n->markDirty(state);

    return n;
}
