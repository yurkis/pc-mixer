#ifndef _PC_MIXER_TRAY_H
#define _PC_MIXER_TRAY_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QSlider>
#include <QToolButton>
#include <QWidgetAction>
#include <QSettings>
#include <QString>
#include <QCoreApplication>
#include <QCursor>
#include <QEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QDebug>

#include "MixerBackend.h" //for CLI mixer integration
#include "MixerGUI.h"

class MixerTray : public QSystemTrayIcon{
	Q_OBJECT
public:
	MixerTray();
	~MixerTray();

private:
	QMenu *actionMenu;
    QMenu *soundOutput;
	QSlider *slider;
	QToolButton *mute, *mixer;
	QWidgetAction *slideA, *muteA, *mixerA;
	QSettings *settings;
	QTimer *timer;
	MixerGUI *GUI;
	int CVOL; //current volume
	int CDIFF; //difference between L/R channels
	bool starting, isMuted;



	void changeVol(int percent, bool modify = true); //Set volume to value (0-100), -1 = mute but save volume    
	void RestartPulseAudio();

	QStringList runShellCommand(QString cmd){
	 //split the command string with individual commands seperated by a ";" (if any)
	   QProcess p;  
	   //Make sure we use the system environment to properly read system variables, etc.
	   p.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
	   //Merge the output channels to retrieve all output possible
	   p.setProcessChannelMode(QProcess::MergedChannels);   
	   p.start(cmd);
	   while(p.state()==QProcess::Starting || p.state() == QProcess::Running){
	     p.waitForFinished(200);
	     QCoreApplication::processEvents();
	   }
	   QString outstr = p.readAllStandardOutput();
	 if(outstr.endsWith("\n")){outstr.chop(1);} //remove the newline at the end 
	 return outstr.split("\n");
	}

private slots:
	void loadVol(); //Sync with backend mixer (or if default device changed)

	//Click slots
	void trayActivated(){
	  actionMenu->popup( QCursor::pos() );
	}

	//Menu Options
	void openMixerGUI(){
	  GUI->updateGUI();
	  GUI->show();
	  GUI->raise();
	}
	
	void closeTray(){
	  QCoreApplication::exit(0);
	}

	void hoverDisable(QAction* action){
	  if(action != soundOutput->menuAction() && actionMenu->geometry().contains(QCursor::pos()) ){
	    soundOutput->hide();
	  }
	}

	void muteClicked(){
	  if(isMuted && CVOL ==0){
	    changeVol(50); //Go to half value
	  }else{
	    changeVol(-1); //Mute or return to original value
	  }
	}
	
	void sliderChanged(int value){
	  changeVol(value);
	}

	//Simple fix for single-instance issues where the DE also tries to start it up 
	//  (double-tap on login)
	void doneStarting(){
	  starting = false;
      timer->start();
	}

	void slotSingleInstance(){
	  this->show();
	  //if(!starting){ openMixerGUI(); }
	}

    void slotFillOutputDevices();

    void slotOutputSelected();

protected:
	//look for wheel events here
	bool event(QEvent *event){
	  if(event->type() == QEvent::Wheel){
	    QWheelEvent *whEvent = static_cast<QWheelEvent*>(event);
	    int change = whEvent->delta()/40; // 3% volume change per 1/15th of a rotation (delta*3/120)
	    change = CVOL+change;
	    if(change > 100){ change = 100; }
	    if(change < 0){ change = 0; }
	    if(change != CVOL){
	      changeVol(change);
	    }
	    return true;
	  }else{
	    return false;
	  }
	}

};

#endif
