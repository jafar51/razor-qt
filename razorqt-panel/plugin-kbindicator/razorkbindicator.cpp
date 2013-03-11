/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Kuzma Shapran <kuzma.shapran@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "razorkbindicator.h"

#include <X11/XKBlib.h>

#include <QtGui/QLabel>
#include <QtGui/QX11Info>


EXPORT_RAZOR_PANEL_PLUGIN_CPP(RazorKbIndicator)


RazorKbIndicator::RazorKbIndicator(const RazorPanelPluginStartInfo *startInfo, QWidget *parent):
    RazorPanelPlugin(startInfo, parent),
    mContent(new QLabel(this))
{
    setObjectName("KbIndicator");

    connect(this, SIGNAL(indicatorsChanged(uint,uint)), this, SLOT(setIndicators(uint,uint)));

    addWidget(mContent);

    int code;
    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;
    int XkbErrorBase;

    mDisplay = QX11Info::display();

    if (XkbLibraryVersion(&major, &minor))
        if (XkbQueryExtension(mDisplay, &code, &mXkbEventBase, &XkbErrorBase, &major, &minor))
            if (XkbUseExtension(mDisplay, &major, &minor))
                XkbSelectEvents(mDisplay, XkbUseCoreKbd, XkbIndicatorStateNotifyMask, XkbIndicatorStateNotifyMask);

    settingsChanged();
}

RazorKbIndicator::~RazorKbIndicator()
{
}

void RazorKbIndicator::settingsChanged()
{
    mBit = settings().value("bit", 0).toInt();
    mContent->setText(settings().value("text", QString("C")).toString());
    mContent->setEnabled(getLockStatus(mBit));
}

void RazorKbIndicator::showConfigureDialog()
{
    RazorKbIndicatorConfiguration *confWindow = this->findChild<RazorKbIndicatorConfiguration*>("KbIndicatorConfigurationWindow");

    if (!confWindow)
        confWindow = new RazorKbIndicatorConfiguration(settings(), this);

    confWindow->show();
    confWindow->raise();
    confWindow->activateWindow();
}

void RazorKbIndicator::setIndicators(unsigned int changed, unsigned int state)
{
    if (changed & (1 << mBit))
        mContent->setEnabled(state & (1 << mBit));
}

bool RazorKbIndicator::getLockStatus(int bit)
{
    bool state = false;
    if (mDisplay)
    {
        unsigned n;
        XkbGetIndicatorState(mDisplay, XkbUseCoreKbd, &n);
        state = (n & (1 << bit));
    }
    return state;
}

void RazorKbIndicator::x11EventFilter(XEvent* event)
{
    XkbEvent* xkbEvent = reinterpret_cast<XkbEvent*>(event);

    if (xkbEvent->type == mXkbEventBase + XkbEventCode)
        if (xkbEvent->any.xkb_type == XkbIndicatorStateNotify)
            emit indicatorsChanged(xkbEvent->indicators.changed, xkbEvent->indicators.state);
}
