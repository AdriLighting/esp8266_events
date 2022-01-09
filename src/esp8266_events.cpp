/**
 * @file      esp8266_events.cpp
 *
 * @brief     main file
 * @see       https://github.com/AdriLighting
 * 
 * @author    AdriLighting   
 * @date      sam. 08 janv. 2022 18:40:11
 *
 */

#include "esp8266_events.h"

/**
 * @brief		timestamp du nombre total de seconde (hr*3600) + (min*60) + sec
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * 
 * @param[in]  hr    heure
 * @param[in]  min   minutes
 * @param[in]  sec   secondes
 *
 * @return  (hr*3600) + (min*60) + sec
 */
time_t makeTime(uint8_t hr, uint8_t min, uint8_t sec){   
  uint32_t seconds = 0;
  seconds+= hr * SECS_PER_HOUR;
  seconds+= min * SECS_PER_MIN;
  seconds+= sec;
  return (time_t)seconds; 
}

/**
 * @brief		instance pour un simple event
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 */
Event::Event(){
	_data = new EventData();
}

/**
 * @brief		statu wifi
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * 
 * @return	true if connected to routeur
 */
bool Event::wiFiIsConnected() {
 return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED);
}

/*
  0 dim   sun
  1 lun
  2 mar
  3 mer
  4 jeu
  5 ven
  6 sam
*/
/**
 * @brief		debug event
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * @param[in]  pos   position du type de debug demander
 */
void Event::debug(uint8_t pos){
	uint8_t pos_1 	= 0;
	uint8_t pos_2 	= 0;
	uint8_t period 	= 0;

	switch (pos) {
		case 0:pos_1 = 1;							break;
		case 1:pos_1 = 2;							break;
		case 2:pos_1 = 3; pos_2 = 4;	break;
		case 3:pos_1 = 3; pos_2 = 0;	break;
		case 4:pos_1 = 1; pos_2 = 4;	break;
		default:break;
	}
	switch (_data->_period) {
		case DAILYREPEAT: 	period = 2; break;
		case WEEKLYREPEAT:  period = 0; break;
		default:break;
	}
	Serial.printf_P(PSTR("[pos: %d][%s][%s][day: %d, c_day: %d, hr: %d, min: %d] %s\n"), 
		_pos,  
		EVT_PERIOD_T[period],  
		EVT_T[pos_1],
		_timeClient->getDay(), 
		_data->_triggerDay, 
		_data->_hr, 
		_data->_min,
		EVT_T[pos_2]
	);
}
void Event::debug_current(){
	Serial.printf_P(PSTR("pos: %d, day: %-5dhr: %d, min: %d, time: %s\n%11s: %-5dhr: %d, min: %d, triggerDay setup: %d\n"), 
		_pos,  
		_timeClient->getDay(), 
		_timeClient->getHours(), 
		_timeClient->getMinutes(),
		_timeClient->getFormattedTime().c_str(), 
		EVT_day, 
		_data->_triggerDay, 
		_data->_hr,		
		_data->_min,
		_data->_triggerDay_setup
	);	
}

