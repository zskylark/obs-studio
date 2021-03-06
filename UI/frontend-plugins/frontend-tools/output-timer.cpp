#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs.hpp>
#include <util/util.hpp>
#include <QAction>
#include <QMainWindow>
#include <QTimer>
#include <QObject>
#include "output-timer.hpp"

using namespace std;

OutputTimer *ot;

OutputTimer::OutputTimer(QWidget *parent)
	: QDialog(parent),
	ui(new Ui_OutputTimer)
{
	ui->setupUi(this);

	QObject::connect(ui->outputTimerStream, SIGNAL(clicked()), this,
		SLOT(StreamingTimerButton()));
	QObject::connect(ui->outputTimerRecord, SIGNAL(clicked()), this,
		SLOT(RecordingTimerButton()));

	streamingTimer = new QTimer(this);
	streamingTimerDisplay = new QTimer(this);

	recordingTimer = new QTimer(this);
	recordingTimerDisplay = new QTimer(this);
}

void OutputTimer::closeEvent(QCloseEvent*)
{
	obs_frontend_save();
}

void OutputTimer::StreamingTimerButton()
{
	if (obs_frontend_streaming_active())
		obs_frontend_streaming_stop();
	else
		obs_frontend_streaming_start();
}

void OutputTimer::RecordingTimerButton()
{
	if (obs_frontend_recording_active())
		obs_frontend_recording_stop();
	else
		obs_frontend_recording_start();
}

void OutputTimer::StreamTimerStart()
{
	if (!isVisible()) {
		ui->outputTimerStream->setEnabled(false);
		return;
	}

	int hours = ui->streamingTimerHours->value();
	int minutes = ui->streamingTimerMinutes->value();
	int seconds = ui->streamingTimerSeconds->value();

	int total = (((hours * 3600) +
		(minutes * 60)) +
		seconds) * 1000;

	if (total == 0)
		total = 1000;

	streamingTimer->setInterval(total);
	streamingTimer->setSingleShot(true);

	QObject::connect(streamingTimer, SIGNAL(timeout()),
		SLOT(EventStopStreaming()));

	QObject::connect(streamingTimerDisplay, SIGNAL(timeout()), this,
		SLOT(UpdateStreamTimerDisplay()));

	streamingTimer->start();
	streamingTimerDisplay->start(1000);
	ui->outputTimerStream->setText(tr("Stop"));

	UpdateStreamTimerDisplay();
}

void OutputTimer::RecordTimerStart()
{
	if (!isVisible()) {
		ui->outputTimerRecord->setEnabled(false);
		return;
	}

	int hours = ui->recordingTimerHours->value();
	int minutes = ui->recordingTimerMinutes->value();
	int seconds = ui->recordingTimerSeconds->value();

	int total = (((hours * 3600) +
			(minutes * 60)) +
			seconds) * 1000;

	if (total == 0)
		total = 1000;

	recordingTimer->setInterval(total);
	recordingTimer->setSingleShot(true);

	QObject::connect(recordingTimer, SIGNAL(timeout()),
		SLOT(EventStopRecording()));

	QObject::connect(recordingTimerDisplay, SIGNAL(timeout()), this,
		SLOT(UpdateRecordTimerDisplay()));

	recordingTimer->start();
	recordingTimerDisplay->start(1000);
	ui->outputTimerRecord->setText(tr("Stop"));

	UpdateRecordTimerDisplay();
}

void OutputTimer::StreamTimerStop()
{
	ui->outputTimerStream->setEnabled(true);

	if (!isVisible() && streamingTimer->isActive() == false)
		return;

	if (streamingTimer->isActive())
		streamingTimer->stop();

	ui->outputTimerStream->setText(tr("Start"));

	if (streamingTimerDisplay->isActive())
		streamingTimerDisplay->stop();

	ui->streamTime->setText("00:00:00");
}

void OutputTimer::RecordTimerStop()
{
	ui->outputTimerRecord->setEnabled(true);

	if (!isVisible() && recordingTimer->isActive() == false)
		return;

	if (recordingTimer->isActive())
		recordingTimer->stop();

	ui->outputTimerRecord->setText(tr("Start"));

	if (recordingTimerDisplay->isActive())
		recordingTimerDisplay->stop();

	ui->recordTime->setText("00:00:00");
}

