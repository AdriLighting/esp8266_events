/**
 * @file      esp8266_events.h
 *
 * @brief     main file
 * @see       https://github.com/AdriLighting
 * 
 * @author    AdriLighting   
 * @date      sam. 08 janv. 2022 18:40:11
 *
 */

#ifndef ESP8266_EVENTS_H
#define ESP8266_EVENTS_H

	#include <Arduino.h>
	#include <ArduinoJson.h>
	#include <ESP8266WiFi.h>
	#include <NTPClient.h>

	#define SECS_PER_MIN  ((time_t)(60UL))
	#define SECS_PER_HOUR ((time_t)(3600UL))

	// #######################################################################
	/*
		DU TEXTE POUR DEBUG C MIEUX
	*/
	static const char EVT_free					[] PROGMEM = "";
	static const char EVT_day						[] PROGMEM = "triggerDay";

	static const char EVT_triggerTimePass		[] PROGMEM = "TriggerTime is passed, set current day to today, waiting different day";
	static const char EVT_eventSetup				[] PROGMEM = "Event setup";
	static const char EVT_eventChecker			[] PROGMEM = "Event trigger";
	static const char EVT_eventSetTime			[] PROGMEM = "Event set new time";
	static const char EVT_triggerTimeSet		[] PROGMEM = "TriggerTime is waiting, set current day to -1,";
	static const char* const EVT_T[] PROGMEM = {
		EVT_triggerTimePass, 
		EVT_eventSetup, 
		EVT_eventChecker, 
		EVT_eventSetTime, 
		EVT_triggerTimeSet
	};

	static const char EVT_period_weeklyR		[] PROGMEM = "WeeklyRepeat";
	static const char EVT_period_weekly			[] PROGMEM = "Weekly";
	static const char EVT_period_dailyR			[] PROGMEM = "DailyReapeat";
	static const char EVT_period_daily			[] PROGMEM = "Dayly";	
	static const char* const EVT_PERIOD_T[] PROGMEM = {
		EVT_period_weeklyR, 
		EVT_period_weekly, 
		EVT_period_dailyR, 
		EVT_period_daily
	};
	//
	// #######################################################################


	/**
	 * mod des events
	 *  DAILYREPEAT 	alarame journalierre pour tous les jours de la semaine
	 *  WEEKLYREPEAT 	alarame journalierre pour le voullu jour de la samaine  (a creer)
	 */
	typedef enum _EVENT_PERIOD
	{
		DAILYREPEAT,
		WEEKLYREPEAT
	} EVENT_PERIOD;

	/*
		triger function
	*/
	typedef void (*trigger_func)();  
	typedef std::function<void()> callback_function_t;

	/**
	 * @class			EventData
	 * 
	 * @brief			class pour les information de declenchement des event
	 * @see				https://github.com/AdriLighting/
	 * 
	 * @author		Adrilighting
	 * @date	   	sam. 08 janv. 2022 18:46:23
	 */
	class EventData {

	public:

		/** \brief heure du déclenchement */
		uint8_t _hr;

		/** \brief minute du déclenchement */
		uint8_t _min; 

		/** \brief seconde du déclenchement */
		uint8_t _sec; 

		/** \brief jour prévu pour le déclenchement */
		int 		_triggerDay;

		/** \brief mise a jour du jour prévu pour le déclenchement */
		bool 		_triggerDay_setup;

		/** \brief activation/désactivation de la fonction trigger de l'event */
		bool		_activate;

		/** \brief type de periode de l'event */
		EVENT_PERIOD _period;

		/**
		 * @brief      contructor
		 */
		EventData() { 
			/*
				valeurs par default
			*/ 
			this->_hr 				= 14;
			this->_min 				= 0;
			this->_sec 				= 0;
			this->_triggerDay_setup	= true;
			this->_activate			= false;
			this->_triggerDay		= -1;
			this->_period			= DAILYREPEAT;
		}

		/**
		 * @brief     utiliser pour recuperer les information d'un event deja instancier
		 *
		 * @param[in]  hr          
		 * @param[in]  min         
		 * @param[in]  sec         
		 * @param[in]  triggerDay  
		 * @param[in]  activate    
		 * @param[in]  period      
		 */
		void set_data( uint8_t hr, uint8_t  min, uint8_t sec, int triggerDay, bool activate, EVENT_PERIOD period  ) { 
			this->_hr 					= hr;
			this->_min 					= min;
			this->_sec					= sec;
			this->_triggerDay		= triggerDay;
			this->_activate			= activate;
			this->_period				= period;
		}	

		/**
		 * @brief      destructor
		 */
		~EventData(){;}
	};

	/**
	 * @class	   	Event
	 * 
	 * @brief			class de geestion pour chaque event 
	 * @see				https://github.com/AdriLighting/
	 * 
	 * @author		Adrilighting
	 * @date	   	sam. 08 janv. 2022 18:46:23
	 */	
	class Event
	{
	private:

		/** \brief  */
		uint32_t 							_timer_ntp_test;

		/** \brief postion of instance array */
		uint8_t 							_pos 						= 0;		

		/** \brief ptr ver l'inctance NTPClient */
		/*
			Voir en terme de ressource le plus optimale
				passer pour une fonction pour recuperer le ptr
				referencer le ptr via la class event
		*/
		NTPClient							* _timeClient 	= nullptr;

		/** \brief ptr ver l'instance EventData instancier par l'event */
		EventData							* _data 				= nullptr;

		/** \brief callback function w/ */
		trigger_func 					_trigger_func 	= NULL;

		/** \brief callback function w/ */
		callback_function_t 	_trigger_func_t	= nullptr;
	public:
		Event();
		~Event();

		void data_print();

		void check();

		void set_NTPClient(NTPClient * Tc);
		void set_pos(uint8_t p);

		uint8_t get_data(EventData * data );
		void get_data(JsonObject & doc);

		void set_time(uint8_t hr, uint8_t min);
		void set_activate(bool v);

		void set_triggerFunc(trigger_func f);
		void set_triggerFunc(callback_function_t f);

		void debug(uint8_t p);
		void debug_current();

		bool wiFiIsConnected();
		
	};	


	/**
	 * @class	   	EventManager
	 * 
	 * @brief			class de geestion general par event(s)
	 * @see				https://github.com/AdriLighting/
	 * 
	 * @author		Adrilighting
	 * @date	   	sam. 08 janv. 2022 18:46:23
	 */	
	class EventManager
	{
	private:

		/** \brief array dynamique des instance event */
		Event 			* _eventArray	= nullptr; 		

		/** \brief taille de l'array */		
		uint8_t 		_count 				= 0;

		/** \brief INUTILE ???? */
		NTPClient 	* _timeClient;
	public:

		EventManager(NTPClient * Tc, uint8_t maxCnt = 2);
		~EventManager(){};

		void loop();

		uint8_t get_data(uint8_t p,  EventData * f);

		void set_triggerFunc(uint8_t p, trigger_func f);
		void set_triggerFunc(uint8_t p, callback_function_t f);
		void set_time(uint8_t p, uint8_t hr, uint8_t min);
		void set_activate(uint8_t p, bool v);
	
	};

#endif // EVENT_H