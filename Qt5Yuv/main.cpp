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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>

#include <lqtutils_string.h>
#include <lc_logging.h>

#include "yuvdisplay.h"

int main(int argc, char** argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    qInstallMessageHandler(lightlogger::log_handler);
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Use an OpenGL shader to convert/upload yuv420p frames");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QSL("source"),
                                 QSL("Raw yuv420p file"));
    parser.addPositionalArgument(QSL("size"),
                                 QSL("Size with the format widthxheight"));
    parser.addPositionalArgument(QSL("fps"),
                                 QSL("Framerate in frames per second"));
    parser.process(app);

    if (parser.positionalArguments().size() != 3)
        parser.showHelp(1);

    QString filePath = parser.positionalArguments().at(0);
    assert(QFile::exists(filePath));

    QStringList tokens = parser.positionalArguments().at(1).split("x");
    assert(tokens.size() == 2);

    QSize videoSize(tokens[0].toInt(), tokens[1].toInt());
    assert(!videoSize.isEmpty());
    assert(!videoSize.isNull());

    int fps = parser.positionalArguments().at(2).toInt();

    qmlRegisterType<YuvDisplay>("luke.yuv", 1, 0, "YuvDisplay");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("g_videoPath", filePath);
    engine.rootContext()->setContextProperty("g_videoSize", videoSize);
    engine.rootContext()->setContextProperty("g_videoFps", fps);
    const QUrl url(QSL("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