void OutputTimer::UpdateStreamTimerDisplay()
{
	int remainingTime = streamingTimer->remainingTime() / 1000;

	int seconds = remainingTime % 60;
	int minutes = (remainingTime % 3600) / 60;
	int hours = remainingTime / 3600;

	QString text;
	text.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
	ui->streamTime->setText(text);
}

void OutputTimer::UpdateRecordTimerDisplay()
{
	int remainingTime = recordingTimer->remainingTime() / 1000;

	int seconds = remainingTime % 60;
	int minutes = (remainingTime % 3600) / 60;
	int hours = remainingTime / 3600;

	QString text;
	text.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
	ui->recordTime->setText(text);
}

void OutputTimer::ShowHideDialog()
{
	if (!isVisible()) {
		setVisible(true);
		QTimer::singleShot(250, this, SLOT(show()));
	} else {
		setVisible(false);
		QTimer::singleShot(250, this, SLOT(hide()));
	}
}

void OutputTimer::EventStopStreaming()
{
	obs_frontend_streaming_stop();
}

void OutputTimer::EventStopRecording()
{
	obs_frontend_recording_stop();
}

static void SaveOutputTimer(obs_data_t *save_data, bool saving, void *)
{
	if (saving) {
		obs_data_t *obj = obs_data_create();

		obs_data_set_int(obj, "streamTimerHours",
				ot->ui->streamingTimerHours->value());
		obs_data_set_int(obj, "streamTimerMinutes",
				ot->ui->streamingTimerMinutes->value());
		obs_data_set_int(obj, "streamTimerSeconds",
				ot->ui->streamingTimerSeconds->value());

		obs_data_set_int(obj, "recordTimerHours",
				ot->ui->recordingTimerHours->value());
		obs_data_set_int(obj, "recordTimerMinutes",
				ot->ui->recordingTimerMinutes->value());
		obs_data_set_int(obj, "recordTimerSeconds",
				ot->ui->recordingTimerSeconds->value());

		obs_data_set_obj(save_data, "output-timer", obj);

		obs_data_release(obj);
	} else {
		obs_data_t *obj = obs_data_get_obj(save_data,
				"output-timer");

		if (!obj)
			obj = obs_data_create();

		ot->ui->streamingTimerHours->setValue(
				obs_data_get_int(obj, "streamTimerHours"));
		ot->ui->streamingTimerMinutes->setValue(
				obs_data_get_int(obj, "streamTimerMinutes"));
		ot->ui->streamingTimerSeconds->setValue(
				obs_data_get_int(obj, "streamTimerSeconds"));

		ot->ui->recordingTimerHours->setValue(
				obs_data_get_int(obj, "recordTimerHours"));
		ot->ui->recordingTimerMinutes->setValue(
				obs_data_get_int(obj, "recordTimerMinutes"));
		ot->ui->recordingTimerSeconds->setValue(
				obs_data_get_int(obj, "recordTimerSeconds"));

		obs_data_release(obj);
	}
}

extern "C" void FreeOutputTimer()
{
}

static void OBSEvent(enum obs_frontend_event event, void *)
{
	if (event == OBS_FRONTEND_EVENT_EXIT) {
		obs_frontend_save();
		FreeOutputTimer();
	} else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
		ot->StreamTimerStart();
	} else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPING) {
		ot->StreamTimerStop();
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
		ot->RecordTimerStart();
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPING) {
		ot->RecordTimerStop();
	}
}

extern "C" void InitOutputTimer()
{
	QAction *action = (QAction*)obs_frontend_add_tools_menu_qaction(
			obs_module_text("OutputTimer"));

	obs_frontend_push_ui_translation(obs_module_get_string);

	QMainWindow *window = (QMainWindow*)obs_frontend_get_main_window();

	ot = new OutputTimer(window);

	auto cb = [] ()
	{
		ot->ShowHideDialog();
	};

	obs_frontend_pop_ui_translation();

	obs_frontend_add_save_callback(SaveOutputTimer, nullptr);
	obs_frontend_add_event_callback(OBSEvent, nullptr);

	action->connect(action, &QAction::triggered, cb);
}