/**
 * @brief		check evvent trigger
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 */
void Event::check(){
	if (_data == nullptr) 					return;
	if (_timeClient == nullptr) 		return;
	if (!wiFiIsConnected()) 				return;
	if (!_timeClient->isTimeSet()) 	return;

	time_t _now;
	time_t _evtTime;

	// if ( (millis()-_timer_ntp_test)>5000) {
	// 	_timer_ntp_test = millis();
	// 	debug_current();
	// 	if (_pos == 1) Serial.println("");
	// }


	/*
			alarame journalierre pour tous les jours de la semaine
		 	. 	un seul declenchment 
		 	
			FONCTIONNEMENT
			_triggerDay 
					indique si l'alarme est en attente, _triggerDay ne doit pas correspondre au jour actuelle 
						(pourra fonctionner en tant que "nextDay" ou  "jour prévu pour l'alarme")
		 	l'alarm se declenche chaque jour de la semaine, pour touts les semaine et pour tous les mois
		 		si l'alarm n'a pas etait declencher pour le jour actuelle
		 		 si l'alarme na pas etait declencher a l'heure exacte
		 		 	declenchment de lalarme

		 	SETUP (_triggerDay_setup)
		 		mise a jour de _triggerDay au d'eclenchment de la fonction check
		 			voir si il faut effectuer l'operation a d'autre moment

		 	SET TIME (Event::set_time)
	*/
	if (_data->_period == DAILYREPEAT) {

		// ##################################################################### Event setup 
		//
		/*
			verification a effectuer au d'eclenchment de la fonction check
		*/
		if (_data->_triggerDay_setup) {

			/*
				timestamp du nombre total de seconde du tempsEvent et du tempsActuelle
					(hr*3600) + (min*60) + sec
			*/
			_now 			= makeTime(_timeClient->getHours(), _timeClient->getMinutes(), _timeClient->getSeconds());
			_evtTime 	= makeTime(_data->_hr, _data->_min, _data->_sec);

			/*
				if 		l'heure prevu pour le declenchement de l'alarme est anterieur a l'heure actuelle
								l'alarme doit etre inactive est prévu pour un autre jour
				else	l'heure prevu pour le declenchement de l'alarme n'esp pas encor arriver
								l'alarme doit etre active est prévu pour le jour actuelle
			*/
			if (_evtTime <=_now) {   
				/*
				on met a jour("_triggerDay") sur le jour actuell ("_timeClient->getDay()")
					on attend un jour different
						on attend que le tempsEvent soit arriver
				*/
				_data->_triggerDay = _timeClient->getDay();
				debug(0);
			} else {
				/*
				on met a jour le jour de l'event ("_triggerDay") par default ("-1")
					on attend que le tempsEvent soit arriver
				*/				
				_data->_triggerDay = -1;
				debug(4);
			}

			/* Fin de la verification */
			_data->_triggerDay_setup = false;			
		}		
		//
		// #####################################################################


		/* on attend un jour different */
		if (_data->_triggerDay != _timeClient->getDay())  {
			/* timeStamp pour comparaison */
			_now 			= makeTime(_timeClient->getHours(), _timeClient->getMinutes(), _timeClient->getSeconds());
			_evtTime 	= makeTime(_data->_hr, _data->_min, _data->_sec);
			if(_evtTime <=_now){
				/* si le temp tempsEvent == tempsActuelle */
				if ((_timeClient->getHours() == _data->_hr)&&(_timeClient->getMinutes() == _data->_min)&&(_timeClient->getSeconds() == _data->_sec)){
					/* 
						rendre l'alarme inactive pour le jour actuele 
							(set _triggerDay avec le jour actuelle) 
					*/
					_data->_triggerDay = _timeClient->getDay();
					debug(1);
					/* declenchment de la fonction si referencer */
					if (_data->_activate && _trigger_func!=NULL) {_trigger_func();yield();}
					else if (_data->_activate && _trigger_func_t!=nullptr) {_trigger_func_t();yield();}
				}
			}
		}
	}
}

/**
 * @brief		Sets the time on event.
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * 
 * @param[in]  hr    
 * @param[in]  min   
 */
void Event::set_time(uint8_t hr, uint8_t min){
	if (_data == nullptr) return;

	if (!wiFiIsConnected()) {
		_data->_hr		= hr;
		_data->_min		= min;	
		_data->_triggerDay_setup = true;	
		return;
	}

	if (_data->_period == DAILYREPEAT) {
			_data->_hr 					= hr;
			_data->_min 				= min;	
			/*
			timestamp du nombre total de seconde du tempsEvent et du tempsActuelle
				(hr*3600) + (min*60) + sec
			*/
			time_t _now 			= makeTime(_timeClient->getHours(), _timeClient->getMinutes(), _timeClient->getSeconds());
			time_t _evtTime 	= makeTime(_data->_hr, _data->_min, _data->_sec);
			/*
				if 		l'heure prevu pour le declenchement de l'alarme est anterieur a l'heure actuelle
								l'alarme doit etre inactive est prévu pour un autre jour
				else	l'heure prevu pour le declenchement de l'alarme n'esp pas encor arriver
								l'alarme doit etre active est prévu pour le jour actuelle
			*/			
			if (_evtTime <=_now) { 
				/*
				rendre l'alarme inactive pour le jour actuele 
					on met a jour("_triggerDay") sur le jour actuell ("_timeClient->getDay()")
						on attend un jour different (Event::check)
							on attend que le tempsEvent soit arriver (Event::check)
				*/
				_data->_triggerDay = _timeClient->getDay();
				debug(3);					
			} else {
				/*
				on met a jour le jour de l'event ("_triggerDay") par default ("-1")
					on attend que le tempsEvent soit arriver (Event::check)
				*/					
				_data->_triggerDay = -1;
				debug(2);	
			}

		// }	
	}
}

