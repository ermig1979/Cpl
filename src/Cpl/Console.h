/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2021 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include "Cpl/Defs.h"

namespace Cpl
{
    /*! @ingroup cpl_console
    * \namespace Console
    * \brief ANSI/VT100 terminal text styling helpers.
    * \note Stylized emits escape sequences only on Linux. On other platforms it returns the original text.
    */
    namespace Console
    {
        /*! @ingroup cpl_console
        * \enum Format
        * \brief ANSI SGR text attributes applied to the styled text.
        */
        enum Format
        {
            FormatDefault = 0,    //!< Default (normal) intensity and attributes.
            FormatBold = 1,       //!< Bold (increased intensity).
            FormatDim = 2,        //!< Dim (decreased intensity).
            FormatItalics = 3,    //!< Italic text.
            FormatUnderlined = 4, //!< Underlined text.
            FormatBlink = 5,      //!< Blinking text.
            FormatReverse = 7,    //!< Swap foreground and background colors.
            FormatHidden = 8,     //!< Concealed (hidden) text.
        };

        /*! @ingroup cpl_console
        * \enum Foreground
        * \brief ANSI SGR foreground (text) colors.
        */
        enum Foreground
        {
            ForegroundDefault = 39,      //!< Terminal default foreground color.
            ForegroundBlack = 30,        //!< Black foreground.
            ForegroundRed = 31,          //!< Red foreground.
            ForegroundGreen = 32,        //!< Green foreground.
            ForegroundYellow = 33,       //!< Yellow foreground.
            ForegroundBlue = 34,         //!< Blue foreground.
            ForegroundMagenta = 35,      //!< Magenta foreground.
            ForegroundCyan = 36,         //!< Cyan foreground.
            ForegroundLightGray = 37,    //!< Light gray foreground.
            ForegroundDarkGray = 90,     //!< Dark gray (bright black) foreground.
            ForegroundLightRed = 91,     //!< Light red foreground.
            ForegroundLightGreen = 92,   //!< Light green foreground.
            ForegroundLightYellow = 93,  //!< Light yellow foreground.
            ForegroundLightBlue = 94,    //!< Light blue foreground.
            ForegroundLightMagenta = 95, //!< Light magenta foreground.
            ForegroundLightCyan = 96,    //!< Light cyan foreground.
            ForegroundWhite = 97,        //!< White foreground.
        };

        /*! @ingroup cpl_console
        * \enum Background
        * \brief ANSI SGR background colors.
        */
        enum Background
        {
            BackgroundDefault = 49,      //!< Terminal default background color.
            BackgroundBlack = 40,        //!< Black background.
            BackgroundRed = 41,          //!< Red background.
            BackgroundGreen = 42,        //!< Green background.
            BackgroundYellow = 43,       //!< Yellow background.
            BackgroundBlue = 44,         //!< Blue background.
            BackgroundMegenta = 45,      //!< Magenta background.
            BackgroundCyan = 46,         //!< Cyan background.
            BackgroundLightGray = 47,    //!< Light gray background.
            BackgroundDarkGray = 100,    //!< Dark gray (bright black) background.
            BackgroundLightRed = 101,    //!< Light red background.
            BackgroundLightGreen = 102,  //!< Light green background.
            BackgroundLightYellow = 103, //!< Light yellow background.
            BackgroundLightBlue = 104,   //!< Light blue background.
            BackgroundLightMagenta = 105, //!< Light magenta background.
            BackgroundLightCyan = 106,   //!< Light cyan background.
            BackgroundWhite = 107,       //!< White background.
        };

        /*! @ingroup cpl_console
        * \enum Reset
        * \brief ANSI SGR codes appended after the text to clear previously applied attributes.
        */
        enum Reset
        {
            ResetAll = 0,         //!< Reset all attributes to the terminal defaults.
            ResetBold = 21,       //!< Reset bold.
            ResetDim = 22,        //!< Reset dim (decreased intensity).
            ResetUnderlined = 24, //!< Reset underline.
            ResetBlink = 25,      //!< Reset blink.
            ResetReverse = 27,    //!< Reset reverse video.
            ResetHidden = 28,     //!< Reset concealed (hidden) text.
        };

        /*! @ingroup cpl_console
        * \fn String Stylized(const String & text, Format format = FormatDefault, Foreground foreground = ForegroundDefault, Background background = BackgroundDefault, Reset reset = ResetAll)
        * \brief Wraps text in ANSI/VT100 SGR escape sequences on Linux; returns the original text on other platforms.
        * \param [in] text - Text to style.
        * \param [in] format - Text attributes such as bold or underline.
        * \param [in] foreground - Foreground (text) color.
        * \param [in] background - Background color.
        * \param [in] reset - Reset sequence appended after the text.
        * \return Styled string on Linux, unmodified text otherwise.
        * \note Escape sequences are emitted only when __linux__ is defined.
        */
        CPL_INLINE String Stylized(const String & text, Format format = FormatDefault, Foreground foreground = ForegroundDefault, 
            Background background = BackgroundDefault, Reset reset = ResetAll)
        {
#ifdef __linux__
            std::stringstream ss;
            ss << "\033[" << (int)format;
            ss << ";" << (int)foreground;
            ss << ";" << (int)background << "m";
            ss << text;
            ss << "\033[" << (int)reset << "m";
            return ss.str();
#else
            return text;
#endif
        }
    }
}
