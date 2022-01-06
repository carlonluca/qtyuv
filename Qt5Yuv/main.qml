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

import QtQuick 2.15
import QtQuick.Window 2.15
import luke.yuv 1.0

Window {
    width: 1280
    height: 720
    visible: true
    title: "YUV to RGB with a Fragment Shader"

    YuvDisplay {
        id: yuvDisplay
        anchors.fill: parent
        videoPath: g_videoPath
        videoSize: g_videoSize
        videoFps: g_videoFps
        Component.onCompleted: {
            play()
        }
    }
}
