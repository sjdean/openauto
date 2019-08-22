/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QMessageBox>
#include <f1x/openauto/autoapp/UI/MonitorWindow.hpp>
#include <f1x/openauto/autoapp/Service/ICarConnect.hpp>
#include "ui_monitorwindow.h"

namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace ui
            {

                MonitorWindow::MonitorWindow(configuration::IConfiguration::Pointer configuration, service::ICarConnect::Pointer carconnect, QWidget *parent)
                        : QWidget(parent)
                        , ui_(new Ui::MonitorWindow)
                        , configuration_(std::move(configuration))
                        , carconnect_(std::move(carconnect))
                {
                    //ui_->setupUi(this);
                }

                MonitorWindow::~MonitorWindow()
                {
                    delete ui_;
                }

                void MonitorWindow::onRefresh()
                {

                }
            }
        }
    }
}
