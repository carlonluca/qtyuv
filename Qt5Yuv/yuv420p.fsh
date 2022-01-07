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

uniform sampler2DRect s_texture_y;
uniform sampler2DRect s_texture_u;
uniform sampler2DRect s_texture_v;
uniform lowp float qt_Opacity;
varying highp vec2 vTexCoord;

void main()
{
    float Y = texture2DRect(s_texture_y, vTexCoord).r;
    float U = texture2DRect(s_texture_u, vec2(vTexCoord.x/2., vTexCoord.y/2.)).r - 0.5;
    float V = texture2DRect(s_texture_v, vec2(vTexCoord.x/2., vTexCoord.y/2.)).r - 0.5;
    vec3 color = vec3(Y, U, V);
    mat3 colorMatrix = mat3(
                1,   0,       1.402,
                1,  -0.344,  -0.714,
                1,   1.772,   0);
    gl_FragColor = vec4(color*colorMatrix, 1.)*qt_Opacity;
}
