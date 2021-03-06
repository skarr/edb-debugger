/*
Copyright (C) 2006 - 2013 Evan Teran
                          eteran@alum.rit.edu

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DialogAttach.h"
#include "IDebuggerCore.h"
#include "Types.h"
#include "edb.h"

#include <QtDebug>
#include <QMap>
#include <QHeaderView>

#include "ui_DialogAttach.h"

#ifdef Q_OS_WIN32
namespace {
	int getuid() {
		return 0;
	}
}
#else
#include <unistd.h>
#endif

//------------------------------------------------------------------------------
// Name: DialogAttach
// Desc: constructor
//------------------------------------------------------------------------------
DialogAttach::DialogAttach(QWidget *parent) : QDialog(parent), ui(new Ui::DialogAttach) {
	ui->setupUi(this);
}

//------------------------------------------------------------------------------
// Name: ~DialogAttach
// Desc:
//------------------------------------------------------------------------------
DialogAttach::~DialogAttach() {
	delete ui;
}

//------------------------------------------------------------------------------
// Name: on_filter_textChanged
// Desc:
//------------------------------------------------------------------------------
void DialogAttach::on_filter_textChanged(const QString &text) {
	update_list(text);
}

//------------------------------------------------------------------------------
// Name: update_list
// Desc:
//------------------------------------------------------------------------------
void DialogAttach::update_list(const QString &filter) {

	if(edb::v1::debugger_core) {
		QMap<edb::pid_t, Process> procs = edb::v1::debugger_core->enumerate_processes();

		const edb::uid_t user_id = getuid();
		const bool filterUID = ui->filter_uid->isChecked();
		const QString lowerFilter = filter.toLower();
		ui->processes_table->setSortingEnabled(false);
		ui->processes_table->setRowCount(0);

		Q_FOREACH(const Process &process_info, procs) {

			const QString process_name = process_info.name;

			if(filter.isEmpty() || process_name.toLower().contains(lowerFilter)) {
				if(!filterUID || process_info.uid == user_id) {
					const int row = ui->processes_table->rowCount();
					ui->processes_table->insertRow(row);

					QTableWidgetItem *const item_pid = new QTableWidgetItem;
					item_pid->setData(Qt::DisplayRole, static_cast<quint64>(process_info.pid));

					QTableWidgetItem *item_uid;
					if(!process_info.user.isEmpty()) {
						item_uid = new QTableWidgetItem(process_info.user);
					} else {
						item_uid = new QTableWidgetItem;
						item_uid->setData(Qt::DisplayRole, static_cast<quint64>(process_info.uid));
					}

					ui->processes_table->setItem(row, 0, item_pid);
					ui->processes_table->setItem(row, 1, item_uid);
					ui->processes_table->setItem(row, 2, new QTableWidgetItem(process_info.name));
				}
			}
		}
		ui->processes_table->setSortingEnabled(true);
	}
}

//------------------------------------------------------------------------------
// Name: showEvent
// Desc:
//------------------------------------------------------------------------------
void DialogAttach::showEvent(QShowEvent *event) {
	Q_UNUSED(event);
	update_list(ui->filter->text());
}

//------------------------------------------------------------------------------
// Name: on_filter_uid_clicked
// Desc:
//------------------------------------------------------------------------------
void DialogAttach::on_filter_uid_clicked(bool checked) {
	Q_UNUSED(checked);
	update_list(ui->filter->text());
}

//------------------------------------------------------------------------------
// Name: selected_pid
// Desc:
//------------------------------------------------------------------------------
edb::pid_t DialogAttach::selected_pid(bool *ok) const {

	Q_ASSERT(ok);

	const QList<QTableWidgetItem *> sel = ui->processes_table->selectedItems();
	if(sel.size() != 0) {
		*ok = true;
		return sel.first()->text().toUInt();
	} else {
		*ok = false;
		return edb::pid_t();
	}
}