/**
 * @brief   get data from event instance with a dynmaic new intsance of EventData
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * 
 * @param      intance temporaire EventData
 *
 * @return    postion de l'array des instance d'event
 */
uint8_t Event::get_data( EventData * data ) { 
	data->set_data(_data->_hr, _data->_min, _data->_sec, _data->_triggerDay, _data->_activate, _data->_period);
	return _pos;
}

/**
 * @brief		pas encor utiliser ?????????
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * 
 * @param   reference of JsonObject source
 */
void Event::get_data(JsonObject & doc) { 
	doc[F("hr")] 					= _data->_hr;
	doc[F("min")] 				= _data->_min;
	doc[F("sec")]	 				= _data->_sec;
	doc[F("triggerday")]	= _data->_triggerDay;
	doc[F("activate")]		= _data->_activate;
	doc[F("pos")]					= _pos;
}

/*
	SETTER
*/
void Event::set_NTPClient(NTPClient * Tc)						{ _timeClient = Tc;}
void Event::set_pos(uint8_t p)											{ _pos = p;}
void Event::set_activate(bool p)										{ _data->_activate = p;}
void Event::set_triggerFunc(trigger_func f)					{ _trigger_func = f;}
void Event::set_triggerFunc(callback_function_t f)	{ _trigger_func_t = f;}

void Event::data_print(){
	Serial.printf_P(PSTR("heure: %d\n"), _data->_hr);
	Serial.printf_P(PSTR("minute: %d\n"), _data->_min);
	Serial.printf_P(PSTR("trigger day: %d\n"), _data->_triggerDay);
	Serial.printf_P(PSTR("period: %d\n"), _data->_period);
}


/**
 * @brief      Constructs a new instance. (insatnce déstinée a un usage unique)
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 * 
 * @param      ptr ver linstance NTPClient
 * @param[in]  taille de l'array des instance d'event
 */
EventManager::EventManager(NTPClient * Tc, uint8_t maxCnt){
	_timeClient = Tc;									// A SUPPRIMER ???
	_eventArray = new Event[maxCnt]; 	// array of events insatnce
	_count = maxCnt; 									// size

	for (uint8_t i = 0; i < _count; ++i) {
		_eventArray[i].set_NTPClient(Tc); 			// passage du ptr pour chaque instance 
		_eventArray[i].set_pos(i);							// position array
	}

}

/*
	GETTER
*/
uint8_t EventManager::get_data(uint8_t p,  EventData * f)				{ return _eventArray[p].get_data(f);}

/*
	SETTER
*/	
void EventManager::set_triggerFunc(uint8_t p, trigger_func f)					{ _eventArray[p].set_triggerFunc(f);}
void EventManager::set_triggerFunc(uint8_t p, callback_function_t f)	{ _eventArray[p].set_triggerFunc(f);}
void EventManager::set_time(uint8_t p, uint8_t hr, uint8_t min)				{ _eventArray[p].set_time(hr, min);	}
void EventManager::set_activate(uint8_t p, bool v) 										{ _eventArray[p].set_activate(v);		}

/**
 * @brief   backgroud for all events trigger
 * @see     https://github.com/AdriLighting/
 *
 * @author  AdriLighting
 * @date    sam. 08 janv. 2022 18:40:11
 */
void EventManager::loop(){
	for (uint8_t i = 0; i < _count; ++i) {
		_eventArray[i].check();
	}
}